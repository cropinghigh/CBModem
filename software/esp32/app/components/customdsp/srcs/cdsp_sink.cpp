#include "cdsp_sink.h"

cdsp_sink_dac::cdsp_sink_dac(chl_i2sanalog* dac_dev, bool dac_out_ch) {
    _dac_dev = dac_dev;
    _dac_out_ch = dac_out_ch;
    _dac_dev->stopTx();
    _dac_dev->clearTxBuffers();
    for(int i = 0; i < CDSP_SINK_DAC_BUFFS_CNT; i++) {
        _out_bufs[i] = (chl_i2sanalog_dact*)heap_caps_malloc(CDSP_DEF_BUFF_SIZE*sizeof(chl_i2sanalog_dact), MALLOC_CAP_DMA);
        if(_out_bufs[i] == NULL) {
            printf("CDSP_SINK_DAC ERROR: NOT ENOUGH MEMORY FOR BUFFER %d\r\n", i);
            return;
        }
        _curr_buff_status[i] = 0;
    }
}

cdsp_sink_dac::~cdsp_sink_dac() {
     for(int i = 0; i < CDSP_SINK_DAC_BUFFS_CNT; i++) {
        heap_caps_free(_out_bufs[i]);
     }
}

void cdsp_sink_dac::setOutChannel(bool dac_out_ch) {
    _dac_out_ch = dac_out_ch;
}

