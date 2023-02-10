#pragma once
#include <math.h>

#include "cdsp_common.h"
#include "cdsp_gen.h"
#include "cdsp_fir.h"
#include "cdsp_fir_taps.h"
#include "cdsp_timing_recovery.h"
#include "cdsp_agc.h"

class cdsp_mod_bfsk : public cdsp_block<uint8_t, cdsp_complex_t> {
public:
    cdsp_mod_bfsk(float fs, float fr0, float fr1, float datarate, int taps_cnt);
    ~cdsp_mod_bfsk();
    void setFs(float fs);
    void setFrs(float fr0, float fr1);
    void setDataRate(float datarate);
    static int IRAM_ATTR requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);
    static int IRAM_ATTR filt_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

private:
    int _taps_cnt;
    cdsp_gen_sine_complex* _gen;
    cdsp_fir<float, cdsp_complex_t>* _filt;
    float* _filt_taps;
    float _datarate;
    float _fs;
    float _incr;
    float _fr0, _fr1;
    float _ctr = 0;
    uint8_t _curr_data;

    void _do_start() override;
    void _do_stop() override;
};

// #define CDSP_DEMOD_BFSK_FILT_GAIN 1.75f
#define CDSP_DEMOD_BFSK_FILT_GAIN 1.0f
class cdsp_demod_bfsk : public cdsp_block<cdsp_complex_t, uint8_t> {
public:
    cdsp_demod_bfsk(float fs, float fr0, float fr1, float datarate, float sync_loop_bw, float sync_damping, float sync_relLimit);
    ~cdsp_demod_bfsk();
    void setFs(float fs);
    void setFrs(float fr0, float fr1);
    void setDataRate(float datarate);
    static int IRAM_ATTR requestData(void* ctx, uint8_t* data, int samples_cnt);
    static int IRAM_ATTR filtout_requestData(void* ctx, float* data, int samples_cnt);
    static int IRAM_ATTR filt0_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);
    static int IRAM_ATTR filt1_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

private:
    int _taps_cnt;
    // int _out_taps_cnt;
    cdsp_gen_sine_complex* _gen0 = NULL;
    cdsp_gen_sine_complex* _gen1 = NULL;
    cdsp_fir<float, cdsp_complex_t>* _filt0;
    cdsp_fir<float, cdsp_complex_t>* _filt1;
    // cdsp_fir<float, float>* _filt_out;
    cdsp_dcblock<float>* _dcblock;
    cdsp_maximum_likelihood_tr<float>* _sync = NULL;
    float* _filt_taps;
    float* _filt_out_taps;
    float _datarate = 0;
    float _fs = 0;
    float _incr;
    float _fr0, _fr1;
    float _out_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _filt0_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _filt1_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _filt1_in_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _gen_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _in_samples[CDSP_DEF_BUFF_SIZE];
    int _filt1_samples_cnt = 0;

    void _do_start() override;
    void _do_stop() override;
};
