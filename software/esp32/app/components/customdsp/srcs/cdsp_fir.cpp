#include "cdsp_fir.h"

CDSP_FIR_TPL
cdsp_fir<TAPS_T, SPLS_T>::cdsp_fir(int taps_cnt, TAPS_T* taps) {
    setTaps(taps_cnt, taps);
}

CDSP_FIR_TPL
cdsp_fir<TAPS_T, SPLS_T>::~cdsp_fir() {
    if(_inbuff != nullptr) {
        delete[](_inbuff);
    }
}

CDSP_FIR_TPL
void cdsp_fir<TAPS_T, SPLS_T>::setTaps(int new_taps_cnt, TAPS_T* taps) {
    if(cdsp_block<SPLS_T, SPLS_T>::_running) {return;}
    _taps_cnt = new_taps_cnt;
    _taps = taps;
    if(_inbuff != nullptr) {
        delete[](_inbuff);
        _inbuff = nullptr;
    }
    _real_buffsize = CDSP_DEF_BUFF_SIZE + _taps_cnt;
    _inbuff = new SPLS_T[_real_buffsize];
    for(int i = 0; i < _real_buffsize; i++) {
        if constexpr (std::is_same<SPLS_T, cdsp_complex_t>::value) {
            _inbuff[i].i = 0;
            _inbuff[i].q = 0;
        } else {
            _inbuff[i] = 0;
        }
    }
}

CDSP_FIR_TPL
int cdsp_fir<TAPS_T, SPLS_T>::requestData(void* ctx, SPLS_T* data, int samples_cnt) {
    cdsp_fir<TAPS_T, SPLS_T>* _this = (cdsp_fir<TAPS_T, SPLS_T>*) ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int ind = _this->_input_func(_this->_func_call_ctx, &_this->_inbuff[_this->_taps_cnt], req_data);
    if(ind <= 0) {goto _exit;}
    _this->_do_work(_this->_inbuff, data, ind);
    for(int i = 0; i < _this->_taps_cnt; i++) {
        _this->_inbuff[i] = _this->_inbuff[ind+i];
    }
_exit:
    return ind;
}

CDSP_FIR_TPL
void cdsp_fir<TAPS_T, SPLS_T>::_do_work(SPLS_T* in, SPLS_T* out, int cnt) {
    for(int i = 0; i < cnt; i++) {
        out[i] = 0;
        //Convolve values with taps
        for(int k = 0; k < _taps_cnt; k++) {
            out[i] = out[i] + (in[i+k] * _taps[k]);
        }
    }
}

//Add types to compile if required
template class cdsp_fir<float, float>;
template class cdsp_fir<float, cdsp_complex_t>;
template class cdsp_fir<cdsp_complex_t, cdsp_complex_t>;