int cdsp_sink_dac::work(void* ctx, int samples_cnt) {
    cdsp_sink_dac* _this = (cdsp_sink_dac*) ctx;
    if(!_this->_running) {return -2;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(got <= 0) {return got;}
    int curr_buff = -1;
    for(int i = 0; i < CDSP_SINK_DAC_BUFFS_CNT; i++) {
        if(_this->_curr_buff_status[i] == 0) {
            //buff is free, using
            curr_buff = i;
            break;
        }
    }
    if(curr_buff == -1) {
        printf("CDSP_SINK_DAC ERROR: NO FREE BUFFERS!\r\n");
    }
    for(int i = 0; i < got; i++) {
        (_this->_dac_out_ch ? _this->_out_bufs[curr_buff][i].ch1d : _this->_out_bufs[curr_buff][i].ch2d) = _this->_in_buff[i] * _this->_conv_coeff;
        (_this->_dac_out_ch ? _this->_out_bufs[curr_buff][i].ch2d : _this->_out_bufs[curr_buff][i].ch1d) = 0;
    }
    int swapped = _this->_dac_dev->set_tx_buffer(_this->_out_bufs[curr_buff], got, CDSP_SINK_TIMEOUT);
    if(swapped < 0) { return swapped; }
    for(int i = 0; i < CDSP_SINK_DAC_BUFFS_CNT; i++) {
        if(_this->_curr_buff_status[i] == swapped + 1) { //buff i has been transmitted successfully, now free
            _this->_curr_buff_status[i] = 0;
            break;
        }
    }
    _this->_curr_buff_status[curr_buff] = swapped+1;
    return got;
}

void cdsp_sink_dac::_do_start() {
    _dac_dev->clearTxBuffers();
    for(int i = 0; i < CDSP_SINK_DAC_BUFFS_CNT; i++) {
        _curr_buff_status[i] = 0;
    }
    //Fill 2 buffers before starting tx to make sure it will not stop
    int got = work(this, CDSP_DEF_BUFF_SIZE);
    if(got <= 0) { printf("CDSP_SINK_DAC START 1 ERROR: GOT %d SPLS!\r\n", got); return; }
    got = work(this, CDSP_DEF_BUFF_SIZE);
    if(got <= 0) { printf("CDSP_SINK_DAC START 2 ERROR: GOT %d SPLS!\r\n", got); return; }
    _dac_dev->startTx();
}

void cdsp_sink_dac::_do_stop() {
    _dac_dev->stopTx();
}

cdsp_sink_si5351_fr::cdsp_sink_si5351_fr(chl_ext_si5351* si5351_dev, bool pll, int startTimerVal, int timer_rate) {
    _si5351_dev = si5351_dev;
    _tmr = new chl_timer(0);
    _tmr->configurePrescaler(CDSP_SINK_SI_TMR_PRESCALER);
    _tmr->configureTimer(true, true, startTimerVal, (80000000/CDSP_SINK_SI_TMR_PRESCALER)/timer_rate);
    _tmr->setInterruptEnabled(true);
    _tmr->setTimerRunning(false);
    _tmr->forceReloadDefVal();
    _tmr->setCallback(isr_work, this);
    setPll(pll);
    setTimerRate(timer_rate);
    setStartTimerVal(startTimerVal);
    _write_bsmph = xSemaphoreCreateBinary();
    if(_write_bsmph == NULL) {
        printf("Si5351 SINK ERROR: NOT ENOUGH MEMORY FOR TX SMPH!\r\n");
        return;
    }
    xSemaphoreGive(_write_bsmph);
}

cdsp_sink_si5351_fr::~cdsp_sink_si5351_fr() {
    delete _tmr;
    vSemaphoreDelete(_write_bsmph);
}

void cdsp_sink_si5351_fr::setPll(bool pll) {
    _pll = pll;
}

void cdsp_sink_si5351_fr::setTimerRate(int timer_rate) {
    _tmr->setTimerVal((80000000/CDSP_SINK_SI_TMR_PRESCALER)/timer_rate);
}

void cdsp_sink_si5351_fr::setStartTimerVal(int startTimerVal) {
    _tmr->setDefaultVal(startTimerVal);
}

void cdsp_sink_si5351_fr::resetTimer() {
    _tmr->forceReloadDefVal();
}

void cdsp_sink_si5351_fr::asyncSetFreq(int32_t newfr) {
    _curr_freq = newfr;
}

int cdsp_sink_si5351_fr::syncSetFreq(int32_t newfr) {
    if(!_running) {return -2;}
    if(xSemaphoreTake(_write_bsmph, CDSP_SINK_TIMEOUT) != pdTRUE) {
        return -3;
    }
    _curr_freq = newfr;
    return 1;
}

int cdsp_sink_si5351_fr::syncQueueFreqs(int32_t* newfrs, int cnt) {
    if(!_running) {return -2;}
    for(int i = 0; i < cnt; i++) {
        if(((_freq_buff_write_ptr+1)%CDSP_DEF_BUFF_SIZE) == _freq_buff_read_ptr) { //if current write will overflow the buffer
            printf("FO\n");
            if(xSemaphoreTake(_write_bsmph, CDSP_SINK_TIMEOUT) != pdTRUE) { //wait until some data is consumed
                return -3;
            }
        }
        _freq_buff[_freq_buff_write_ptr] = newfrs[i];
        _freq_buff_write_ptr = (_freq_buff_write_ptr+1)%CDSP_DEF_BUFF_SIZE;
    }
    return cnt;
}

void cdsp_sink_si5351_fr::isr_work(void* ctx) {
    cdsp_sink_si5351_fr* _this = (cdsp_sink_si5351_fr*)ctx;
    portBASE_TYPE contsw_req = false;
    if(_this->_freq_buff_write_ptr != _this->_freq_buff_read_ptr) { //if some data is in the buffer
        _this->_curr_freq = _this->_freq_buff[_this->_freq_buff_read_ptr];
        _this->_freq_buff_read_ptr = (_this->_freq_buff_read_ptr+1)%CDSP_DEF_BUFF_SIZE;
    }
    _this->_si5351_dev->set_pll_frequency_fast(_this->_pll, _this->_curr_freq);
    xSemaphoreGiveFromISR(_this->_write_bsmph, &contsw_req);
    if(contsw_req) {
        portYIELD_FROM_ISR();
    }
}

int cdsp_sink_si5351_fr::work(void* ctx, int samples_cnt) {
    cdsp_sink_si5351_fr* _this = (cdsp_sink_si5351_fr*)ctx;
    if(!_this->_running) {return -2;}
    if(xSemaphoreTake(_this->_write_bsmph, CDSP_SINK_TIMEOUT) != pdTRUE) {
        return -3;
    }
    int got = _this->_input_func(_this->_func_call_ctx, &(_this->_curr_freq), 1);
    return got;
}

void cdsp_sink_si5351_fr::_do_start() {
    xSemaphoreGive(_write_bsmph);
    _freq_buff_write_ptr = 0;
    _freq_buff_read_ptr = 0;
    _tmr->forceReloadDefVal();
    _tmr->setTimerRunning(true);
}

void cdsp_sink_si5351_fr::_do_stop() {
    _tmr->setTimerRunning(false);
}


cdsp_sink_combined::cdsp_sink_combined(chl_i2sanalog* dac_dev, chl_ext_si5351* si5351_dev, bool dac_out_ch, bool pll, int startTimerVal, int timer_rate, int interp, int taps_cnt) {
    sink_dac = new cdsp_sink_dac(dac_dev, dac_out_ch);
    sink_si = new cdsp_sink_si5351_fr(si5351_dev, pll, startTimerVal, timer_rate);
    _taps_cnt = taps_cnt;
    _interp = interp;
    _timer_rate = timer_rate;
    _interpol = new cdsp_rational_interpolator<float>(_interp);
    _interpol_fir_taps = new float[_taps_cnt];
    cdsp_calc_taps_lpf_float(_interpol_fir_taps, _taps_cnt, timer_rate*_interp, timer_rate/2.0f, true);
    _interpol_fir = new cdsp_fir<float, float>(_taps_cnt, _interpol_fir_taps);
    sink_dac->setInputBlk(_interpol_fir, _interpol_fir->requestData);
    _interpol_fir->setInputBlk(_interpol, _interpol->requestData);
    _interpol->setInputFunc(this, dac_req_func);
}

cdsp_sink_combined::~cdsp_sink_combined() {
    delete sink_dac;
    delete sink_si;
    delete _interpol;
    delete[] _interpol_fir_taps;
}

int cdsp_sink_combined::work(void* ctx, int samples_cnt) {
    cdsp_sink_combined* _this = (cdsp_sink_combined*) ctx;
    return _this->sink_dac->work(_this->sink_dac, samples_cnt);
}

int cdsp_sink_combined::dac_req_func(void* ctx, float* data, int samples_cnt) {
    cdsp_sink_combined* _this = (cdsp_sink_combined*) ctx;
    if(!_this->_running) {return -2;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(got <= 0) {return got;}
    for(int i = 0; i < got; i++) {
        cdsp_complex_t spl = _this->_in_buff[i];
        float ph = spl.ph();
        float phdiff = (ph - _this->_prev_ph);
        phdiff += (phdiff > (FL_PI)) ? -(2.0f*FL_PI) : (phdiff<(-FL_PI)) ? (2.0f*FL_PI) : 0.0f; //find smallest angle diff; equal to 2pi * curr_f / fS
        float curr_freq_hz = phdiff * _this->_timer_rate / (2.0f*FL_PI);
        float ampl = spl.mag();
        if(ampl > 1.0f) ampl = 1.0f;
        if(ampl < 0.0f) ampl = 0.0f;
        _this->_prev_ph = ph;
        _this->_fr_buff[i] = roundf(curr_freq_hz);
        data[0] = ampl;
    }
    _this->sink_si->syncQueueFreqs(_this->_fr_buff, got);
    return got;
}

void cdsp_sink_combined::_do_start() {
    _buff_data = 0;
    _prev_ph = 0;
    sink_si->start(true);
    sink_dac->start(true);
}

void cdsp_sink_combined::_do_stop() {
    sink_si->stop(true);
    sink_dac->stop(true);
}
