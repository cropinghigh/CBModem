#pragma once
#include <math.h>
#include <algorithm>

#include "cdsp_common.h"
#include "cdsp_gen.h"
#include "cdsp_fir.h"
#include "cdsp_fir_taps.h"
#include "cdsp_timing_recovery.h"
#include "cdsp_agc.h"

#define MOD_PREAMBLE_SIZE 32

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
    int _preamble_ctr = 0;

    void _do_start() override;
    void _do_stop() override;
};

class cdsp_mod_mfsk : public cdsp_block<uint8_t, cdsp_complex_t> {
public:
    cdsp_mod_mfsk(float fs, int frcnt, float* frs, float datarate, int taps_cnt);
    ~cdsp_mod_mfsk();
    void setFs(float fs);
    void setFrs(int frcnt, float* frs);
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
    int _frcnt;
    int _frbits;
    float* _frs = NULL;
    float _ctr = 0;
    uint8_t _curr_data[256];
    int _preamble_ctr = 0;

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
    static int IRAM_ATTR dcb_requestData(void* ctx, float* data, int samples_cnt);

private:
    int _taps_cnt;
    cdsp_fir<float, float>* _filt = NULL;
    cdsp_dcblock<float>* _dcblock;
    cdsp_maximum_likelihood_tr<float>* _sync = NULL;
    float* _filt_taps = NULL;
    int _filt_ptr = 0;
    float* _filt_buff = NULL;
    float _datarate = 0;
    float _fs = 0;
    float _incr;
    float _fr0, _fr1;
    float _out_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _in_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _prev_spl;
    float _syncmul;
    float _syncdiv;

    void _do_start() override;
    void _do_stop() override;
};

class cdsp_demod_mfsk : public cdsp_block<cdsp_complex_t, uint8_t> {
public:
    cdsp_demod_mfsk(float fs, int frcnt, float* frs, float datarate, float sync_loop_bw, float sync_damping, float sync_relLimit);
    ~cdsp_demod_mfsk();
    void setFs(float fs);
    void setFrs(int frcnt, float* frs);
    void setDataRate(float datarate);
    static int IRAM_ATTR requestData(void* ctx, uint8_t* data, int samples_cnt);
    static int IRAM_ATTR dcb_requestData(void* ctx, float* data, int samples_cnt);

private:
    int _taps_cnt;
    cdsp_dcblock<float>* _dcblock;
    cdsp_maximum_likelihood_tr<float>* _sync = NULL;
    float* _filt_taps;
    cdsp_fir<float, float>* _filt = NULL;
    float _datarate = 0;
    float _fs = 0;
    float _incr;
    int _frcnt = 0;
    int _frbits;
    float* _frs = NULL;
    float _out_samples[CDSP_DEF_BUFF_SIZE];
    uint8_t _outbuff[CDSP_DEF_BUFF_SIZE];
    int _outbuff_data = 0;
    int _outbuff_ptr = 0;
    uint8_t _outbuff_bit = 0;
    cdsp_complex_t _in_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _prev_spl;
    float _syncmul;
    float _syncdiv;

    void _do_start() override;
    void _do_stop() override;
};
