#include "cdsp_rational_resamplers.h"

#include <stdio.h>

CDSP_RESAMP_TPL
cdsp_rational_interpolator<SPLS_T>::cdsp_rational_interpolator(int interp) {
    _interp = interp;
}

CDSP_RESAMP_TPL
void cdsp_rational_interpolator<SPLS_T>::setInterpolation(int interp) {
    _interp = interp;
    _interp_ctr = 0;
}

CDSP_RESAMP_TPL
int cdsp_rational_interpolator<SPLS_T>::requestData(void* ctx, SPLS_T* data, int samples_cnt) {
    cdsp_rational_interpolator<SPLS_T>* _this = (cdsp_rational_interpolator<SPLS_T>*) ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got_samples = 0;
    float avga = 0.0f;
    while(got_samples < req_samples) {
        if(_this->_buff_avail > 0) {
            data[got_samples] = _this->_in_buf[_this->_buff_ptr];
            avga += data[got_samples];
            got_samples++;
            _this->_interp_ctr++;
            if(_this->_interp_ctr >= _this->_interp) {
                _this->_buff_ptr++;
                _this->_buff_avail--;
                _this->_interp_ctr = 0;
            }
        } else {
            int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buf, std::max(req_samples/_this->_interp, 1));
            if(ind <= 0) {return ind;}
            _this->_buff_avail = ind;
            _this->_buff_ptr = 0;
        }
    }
    // printf("got %d, avg a: %f\n", got_samples, avga);
    return got_samples;
}

CDSP_RESAMP_TPL
cdsp_rational_decimator<SPLS_T>::cdsp_rational_decimator(int decim) {
    _decim = decim;
}

CDSP_RESAMP_TPL
void cdsp_rational_decimator<SPLS_T>::setDecimation(int decim) {
    _decim = decim;
    _decim_ctr = 0;
}

CDSP_RESAMP_TPL
int cdsp_rational_decimator<SPLS_T>::requestData(void* ctx, SPLS_T* data, int samples_cnt) {
    cdsp_rational_decimator<SPLS_T>* _this = (cdsp_rational_decimator<SPLS_T>*) ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_samples = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got_samples = 0;
    while(got_samples < req_samples) {
        if(_this->_buff_avail > (_this->_decim - _this->_decim_ctr)) {
            _this->_buff_ptr += (_this->_decim - _this->_decim_ctr);
            _this->_buff_avail -= (_this->_decim - _this->_decim_ctr);
            //Read sample and move further on the buffer
            data[got_samples] = _this->_in_buf[_this->_buff_ptr];
            got_samples++;
            _this->_decim_ctr = 0;
        } else {
            //Request new buffer
            _this->_decim_ctr += _this->_buff_avail; //Count remaining unused data in buffer
            int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buf, CDSP_DEF_BUFF_SIZE);
            if(ind <= 0) {return ind;}
            _this->_buff_avail = ind;
            _this->_buff_ptr = 0;
        }
    }
    return got_samples;
}

//Add types to compile if required
template class cdsp_rational_interpolator<float>;
// template class cdsp_rational_interpolator<cdsp_complex_t>;

template class cdsp_rational_decimator<float>;
template class cdsp_rational_decimator<cdsp_complex_t>;

