#pragma once
#include <math.h>

#include "cdsp_common.h" 

#define CDSP_AGC_TPL template<typename SPLS_T>
#define CDSP_AGC_REF 1.0f

CDSP_AGC_TPL
class cdsp_agc : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_agc(float rate);
    void setRate(float rate);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    SPLS_T _in_data[CDSP_DEF_BUFF_SIZE];
    float _rate = 0;
    float _mul = 1.0f;

    void _do_start() override;
    void _do_stop() override;
};

CDSP_AGC_TPL
class cdsp_dcblock : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_dcblock(float rate);
    void setRate(float rate);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    SPLS_T _in_data[CDSP_DEF_BUFF_SIZE];
    SPLS_T _prev_in;
    SPLS_T _prev_out;
    float _rate = 0;

    void _do_start() override;
    void _do_stop() override;
};
