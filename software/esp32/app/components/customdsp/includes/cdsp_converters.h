#pragma once
#include <math.h>
#include "cdsp_common.h"

class cdsp_conv_int8_float : public cdsp_block<int8_t, float> {
public:
    cdsp_conv_int8_float(int8_t max_in_val);
    void setMaxVal(int8_t max_in_val);
    static int IRAM_ATTR requestData(void* ctx, float* data, int samples_cnt);

private:
    float _conv_mul;
    int8_t _in_buff[CDSP_DEF_BUFF_SIZE];
};

class cdsp_conv_int16_float : public cdsp_block<int16_t, float> {
public:
    cdsp_conv_int16_float(int16_t max_in_val);
    void setMaxVal(int16_t max_in_val);
    static int IRAM_ATTR requestData(void* ctx, float* data, int samples_cnt);

private:
    float _conv_mul;
    int16_t _in_buff[CDSP_DEF_BUFF_SIZE];
};

class cdsp_conv_float_int8 : public cdsp_block<float, int8_t> {
public:
    cdsp_conv_float_int8(int8_t max_in_val);
    void setMaxVal(int8_t max_in_val);
    static int IRAM_ATTR requestData(void* ctx, int8_t* data, int samples_cnt);

private:
    float _conv_mul;
    float _in_buff[CDSP_DEF_BUFF_SIZE];
};


class cdsp_conv_float_int16 : public cdsp_block<float, int16_t> {
public:
    cdsp_conv_float_int16(int16_t max_in_val);
    void setMaxVal(int16_t max_in_val);
    static int IRAM_ATTR requestData(void* ctx, int16_t* data, int samples_cnt);

private:
    float _conv_mul;
    float _in_buff[CDSP_DEF_BUFF_SIZE];
};


class cdsp_conv_complex_real : public cdsp_block<cdsp_complex_t, float> {
public:
    static int IRAM_ATTR requestData(void* ctx, float* data, int samples_cnt);

private:
    cdsp_complex_t _in_buff[CDSP_DEF_BUFF_SIZE];
};

class cdsp_conv_real_complex : public cdsp_block<float, cdsp_complex_t> {
public:
    static int IRAM_ATTR requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

private:
    float _in_buff[CDSP_DEF_BUFF_SIZE];
};
