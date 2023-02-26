#ifndef H_CDSP_RATIONAL_RESAMPLERS
#define H_CDSP_RATIONAL_RESAMPLERS
#include <math.h>
#include "cdsp_common.h"

#define CDSP_RESAMP_TPL template<typename SPLS_T>

CDSP_RESAMP_TPL
class cdsp_rational_interpolator : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_rational_interpolator(int32_t interp);
    void setInterpolation(int32_t interp);
    void setShift(int32_t out_spls);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    int32_t _interp;
    int32_t _interp_ctr = 0;
    uint32_t _buff_avail = 0;
    uint32_t _buff_ptr = 0;
    SPLS_T _in_buf[CDSP_DEF_BUFF_SIZE];

    void _do_start() override;
    void _do_stop() override;
};

CDSP_RESAMP_TPL
class cdsp_rational_decimator : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_rational_decimator(int32_t decim);
    void setDecimation(int32_t decim);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    int32_t _decim;
    int32_t _decim_ctr = 0;
    uint32_t _buff_avail = 0;
    uint32_t _buff_ptr = 0;
    SPLS_T _in_buf[CDSP_DEF_BUFF_SIZE];

    void _do_start() override;
    void _do_stop() override;
};
#endif
