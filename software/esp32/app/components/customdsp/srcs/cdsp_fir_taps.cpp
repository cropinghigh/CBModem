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
    float sum = 0.0f;
    for(int i = 0; i < taps_cnt; i++) {
        sum += taps[i];
    }
    float div = 1.0f/sum;
    for(int i = 0; i < taps_cnt; i++) {
        taps[i] = taps[i] * div;
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
    float sum = 0.0f;
    for(int i = 0; i < taps_cnt; i++) {
        sum += taps[i];
    }
    float div = 1.0f/sum;
    for(int i = 0; i < taps_cnt; i++) {
        taps[i] = taps[i] * div;
    }
}

void cdsp_calc_taps_complex_fromresp(cdsp_complex_t* resp, cdsp_complex_t* taps, int resp_cnt, int taps_cnt, bool apply_window) {
    assert(taps_cnt >= 2);
    //Using simple DFT
    for(int i = 0; i < taps_cnt; i++) {
        taps[i] = 0;
        // float frFrac = ((float)(i-((taps_cnt-1)/2.0f)))/((float)taps_cnt);
        // float frFrac = (float)i/(float)taps_cnt;
        float fr = ((float)(i-((taps_cnt-1)/2.0f)));
        for(int m = 0; m < resp_cnt; m++) {
            float ph = 2.0f*FL_PI*fr*((float)m)/((float)(resp_cnt));
            float w = 1.0f;
            if(apply_window) {
                w = 0.42f - 0.5f * cosf(2.0f*FL_PI*(float)i/(float)(taps_cnt-1)) + 0.08f * cosf(4.0f * FL_PI *(float)i/(float)(taps_cnt-1));
            }
            // taps[i].i += resp[m].i * cosf(ph) * w;
            // taps[i].q += resp[m].q * sinf(ph) * w;
            cdsp_complex_t p;
            p.i = cosf(ph) * w;
            p.q = -sinf(ph) * w;
            taps[i] = taps[i] + (resp[(m+(resp_cnt/2)) % resp_cnt] * p);
            // printf("ph %f w %f fr %f pi %f pq %f outi %f outq %f\n", ph, w, frFrac, p.i, p.q, taps[i].i, taps[i].q);
        }
        taps[i].i /= (float)resp_cnt;
        taps[i].q /= (float)resp_cnt;
        taps[i].q = 0.0f;
    }
}
