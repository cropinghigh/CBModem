#pragma once

#include <type_traits>
#include "esp_attr.h"

#include <stdint.h>
//some ideas could be(and probably) taken from Ryzerth's SDR++ dsp code

//Using float because it's convenient to use and measured performance is close to the integer, except division, but it's being used relative rarely
/*
avg performance test results:
int16_t addition: 917 uS per 10000 operations(0.0917 uS per operation)
int16_t substraction: 918 uS per 10000 operations(0.0918 uS per operation)
int16_t multiplication: 875 uS per 10000 operations(0.0875 uS per operation)
int16_t division: 1042 uS per 10000 operations(0.1042 uS per operation)
int16_t scaled mul: 1042 uS per 10000 operations(0.1042 uS per operation)
int16_t non-int mul: 1000 uS per 10000 operations(0.1 uS per operation)

float addition: 917 uS per 10000 operations(0.0917 uS per operation)
float substraction: 917 uS per 10000 operations(0.0917 uS per operation)
float multiplication: 917 uS per 10000 operations(0.0917 uS per operation)
float division: 2667 uS per 10000 operations(0.2667 uS per operation)
float scaled mul: 2876 uS per 10000 operations(0.2876 uS per operation)
float non-int mul: 834 uS per 10000 operations(0.0834 uS per operation)
*/
#define CDSP_DEF_BUFF_SIZE 128 //in elements

#define FL_PI 3.141592653589793238f

class cdsp_complex_t {
public:
    float i;
    float q;
    inline void operator=(float b) {
        // Implicit float to complex conversion
        i = b;
        q = 0;
    }
    inline cdsp_complex_t operator+(cdsp_complex_t b) {
        return (cdsp_complex_t) {i + b.i, q + b.q};
    }
    inline cdsp_complex_t operator+(float b) {
        return (cdsp_complex_t) {i + b, q};
    }
    inline cdsp_complex_t operator-(cdsp_complex_t b) {
        return (cdsp_complex_t) {i - b.i, q - b.q};
    }

    inline cdsp_complex_t operator-(float b) {
        return (cdsp_complex_t) {i - b, q};
    }

    inline cdsp_complex_t operator*(cdsp_complex_t b) {
        //(a + bi) * (c + di) = a*c + (a*d)i + (c*b)i - (b*d) = (a*c - b*d) + (a*d + b*c)i
        return (cdsp_complex_t) {(i*b.i - q*b.q), (i*b.q + q*b.i)};
    }

    inline cdsp_complex_t operator*(float b) {
        // printf("mul %f * (%f + %fi) = (%f + %fi)\n", b, a.i, a.q, a.i*b, a.q*b);
        return (cdsp_complex_t) {(i*b), (q*b)};
    }

    inline cdsp_complex_t operator/(cdsp_complex_t b) {
        return (cdsp_complex_t) {((i*b.i + q*b.q) / (b.i*b.i + b.q*b.q)), ((q*b.i - i*b.q) / (b.i*b.i + b.q*b.q))};
    }

    inline cdsp_complex_t operator/(float b) {
        return (cdsp_complex_t) {(i/b), (q/b)};
    }

    inline float pow2() {
        //Raise a to the second power
        return (i*i + q*q);
    }

    inline float mag() {
        //Get magnitude of the vector(slow, but exact)
        return sqrtf(pow2());
    }

    inline float ph() {
        //Get phase angle of the vector in radians(slow, but exact)
        return atan2f(q, i);
    }
};
template<typename T_OUT>
class cdsp_block_base {
public:
    virtual ~cdsp_block_base() {};
    // virtual int requestData(T_OUT* data, int samples_cnt) = 0;
    virtual void start(bool chain) = 0;
    virtual void stop(bool chain) = 0;
};

template<typename T_IN, typename T_OUT>
class cdsp_block : public cdsp_block_base<T_OUT> {
public:
    virtual ~cdsp_block() {};
    void setInputBlk(cdsp_block_base<T_IN>* blk, int (*reqFunc)(void*, T_IN*, int)) {
        _xinput_blk = blk;
        if(reqFunc != NULL) {
            setInputFunc(_xinput_blk, reqFunc);
        }
    }
    void setInputFunc(void* ctx, int (*reqFunc)(void*, T_IN*, int)) {
        _func_call_ctx = ctx;
        _input_func = reqFunc;
    }
    void start(bool chain) override {
        if(_running) { return;}
        _running = true;
        if(chain && _xinput_blk != NULL) {
            _xinput_blk->start(chain);
        }
        _do_start();
    }
    void stop(bool chain) override {
        if(!_running) { return;}
        _running = false;
        if(chain && _xinput_blk != NULL) {
            _xinput_blk->stop(chain);
        }
        _do_stop();
    }
protected:
    cdsp_block_base<T_IN>* _xinput_blk = NULL;
    void* _func_call_ctx = NULL;
    int (*_input_func)(void*, T_IN*, int) = NULL;
    bool _running = false;
    virtual void _do_start() {}
    virtual void _do_stop() {}
};
