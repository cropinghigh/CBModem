#include "cdsp_modulator.h" 

#include "stdio.h"

cdsp_mod_bfsk::cdsp_mod_bfsk(float fs, float fr0, float fr1, float datarate, int taps_cnt) {
    _taps_cnt = taps_cnt;
    _gen = new cdsp_gen_sine_complex(fs, fr0);
    setFs(fs);
    setFrs(fr0, fr1);
    setDataRate(datarate);
    _filt_taps = new float[_taps_cnt];
    float maxfreq = std::max(fabsf(fr0), fabsf(fr1)) + datarate;
    cdsp_calc_taps_lpf_float(_filt_taps, _taps_cnt, fs, maxfreq, true);
    _filt = new cdsp_fir<float, cdsp_complex_t>(_taps_cnt, _filt_taps);
    _filt->setInputFunc(this, filt_requestData);
}

cdsp_mod_bfsk::~cdsp_mod_bfsk() {
    delete[] _filt_taps;
    delete _filt;
    delete _gen;
}

void cdsp_mod_bfsk::setFs(float fs) {
    _fs = fs;
    _gen->setFs(_fs);
    if(_fs != 0) {
        _incr = _datarate / _fs;
        // printf("dr %f fs %f incr %f\n", _datarate, _fs, _incr);
    }
}

void cdsp_mod_bfsk::setFrs(float fr0, float fr1) {
    _fr0 = fr0;
    _fr1 = fr1;
}

void cdsp_mod_bfsk::setDataRate(float datarate) {
    _datarate = datarate;
    if(_fs != 0) {
        setFs(_fs);
    }
}

int cdsp_mod_bfsk::requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_mod_bfsk* _this = (cdsp_mod_bfsk*) ctx;
    if(!_this->_running) {return -2;}
    // return _this->_filt->requestData(_this->_filt, data, samples_cnt); Works better without FIR
    return _this->filt_requestData(_this, data, samples_cnt);
}

int cdsp_mod_bfsk::filt_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_mod_bfsk* _this = (cdsp_mod_bfsk*) ctx;
    if(!_this->_running || _this->_input_func==NULL || _this->_incr <= 0) { return -2;}
    int ret = 0;
    while(ret < samples_cnt) {
        int remaining_spls = std::min((int)roundf((1.0f / _this->_incr) * (1.0f - _this->_ctr)), samples_cnt-ret);
        // printf("remain %f, want %d, ctr %f, req %d\n", floorf((1.0f / _this->_incr) * (1.0f - _this->_ctr)), samples_cnt-ret, _this->_ctr, remaining_spls);
        int gotmain = _this->_gen->requestData(_this->_gen, &data[ret], remaining_spls);
        if(gotmain <= 0) {return gotmain;}
        ret += gotmain;
        _this->_ctr += gotmain * _this->_incr;
        if(_this->_ctr >= 0.99f) {
            // printf("%d %d %d %d %f %f\n", remaining_spls, gotmain, samples_cnt, ret, _this->_incr, _this->_ctr);
            if(_this->_preamble_ctr < MOD_PREAMBLE_SIZE) {
                _this->_gen->setFr((_this->_preamble_ctr % 2) ? _this->_fr1 : _this->_fr0);
                _this->_preamble_ctr++;
            } else {
                int got = _this->_input_func(_this->_func_call_ctx, &_this->_curr_data, 1);
                if(got != 1) {return got;}
                if(_this->_curr_data) {
                    _this->_gen->setFr(_this->_fr1);
                } else {
                    _this->_gen->setFr(_this->_fr0);
                }
            }
            _this->_ctr = 0.0f;
        }
    }
    return ret;
}

void cdsp_mod_bfsk::_do_start()  {
    _ctr = 0;
    _preamble_ctr = 0;
    _gen->start(true);
    _filt->start(true);
}

void cdsp_mod_bfsk::_do_stop() {
    _gen->stop(true);
    _filt->stop(true);
}


