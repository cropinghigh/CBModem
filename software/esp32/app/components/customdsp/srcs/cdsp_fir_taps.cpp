#include "cdsp_fir_taps.h"

void cdsp_calc_taps_lpf_float(float* taps, int taps_cnt, float fs, float fcut, bool apply_window) {
    assert(taps_cnt >= 2);
    //using sinc and Blackman window
    //Generate sinc taps
    for(int i = 0; i < taps_cnt; i++) {
        float fr = ((float)i - (taps_cnt-1.0f)/2.0f) / fs;
        float ph = 2.0f * FL_PI * fcut * fr;
        if(ph == 0.0f) {
            taps[i] = 2.0f * (fcut/fs) * 1.0f;
        } else {
            taps[i] = 2.0f * (fcut/fs) * (sinf(ph) / ph);
        }
    }
    //Apply window
    if(apply_window) {
        for(int i = 0; i < taps_cnt; i++) {
            float w = 0.42f - 0.5f * cosf(2.0f*FL_PI*(float)i/(float)taps_cnt) + 0.08f * cosf(4.0f * FL_PI *(float)i/(float)taps_cnt);
            taps[i] = taps[i] * w;
        }
    }
}

void cdsp_calc_taps_lpf_rrc(float* taps, int taps_cnt, float sps, float alpha) {
    //sps as float to avoid many converions; assuming it's actually integer
    assert(taps_cnt >= 2);
    //Formulas from wiki
    float s2 = sqrtf(2);
    for(int i = 0; i < taps_cnt; i++) {
        float t = ((float)i - (taps_cnt-1.0f)/2.0f);
        if(t == 0) {
            taps[i] = (1.0f/sps) * (1.0f + alpha*(4.0f/FL_PI - 1.0f));
        } else if(fabsf(t) == sps/(4.0f*alpha)) {
            taps[i] = (alpha/(sps*s2)) * ((1.0f + 2.0f/FL_PI)*sinf(FL_PI/(4.0f*alpha)) + (1.0f - 2.0f/FL_PI)*cos(FL_PI/(4.0f*alpha)));
        } else {
            taps[i] = (1.0f/sps) * (sin(FL_PI * t * (1.0f-alpha) / sps) + (4.0f*alpha*t*cos(FL_PI * t * (1.0f+alpha) / sps)/sps)) / (FL_PI*t*(1.0f - powf((4.0f*alpha*t/sps), 2))/sps);
        }
    }
}
