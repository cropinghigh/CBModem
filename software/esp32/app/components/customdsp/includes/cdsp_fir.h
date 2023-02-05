#pragma once
#include <math.h>
#include "cdsp_common.h"

#define CDSP_FIR_TPL template<typename TAPS_T, typename SPLS_T>

CDSP_FIR_TPL
class cdsp_fir : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_fir(int taps_cnt, TAPS_T* taps);
    ~cdsp_fir();
    void setTaps(int new_taps_cnt, TAPS_T* taps);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    int _taps_cnt;
    TAPS_T* _taps;
    int _real_buffsize;
    SPLS_T* _inbuff = nullptr;

    inline void _do_work(SPLS_T* in, SPLS_T* out, int cnt);
};
