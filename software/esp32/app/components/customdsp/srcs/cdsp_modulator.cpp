#include "cdsp_modulator.h" 

#include "stdio.h"

cdsp_mod_bfsk::cdsp_mod_bfsk(float fs, float fr0, float fr1, float datarate, int taps_cnt) {
    _taps_cnt = taps_cnt;
    setFs(fs);
    setFrs(fr0, fr1);
    setDataRate(datarate);
    _filt_taps = new float[_taps_cnt];
    float maxfreq = std::max(fabsf(fr0), fabsf(fr1)) + datarate;
    cdsp_calc_taps_lpf_float(_filt_taps, _taps_cnt, fs, maxfreq, true);
    // cdsp_calc_taps_lpf_rrc(_filt_taps, _taps_cnt, fs/maxfreq, 0.35f);
    _filt = new cdsp_fir<float, cdsp_complex_t>(_taps_cnt, _filt_taps);
    _gen = new cdsp_gen_sine_complex(fs, fr0);
    _filt->setInputFunc(this, filt_requestData);
}

cdsp_mod_bfsk::~cdsp_mod_bfsk() {
    delete[] _filt_taps;
    delete _filt;
    delete _gen;
}

void cdsp_mod_bfsk::setFs(float fs) {
    _fs = fs;
    if(_fs != 0) {
        _incr = _datarate / _fs;
    }
}

void cdsp_mod_bfsk::setFrs(float fr0, float fr1) {
    _fr0 = fr0;
    _fr1 = fr1;
}

void cdsp_mod_bfsk::setDataRate(float datarate) {
    _datarate = datarate;
    if(_fs != 0) {
        _incr = _datarate / _fs;
    }
}

int cdsp_mod_bfsk::requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_mod_bfsk* _this = (cdsp_mod_bfsk*) ctx;
    if(!_this->_running) {return -2;}
    return _this->_filt->requestData(_this->_filt, data, samples_cnt);
}

int cdsp_mod_bfsk::filt_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_mod_bfsk* _this = (cdsp_mod_bfsk*) ctx;
    if(!_this->_running || _this->_input_func==NULL || _this->_incr <= 0) { return -2;}
    int ret = 0;
    while(ret < samples_cnt) {
        int remaining_spls = std::min(int((1.0f / _this->_incr) - _this->_ctr), samples_cnt-ret);
        int gotmain = _this->_gen->requestData(_this->_gen, &data[ret], remaining_spls);
        if(gotmain <= 0) {return gotmain;}
        ret += gotmain;
        _this->_ctr += gotmain * _this->_incr;
        if(_this->_ctr >= 1.0f) {
            int got = _this->_input_func(_this->_func_call_ctx, &_this->_curr_data, 1);
            if(got != 1) {return got;}
            if(_this->_curr_data) {
                _this->_gen->setFr(_this->_fr1);
            } else {
                _this->_gen->setFr(_this->_fr0);
            }
            _this->_ctr = 0.0f;
        }
    }
    return ret;
}

void cdsp_mod_bfsk::_do_start()  {
    _gen->start(true);
    _filt->start(true);
}

void cdsp_mod_bfsk::_do_stop() {
    _gen->stop(true);
    _filt->stop(true);
}


cdsp_demod_bfsk::cdsp_demod_bfsk(float fs, float fr0, float fr1, float datarate, float sync_loop_bw, float sync_damping, float sync_relLimit) {
    // _taps_cnt = taps_cnt;
    _taps_cnt = 3;
    // _out_taps_cnt = out_taps_cnt;
    _filt_taps = new float[_taps_cnt];
    // _filt_out_taps = new float[_out_taps_cnt];
    _filt0 = new cdsp_fir<float, cdsp_complex_t>(_taps_cnt, _filt_taps);
    _filt1 = new cdsp_fir<float, cdsp_complex_t>(_taps_cnt, _filt_taps);
    // _filt_out = new cdsp_fir<float, float>(_out_taps_cnt, _filt_out_taps);
    setDataRate(datarate);
    setFs(fs);
    _sync = new cdsp_maximum_likelihood_tr<float>(_incr, sync_loop_bw, sync_damping, sync_relLimit);
    _dcblock = new cdsp_dcblock<float>(0.99999f);
    _gen0 = new cdsp_gen_sine_complex(fs, fr0);
    _gen1 = new cdsp_gen_sine_complex(fs, fr1);
    setFrs(fr0, fr1);
    _sync->setInputBlk(_dcblock, _dcblock->requestData);
    // _dcblock->setInputBlk(_filt_out, _filt_out->requestData);
    // _filt_out->setInputFunc(this, filtout_requestData);
    _dcblock->setInputFunc(this, filtout_requestData);
    _filt0->setInputFunc(this, filt0_requestData);
    _filt1->setInputFunc(this, filt1_requestData);
}

cdsp_demod_bfsk::~cdsp_demod_bfsk() {
    delete[] _filt_taps;
    delete[] _filt_out_taps;
    delete _filt0;
    delete _filt1;
    // delete _filt_out;
    delete _dcblock;
    delete _gen0;
    delete _gen1;
    delete _sync;
}

