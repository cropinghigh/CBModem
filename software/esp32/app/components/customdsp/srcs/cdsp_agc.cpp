#include "cdsp_agc.h"

CDSP_AGC_TPL
cdsp_agc<SPLS_T>::cdsp_agc(float rate) {
    setRate(rate);
}

CDSP_AGC_TPL
void cdsp_agc<SPLS_T>::setRate(float rate) {
    _rate = rate;
}

CDSP_AGC_TPL
int cdsp_agc<SPLS_T>::requestData(void* ctx, SPLS_T* data, int samples_cnt) {
    cdsp_agc<SPLS_T>* _this = (cdsp_agc<SPLS_T>*) ctx;
    if(!_this->_running || _this->_input_func==NULL) { return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_input_func(_this->_func_call_ctx, _this->_in_data, req_samples);
    if(got <= 0) { return got; }
    for(int i = 0; i < got; i++) {
        float ampl;
        data[i] = _this->_in_data[i] * _this->_mul;
        if constexpr (std::is_same<SPLS_T, cdsp_complex_t>::value) {
            ampl = fabsf(data[i].mag());
        } else {
            ampl = fabsf(data[i]);
        }
        _this->_mul += (CDSP_AGC_REF-ampl)*_this->_rate;
    }
    return got;
}

CDSP_AGC_TPL
void cdsp_agc<SPLS_T>::_do_start() {
    _mul = 1.0f;
}

CDSP_AGC_TPL
void cdsp_agc<SPLS_T>::_do_stop() {
    
}

CDSP_AGC_TPL
cdsp_dcblock<SPLS_T>::cdsp_dcblock(float rate) {
    setRate(rate);
}

CDSP_AGC_TPL
void cdsp_dcblock<SPLS_T>::setRate(float rate) {
    _rate = rate;
}

CDSP_AGC_TPL
int cdsp_dcblock<SPLS_T>::requestData(void* ctx, SPLS_T* data, int samples_cnt) {
    cdsp_dcblock<SPLS_T>* _this = (cdsp_dcblock<SPLS_T>*) ctx;
    if(!_this->_running || _this->_input_func==NULL) { return -2;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = _this->_input_func(_this->_func_call_ctx, _this->_in_data, req_samples);
    if(got <= 0) { return got; }
    for(int i = 0; i < got; i++) {
        data[i] = _this->_in_data[i] - _this->_prev_in + _this->_prev_out*_this->_rate;
        _this->_prev_in = _this->_in_data[i];
        _this->_prev_out = data[i];
    }
    return got;
}

CDSP_AGC_TPL
void cdsp_dcblock<SPLS_T>::_do_start() {
    _prev_in = 0;
    _prev_out = 0;
}

CDSP_AGC_TPL
void cdsp_dcblock<SPLS_T>::_do_stop() {
    
}

//Add types to compile if required
template class cdsp_agc<float>;
template class cdsp_agc<cdsp_complex_t>;
template class cdsp_dcblock<float>;
template class cdsp_dcblock<cdsp_complex_t>;
