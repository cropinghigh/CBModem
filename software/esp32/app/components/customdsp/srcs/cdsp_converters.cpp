#include "cdsp_converters.h"

cdsp_conv_int8_float::cdsp_conv_int8_float(int8_t max_in_val) {
    setMaxVal(max_in_val);
}
void cdsp_conv_int8_float::setMaxVal(int8_t max_in_val) {
    _conv_mul = 1.0f/((float)max_in_val);
}
int cdsp_conv_int8_float::requestData(void* ctx, float* data, int samples_cnt) {
    cdsp_conv_int8_float* _this = (cdsp_conv_int8_float*)ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(ind <= 0) {goto _exit;}
    for(int i = 0; i < ind; i++) {
        data[i] = (float)_this->_in_buff[i] * _this->_conv_mul;
    }
_exit:
    return ind;
}

cdsp_conv_int16_float::cdsp_conv_int16_float(int16_t max_in_val) {
    setMaxVal(max_in_val);
}
void cdsp_conv_int16_float::setMaxVal(int16_t max_in_val) {
    _conv_mul = 1.0f/((float)max_in_val);
}
int cdsp_conv_int16_float::requestData(void* ctx, float* data, int samples_cnt) {
    cdsp_conv_int16_float* _this = (cdsp_conv_int16_float*)ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(ind <= 0) {goto _exit;}
    for(int i = 0; i < ind; i++) {
        data[i] = (float)_this->_in_buff[i] * _this->_conv_mul;
    }
_exit:
    return ind;
}

cdsp_conv_float_int8::cdsp_conv_float_int8(int8_t max_in_val) {
    setMaxVal(max_in_val);
}
void cdsp_conv_float_int8::setMaxVal(int8_t max_in_val) {
    _conv_mul = max_in_val;
}
int cdsp_conv_float_int8::requestData(void* ctx, int8_t* data, int samples_cnt) {
    cdsp_conv_float_int8* _this = (cdsp_conv_float_int8*)ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(ind <= 0) {goto _exit;}
    for(int i = 0; i < ind; i++) {
        data[i] = (int8_t)roundf((float)_this->_in_buff[i] * _this->_conv_mul);
    }
_exit:
    return ind;
}

cdsp_conv_float_int16::cdsp_conv_float_int16(int16_t max_in_val) {
    setMaxVal(max_in_val);
}
void cdsp_conv_float_int16::setMaxVal(int16_t max_in_val) {
    _conv_mul = max_in_val;
}
int cdsp_conv_float_int16::requestData(void* ctx, int16_t* data, int samples_cnt) {
    cdsp_conv_float_int16* _this = (cdsp_conv_float_int16*)ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(ind <= 0) {goto _exit;}
    for(int i = 0; i < ind; i++) {
        data[i] = (int16_t)roundf((float)_this->_in_buff[i] * _this->_conv_mul);
    }
_exit:
    return ind;
}

int cdsp_conv_complex_real::requestData(void* ctx, float* data, int samples_cnt) {
    cdsp_conv_complex_real* _this = (cdsp_conv_complex_real*)ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(ind <= 0) {goto _exit;}
    for(int i = 0; i < ind; i++) {
        data[i] = _this->_in_buff[i].i;
    }
_exit:
    return ind;
}

int cdsp_conv_real_complex::requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_conv_real_complex* _this = (cdsp_conv_real_complex*)ctx;
    if(!_this->_running || _this->_input_func==NULL) {return -1;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int ind = _this->_input_func(_this->_func_call_ctx, _this->_in_buff, req_data);
    if(ind <= 0) {goto _exit;}
    for(int i = 0; i < ind; i++) {
        data[i] = _this->_in_buff[i];
    }
_exit:
    return ind;
}
