#pragma once
#include <math.h>
#include "stdio.h"

#include "cdsp_common.h"

#define CDSP_TR_TPL template<typename SPLS_T>

CDSP_TR_TPL
class cdsp_maximum_likelihood_tr : public cdsp_block<SPLS_T, SPLS_T> {
public:
    cdsp_maximum_likelihood_tr(int in_sps, float loop_bw, float damping, float relLimit);
    void setInSps(int in_sps);
    void setLoopBw(float loop_bw);
    void setDamping(float damping);
    void setRelLimit(float relLimit);
    inline void recalcLoopParams();
    inline void update_shift(float shiftChange);
    static int IRAM_ATTR requestData(void* ctx, SPLS_T* data, int samples_cnt);

private:
    int32_t _decim;
    int32_t _decim_ctr = 0;
    SPLS_T _in_buf[CDSP_DEF_BUFF_SIZE];
    uint32_t _buff_avail = 0;
    uint32_t _buff_ptr = 0;

    int32_t _decim_shift_int;
    float _decim_shift_frac;
    float _decim_shift_frac_taps[4]; //Using linear interpolation
    int32_t _decim_shift_ctr = 0;
    SPLS_T _process_buff[3];
    SPLS_T _process_in_buff[4];

    float _error_gain = 100.0f;
    float _loop_gain_p = 0.0f;
    float _loop_gain_i = 0.0f;
    float _loop_i_buff = 0.0f;
    float _loop_bandwidth = 0.04f;
    float _loop_damp_fact = 2.0f;
    float _loop_rel_limit = 0.1f;
    float _err = 0;

    inline float _ted_work();
    void _do_start() override;
    void _do_stop() override;
};
