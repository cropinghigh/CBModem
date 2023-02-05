#pragma once
#include <math.h>
#include "cdsp_common.h"

#define CDSP_RESAMP_TPL template<typename SPLS_T>

CDSP_RESAMP_TPL
class cdsp_rational_interpolator : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_rational_interpolator(int interp);
    void setInterpolation(int interp);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    int _interp;
    int _interp_ctr = 0;
    uint32_t _buff_avail = 0;
    uint32_t _buff_ptr = 0;
    SPLS_T _in_buf[CDSP_DEF_BUFF_SIZE];
};

CDSP_RESAMP_TPL
class cdsp_rational_decimator : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_rational_decimator(int decim);
    void setDecimation(int decim);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    uint32_t _decim;
    uint32_t _decim_ctr = 0;
    uint32_t _buff_avail = 0;
    uint32_t _buff_ptr = 0;
    SPLS_T _in_buf[CDSP_DEF_BUFF_SIZE];
};