cdsp_mod_mfsk::cdsp_mod_mfsk(float fs, int frcnt, float* frs, float datarate, int taps_cnt) {
    _taps_cnt = taps_cnt;
    _frcnt = frcnt;
    _gen = new cdsp_gen_sine_complex(fs, frs[0]);
    setFs(fs);
    setFrs(_frcnt, frs);
    setDataRate(datarate);
    _filt_taps = new float[_taps_cnt];
    float maxfr = -100000;
    for(int i = 0; i < frcnt; i++) {
        maxfr = std::max(maxfr, frs[i]);
    }
    float maxfreq = maxfr + datarate;
    cdsp_calc_taps_lpf_float(_filt_taps, _taps_cnt, fs, maxfreq, true);
    _filt = new cdsp_fir<float, cdsp_complex_t>(_taps_cnt, _filt_taps);
    _filt->setInputFunc(this, filt_requestData);
}

cdsp_mod_mfsk::~cdsp_mod_mfsk() {
    delete[] _filt_taps;
    delete[] _frs;
    delete _filt;
    delete _gen;
}

void cdsp_mod_mfsk::setFs(float fs) {
    _fs = fs;
    _gen->setFs(_fs);
    if(_fs != 0) {
        _incr = _datarate / _fs;
    }
}

void cdsp_mod_mfsk::setFrs(int frcnt, float* frs) {
    _frcnt = frcnt;
    _frbits = floorf(log2f(frcnt));
    assert(_frbits < 256 && _frbits >= 1);
    if(_frs != NULL) {
        delete[] _frs;
    }
    _frs = new float[_frcnt];
    for(int i = 0; i < _frcnt; i++) {
        _frs[i] = frs[i];
    }
}

void cdsp_mod_mfsk::setDataRate(float datarate) {
    _datarate = datarate;
    if(_fs != 0) {
        _incr = _datarate / _fs;
    }
}

int cdsp_mod_mfsk::requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_mod_mfsk* _this = (cdsp_mod_mfsk*) ctx;
    if(!_this->_running) {return -2;}
    // return _this->_filt->requestData(_this->_filt, data, samples_cnt); Works much better without FIR
    return _this->filt_requestData(_this, data, samples_cnt);
}

int cdsp_mod_mfsk::filt_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_mod_mfsk* _this = (cdsp_mod_mfsk*) ctx;
    if(!_this->_running || _this->_input_func==NULL || _this->_incr <= 0) { return -2;}
    int ret = 0;
    while(ret < samples_cnt) {
        int remaining_spls = std::min(int((1.0f / _this->_incr) - _this->_ctr), samples_cnt-ret);
        int gotmain = _this->_gen->requestData(_this->_gen, &data[ret], remaining_spls);
        if(gotmain <= 0) {return gotmain;}
        ret += gotmain;
        _this->_ctr += gotmain * _this->_incr;
        if(_this->_ctr >= 1.0f) {
            if(_this->_preamble_ctr < MOD_PREAMBLE_SIZE) {
                _this->_gen->setFr((_this->_preamble_ctr % 2) ? _this->_frs[_this->_frcnt-1] : _this->_frs[0]);
                _this->_preamble_ctr++;
            } else {
                int got = _this->_input_func(_this->_func_call_ctx, _this->_curr_data, _this->_frbits);
                if(got <= 0) {return got;}
                uint8_t newfr = 0;
                for(int i = 0; i < got; i++) {
                    newfr |= ((_this->_curr_data[i] & 0b1) << i);
                }
                _this->_gen->setFr(_this->_frs[newfr]);
            }
            _this->_ctr = 0.0f;
        }
    }
    return ret;
}

void cdsp_mod_mfsk::_do_start() {
    _ctr = 0;
    _preamble_ctr = 0;
    _gen->start(true);
    _filt->start(true);
}

void cdsp_mod_mfsk::_do_stop() {
    _gen->stop(true);
    _filt->stop(true);
}


cdsp_demod_bfsk::cdsp_demod_bfsk(float fs, float fr0, float fr1, float datarate, float sync_loop_bw, float sync_damping, float sync_relLimit) {
    setFs(fs);
    setFrs(fr0, fr1);
    setDataRate(datarate);
    _filt = new cdsp_fir<float, float>(_taps_cnt, _filt_taps);
    _sync = new cdsp_maximum_likelihood_tr<float>(_incr, sync_loop_bw, sync_damping, sync_relLimit);
    _dcblock = new cdsp_dcblock<float>(1.0f);
    _sync->setInputBlk(_dcblock, _dcblock->requestData);
    _dcblock->setInputBlk(_filt, _filt->requestData);
    _filt->setInputFunc(this, dcb_requestData);
}

