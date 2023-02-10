#pragma once
#include <math.h>

#include "cdsp_common.h"

class cdsp_gen_sine_complex : public cdsp_block<void, cdsp_complex_t> {
public:
    cdsp_gen_sine_complex(float fs, float fr);
    void setFs(float fs);
    void setFr(float fr);
    static int IRAM_ATTR requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);
private:
    float _phase = 0;
    float _fs = 1;
    float _fr = 0;
    float _incr = 1;
};

class cdsp_gen_sine_float : public cdsp_block<void, float> {
public:
    cdsp_gen_sine_float(float fs, float fr);
    void setFs(float fs);
    void setFr(float fr);
    static int IRAM_ATTR requestData(void* ctx, float* data, int samples_cnt);
private:
    float _phase = 0;
    float _fs = 1;
    float _fr = 0;
    float _incr = 1;
};
