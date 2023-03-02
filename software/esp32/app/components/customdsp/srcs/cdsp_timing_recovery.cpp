#include "cdsp_timing_recovery.h"

CDSP_TR_TPL
cdsp_maximum_likelihood_tr<SPLS_T>::cdsp_maximum_likelihood_tr(int in_sps, float loop_bw, float damping, float relLimit) {
    setInSps(in_sps);
    setLoopBw(loop_bw);
    setDamping(damping);
    setRelLimit(relLimit);
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::setInSps(int in_sps) {
    assert(in_sps >= 2);
    _decim = in_sps;
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::setLoopBw(float loop_bw) {
    _loop_bandwidth = loop_bw;
    recalcLoopParams();
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::setDamping(float damping) {
    _loop_damp_fact = damping;
    recalcLoopParams();
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::setRelLimit(float relLimit) {
    _loop_rel_limit = relLimit;
    recalcLoopParams();
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::recalcLoopParams() {
    _loop_i_buff = 0;
    _loop_gain_p = _error_gain * (4.0f*_loop_damp_fact)/(_loop_damp_fact + 1.0f/(4.0f*_loop_damp_fact)) * _loop_bandwidth;
    _loop_gain_i = _error_gain * 4.0f/powf((_loop_damp_fact + 1.0f/(4.0f*_loop_damp_fact)), 2.0f) * powf(_loop_bandwidth, 2.0f);
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::update_shift(float shiftChange) {
    _decim_shift_frac += shiftChange;
    while(_decim_shift_frac < 0.0f || _decim_shift_frac >= 1.0f) {
        if(_decim_shift_frac < 0.0f) {
            _decim_shift_int -= 1;
            _decim_ctr += 1;
            _decim_shift_frac += 1.0f;
        }
        if(_decim_shift_frac >= 1.0f) {
            _decim_shift_int += 1;
            _decim_ctr -= 1;
            _decim_shift_frac -= 1.0f;
        }
    }
    _decim_shift_frac_taps[0] = _decim_shift_frac;
    _decim_shift_frac_taps[1] = 1.0f - _decim_shift_frac;
}

int spl_id = 0;
float s = 0;
CDSP_TR_TPL
int cdsp_maximum_likelihood_tr<SPLS_T>::requestData(void* ctx, SPLS_T* data, int samples_cnt) {
    cdsp_maximum_likelihood_tr<SPLS_T>* _this = (cdsp_maximum_likelihood_tr<SPLS_T>*) ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got_samples = 0;
    while(got_samples < req_samples) {
        if(_this->_buff_avail > 0) {
            _this->_process_in_buff[_this->_process_in_buff_ctr] = _this->_in_buf[_this->_buff_ptr];
            if(_this->_decim_ctr >= _this->_decim) {
                _this->_decim_ctr = 0;
                int id_0 = (_this->_process_in_buff_ctr + 4 - 3) % 4;
                int id_1 = (_this->_process_in_buff_ctr + 4 - 2) % 4;
                int id_2 = (_this->_process_in_buff_ctr + 4 - 1) % 4;
                int id_3 = _this->_process_in_buff_ctr;
                _this->_process_buff[0] = (_this->_process_in_buff[id_0]*_this->_decim_shift_frac_taps[1]) + (_this->_process_in_buff[id_1]*_this->_decim_shift_frac_taps[0]);
                _this->_process_buff[1] = (_this->_process_in_buff[id_1]*_this->_decim_shift_frac_taps[0]) + (_this->_process_in_buff[id_2]*_this->_decim_shift_frac_taps[1]);
                _this->_process_buff[2] = (_this->_process_in_buff[id_2]*_this->_decim_shift_frac_taps[1]) + (_this->_process_in_buff[id_3]*_this->_decim_shift_frac_taps[0]);
                float shiftChange = _this->_ted_work();
                _this->update_shift(shiftChange);
                data[got_samples] = _this->_process_buff[1];
                if constexpr (std::is_same<SPLS_T, cdsp_complex_t>::value) {
                    s = data[got_samples].i;
                } else {
                    s = data[got_samples];
                }
                got_samples++;
            }
            _this->_process_in_buff_ctr = (_this->_process_in_buff_ctr+1) % 4;
            float dbgerr = _this->_err;
            float dbgshift = ((_this->_decim_shift_int+_this->_decim_shift_frac)*1.0f)/((float)_this->_decim);
            if constexpr (std::is_same<SPLS_T, cdsp_complex_t>::value) {
                if(s == 0) {
//                     printf("%d %f - - - %f\n", spl_id, _this->_in_buf[_this->_buff_ptr].i, _this->_in_buf[_this->_buff_ptr].q);
                } else {
//                     printf("%d %f %f %f %f %f\n", spl_id, _this->_in_buf[_this->_buff_ptr].i, s, dbgshift, dbgerr, _this->_in_buf[_this->_buff_ptr].q);
                    s = 0;
                }
            } else {
                if(s == 0) {
//                     printf("%d %f - - -\n", spl_id, _this->_in_buf[_this->_buff_ptr]);
                } else {
//                     printf("%d %f %f %f %f\n", spl_id, _this->_in_buf[_this->_buff_ptr], s, dbgshift, dbgerr);
                    s = 0;
                }
            }
            
            // wf.write((char *) &_this->_in_buf[_this->_buff_ptr], sizeof(float));
            // wf.write((char *) &dbgerr, sizeof(float));
            // wf.write((char *) &dbgshift, sizeof(float));
            // wf.write((char *) &s, sizeof(float));
            // wf.flush();
            spl_id++;
            _this->_buff_ptr++;
            _this->_buff_avail--;
            _this->_decim_ctr++;
        } else {
            //Request new buffer
            int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buf, std::min(req_samples*_this->_decim, (int32_t)CDSP_DEF_BUFF_SIZE));
            if(ind <= 0) {return ind;}
            _this->_buff_avail = ind;
            _this->_buff_ptr = 0;
        }
    }
    return got_samples;
}

CDSP_TR_TPL
float cdsp_maximum_likelihood_tr<SPLS_T>::_ted_work() {
    //Using samples in _process_buff
    SPLS_T derivative = _process_buff[2] - _process_buff[0];
    float error;
    if constexpr (std::is_same<SPLS_T, cdsp_complex_t>::value) {
        error = (sgn(_process_buff[1].i) * derivative.i) + (sgn(_process_buff[1].q) * derivative.q);
    } else {
        // error = _process_buff[1]*derivative;
        error = sgn(_process_buff[1])*(derivative);
    }
    error = std::erf(error);
    _err = error;
    _loop_i_buff += error*_loop_gain_i;
    if(_loop_i_buff >= _loop_rel_limit) {
        _loop_i_buff = _loop_rel_limit;
    } else if(_loop_i_buff <= -_loop_rel_limit) {
        _loop_i_buff = -_loop_rel_limit;
    }
    return error*_loop_gain_p + _loop_i_buff;
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::_do_start() {
    _decim_ctr = 7;
    _decim_shift_int = 0;
    _decim_shift_frac = 0.0f;
    _decim_shift_frac_taps[0] = _decim_shift_frac;
    _decim_shift_frac_taps[1] = 1.0f - _decim_shift_frac;
    _buff_avail = 0;
    _buff_ptr = 0;
    _process_in_buff_ctr = 0;
    _loop_i_buff = 0.0f;
    _process_in_buff[0] = 0.0f;
    _process_in_buff[1] = 0.0f,
    _process_in_buff[2] = 0.0f;
}

CDSP_TR_TPL
void cdsp_maximum_likelihood_tr<SPLS_T>::_do_stop() {
    
}

//Add types to compile if required
template class cdsp_maximum_likelihood_tr<float>;
template class cdsp_maximum_likelihood_tr<cdsp_complex_t>;