cdsp_demod_bfsk::~cdsp_demod_bfsk() {
    delete _filt;
    delete _dcblock;
    delete _sync;
    delete[] _filt_taps;
}

void cdsp_demod_bfsk::setFs(float fs) {
    _fs = fs;
    if(_datarate != 0) {
        _incr = _fs / (_datarate);
        _taps_cnt = roundf(_incr)*2;
        if(_filt_taps != NULL) {
            delete[] _filt_taps;
        }
        _filt_taps = new float[_taps_cnt];
        // if(_filt_buff != NULL) {
            // delete[] _filt_buff;
        // }
        // _filt_buff = new float [_taps_cnt];
        // for(int i = 0; i < _taps_cnt; i++) {
        //     _filt_taps[i] = 1.0f/((float)_taps_cnt);
        //     _filt_buff[i] = 0.0f;
        // }
        cdsp_calc_taps_lpf_float(_filt_taps, _taps_cnt, _fs, _datarate, true);
        if(_filt != NULL) {
            _filt->setTaps(_taps_cnt, _filt_taps);
        }
        if(_sync != NULL) {
            _sync->setInSps(_incr);
        }
    }
}

void cdsp_demod_bfsk::setFrs(float fr0, float fr1) {
    _fr0 = -fr0 / _fs;
    _fr1 = -fr1 / _fs; //rx samples have inverted frequency for some reason
    float high = std::max(fabsf(_fr0), fabsf(_fr1));
    _syncmul = 1.0f/high;
    _syncdiv = high;
    if(_datarate != 0) {
        setFs(_fs);
    }
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
        float err0 = fabsf(_this->_fr0 - _this->_out_samples[i]*_this->_syncdiv);
        float err1 = fabsf(_this->_fr1 - _this->_out_samples[i]*_this->_syncdiv);
        data[i] = err1 < err0 ? 1 : 0; //Binary slicer
    }
    return got;
}

