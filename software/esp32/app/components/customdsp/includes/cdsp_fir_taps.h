#pragma once
#include <math.h>
#include "cdsp_common.h"

void cdsp_calc_taps_lpf_float(float* taps, int taps_cnt, float fs, float fcut, bool apply_window);

void cdsp_calc_taps_lpf_rrc(float* taps, int taps_cnt, float sps, float alpha);

void cdsp_calc_taps_complex_fromresp(cdsp_complex_t* resp, cdsp_complex_t* taps, int resp_cnt, int taps_cnt, bool apply_window);