void cdsp_demod_bfsk::setFs(float fs) {
    _fs = fs;
    if(_datarate != 0) {
        _incr = _fs / (_datarate);
        // cdsp_calc_taps_lpf_rrc(_filt_out_taps, _out_taps_cnt, _incr/2.0f, 0.35f);
        // cdsp_calc_taps_lpf_float(_filt_out_taps, _out_taps_cnt, _fs, _datarate/4.0f, true);
        // cdsp_calc_taps_lpf_float(_filt_taps, _taps_cnt, _fs, _datarate, true);
        delete[] _filt_taps;
        _taps_cnt = roundf(_incr);
        _filt_taps = new float[_taps_cnt];
        for(int i = 0; i < _taps_cnt; i++) {
            _filt_taps[i] = 1.0f/((float)_taps_cnt);
        }
        _filt0->setTaps(_taps_cnt, _filt_taps);
        _filt1->setTaps(_taps_cnt, _filt_taps);
        if(_sync != NULL) {
            _sync->setInSps(_incr);
        }
    }
}

void cdsp_demod_bfsk::setFrs(float fr0, float fr1) {
    _fr0 = -fr0;
    _fr1 = -fr1; //rx samples have inverted frequency for some reason
    _gen0->setFr(_fr0);
    _gen1->setFr(_fr1);
}

void cdsp_demod_bfsk::setDataRate(float datarate) {
    _datarate = datarate;
    if(_datarate != 0) {
        setFs(_fs);
    }
}

int cdsp_demod_bfsk::requestData(void* ctx, uint8_t* data, int samples_cnt) {
    cdsp_demod_bfsk* _this = (cdsp_demod_bfsk*) ctx;
    if(!_this->_running || _this->_incr <= 0) { return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_sync->requestData(_this->_sync, _this->_out_samples, req_samples);
    if(got <= 0) { return got; }
    for(int i = 0; i < got; i++) {
        data[i] = _this->_out_samples[i] >= 0.0f; //Binary slicer
    }
    return got;
}

int cdsp_demod_bfsk::filtout_requestData(void* ctx, float* data, int samples_cnt) {
    cdsp_demod_bfsk* _this = (cdsp_demod_bfsk*) ctx;
    if(!_this->_running || _this->_incr <= 0) { return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_filt0->requestData(_this->_filt0, _this->_filt0_samples, req_samples);
    if(got <= 0) { return got; }
    got = _this->_filt1->requestData(_this->_filt1, _this->_filt1_samples, got);
    if(got <= 0) { _this->_filt1_samples_cnt = 0;return got; }
    for(int i = 0; i < got; i++) {
        //get magnitude on 0 and 1 frequencies
        float ampl0 = _this->_filt0_samples[i].mag() * CDSP_DEMOD_BFSK_FILT_GAIN;
        float ampl1 = _this->_filt1_samples[i].mag() * CDSP_DEMOD_BFSK_FILT_GAIN;
        data[i] = ampl1 - ampl0; //Demodulated value is their difference
    }
    _this->_filt1_samples_cnt = 0;
    return got;
}

int cdsp_demod_bfsk::filt0_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_demod_bfsk* _this = (cdsp_demod_bfsk*) ctx;
    if(!_this->_running || _this->_input_func==NULL || _this->_incr <= 0) {return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_input_func(_this->_func_call_ctx, _this->_in_samples, req_samples);
    if(got <= 0) {return got;}
    int gotgen = _this->_gen0->requestData(_this->_gen0, _this->_gen_samples, got);
    if(gotgen <= 0) {return gotgen;}
    if(gotgen != got) { return -1; }
    for(int i = 0; i < got; i++) {
        data[i] = _this->_gen_samples[i] * _this->_in_samples[i];
    }
    gotgen = _this->_gen1->requestData(_this->_gen1, _this->_gen_samples, got);
    if(gotgen <= 0) {return gotgen;}
    if(gotgen != got) { return -1; }
    for(int i = 0; i < got; i++) {
        _this->_filt1_in_samples[i] = _this->_gen_samples[i] * _this->_in_samples[i];
    }
    _this->_filt1_samples_cnt = got;
    return got;
}

int cdsp_demod_bfsk::filt1_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_demod_bfsk* _this = (cdsp_demod_bfsk*) ctx;
    if(!_this->_running || _this->_incr <= 0) {return -2;}
    int req_samples = std::min(_this->_filt1_samples_cnt, samples_cnt);
    for(int i = 0; i < req_samples; i++) {
        data[i] = _this->_filt1_in_samples[i];
    }
    return req_samples;
}

void cdsp_demod_bfsk::_do_start() {
    _gen0->start(true);
    _gen1->start(true);
    _filt0->start(true);
    _filt1_samples_cnt = 0;
    _filt1->start(true);
    _sync->start(true);
}

void cdsp_demod_bfsk::_do_stop() {
    _gen0->stop(true);
    _gen1->stop(true);
    _filt0->stop(true);
    _filt1->stop(true);
    _sync->stop(true);
}