int cdsp_demod_bfsk::dcb_requestData(void* ctx, float* data, int samples_cnt) {
    cdsp_demod_bfsk* _this = (cdsp_demod_bfsk*) ctx;
    if(!_this->_running || _this->_input_func==NULL || _this->_incr <= 0) {return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_input_func(_this->_func_call_ctx, _this->_in_samples, req_samples);
    if(got <= 0) {return got;}
    for(int i = 0; i < got; i++) {
        float fr_rel = (_this->_in_samples[i]*_this->_prev_spl.conj()).ph() * (1.0f/(2.0f*FL_PI)); //Interesting f/fs detector, stolen from gnuradio quadrature demod; works SIGNIFICANTLY better than coherent multiplication&filtering
        data[i] = fr_rel * _this->_syncmul;
//         _this->_filt_buff[_this->_filt_ptr] = fr_rel;
//         _this->_filt_ptr = (_this->_filt_ptr + 1) % _this->_taps_cnt;
//         data[i] = 0;
//         for(int k = 0; k < _this->_taps_cnt; k++) {
//            
//             data[i] += _this->_filt_buff[k] * _this->_filt_taps[k];
//         }
//         data[i] *= _this->_syncmul;
        _this->_prev_spl = _this->_in_samples[i];
    }
    return got;
}

void cdsp_demod_bfsk::_do_start() {
    _filt_ptr = 0;
    _sync->start(true);
}

void cdsp_demod_bfsk::_do_stop() {
    _sync->stop(true);
}


cdsp_demod_mfsk::cdsp_demod_mfsk(float fs, int frcnt, float* frs, float datarate, float sync_loop_bw, float sync_damping, float sync_relLimit) {
    setFs(fs);
    setFrs(frcnt, frs);
    setDataRate(datarate);
    _filt = new cdsp_fir<float, float>(_taps_cnt, _filt_taps);
    _sync = new cdsp_maximum_likelihood_tr<float>(_incr, sync_loop_bw, sync_damping, sync_relLimit);
    _dcblock = new cdsp_dcblock<float>(1.0f);
    _sync->setInputBlk(_dcblock, _dcblock->requestData);
    _dcblock->setInputBlk(_filt, _filt->requestData);
    _filt->setInputFunc(this, dcb_requestData);
}

cdsp_demod_mfsk::~cdsp_demod_mfsk() {
    delete[] _frs;
    delete[] _filt_taps;
    delete _dcblock;
    delete _sync;
    delete _filt;
}

void cdsp_demod_mfsk::setFs(float fs) {
    _fs = fs;
    if(_datarate != 0) {
        _incr = _fs / (_datarate);
        _taps_cnt = roundf(_incr);
        _filt_taps = new float[_taps_cnt];
        cdsp_calc_taps_lpf_float(_filt_taps, _taps_cnt, _fs, _datarate, true);
        if(_filt != NULL) {
            _filt->setTaps(_taps_cnt, _filt_taps);
        }
        if(_sync != NULL) {
            _sync->setInSps(_incr);
        }
    }
}

void cdsp_demod_mfsk::setFrs(int frcnt, float* frs) {
    _frcnt = frcnt;
    _frbits = floorf(log2f(frcnt));
    assert(_frbits < 256 && _frbits >= 1);
    if(_frs != NULL) {
        delete[] _frs;
    }
    _frs = new float[_frcnt];
    float high = 0.0f;
    for(int i = 0; i < _frcnt; i++) {
        _frs[i] = -frs[i] / _fs; //rx samples have inverted frequency for some reason
        if(fabsf(_frs[i]) > high) {
            high = fabsf(_frs[i]);
        }
    }
    _syncmul = 1.0f/high;
    _syncdiv = high;
    if(_datarate != 0) {
        setFs(_fs);
    }
}

void cdsp_demod_mfsk::setDataRate(float datarate) {
    _datarate = datarate;
    if(_datarate != 0) {
        setFs(_fs);
    }
}

int cdsp_demod_mfsk::requestData(void* ctx, uint8_t* data, int samples_cnt) {
    cdsp_demod_mfsk* _this = (cdsp_demod_mfsk*) ctx;
    if(!_this->_running || _this->_incr <= 0) { return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got_samples = 0;
    while(got_samples < req_samples) {
        if(_this->_outbuff_data > 0) {
            data[got_samples] = (_this->_outbuff[_this->_outbuff_ptr] & (1 << _this->_outbuff_bit)) >> _this->_outbuff_bit;
            got_samples++;
            _this->_outbuff_bit++;
            if(_this->_outbuff_bit >= _this->_frbits) {
                _this->_outbuff_bit = 0;
                _this->_outbuff_data--;
                _this->_outbuff_ptr++;
            }
        } else {
            int got = _this->_sync->requestData(_this->_sync, _this->_out_samples, std::max(req_samples/_this->_frbits, 1));
            if(got <= 0) { return (got_samples == 0) ? got : got_samples; }
            for(int i = 0; i < got; i++) {
                float lasterr = 10000.0f;
                for(int k = 0; k < _this->_frcnt; k++) {
                    float frtarget = _this->_frs[k];
                    float err = fabsf(frtarget - _this->_out_samples[i]*_this->_syncdiv);
                    if(err < lasterr) {
                        _this->_outbuff[i] = k;
                        lasterr = err;
                    }
                }
            }
            _this->_outbuff_data = got;
            _this->_outbuff_ptr = 0;
        }
    }
    return got_samples;
}

int cdsp_demod_mfsk::dcb_requestData(void* ctx, float* data, int samples_cnt) {
    cdsp_demod_mfsk* _this = (cdsp_demod_mfsk*) ctx;
    if(!_this->_running || _this->_input_func==NULL || _this->_incr <= 0) {return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_input_func(_this->_func_call_ctx, _this->_in_samples, req_samples);
    if(got <= 0) {return got;}
    for(int i = 0; i < got; i++) {
        float fr_rel = (_this->_in_samples[i]*_this->_prev_spl.conj()).ph() * (1.0f/(2.0f*FL_PI)); //Interesting f/fs detector, stolen from gnuradio quadrature demod; works SIGNIFICANTLY better than coherent multiplication&filtering
        data[i] = fr_rel * _this->_syncmul;
        _this->_prev_spl = _this->_in_samples[i];
    }
    return got;
}


void cdsp_demod_mfsk::_do_start() {
    _outbuff_data = 0;
    _outbuff_ptr = 0;
    _outbuff_bit = 0;
    _sync->start(true);
}

void cdsp_demod_mfsk::_do_stop() {
    _sync->stop(true);
}


