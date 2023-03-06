#ifndef H_CDSP_MODULATOR
#define H_CDSP_MODULATOR
#include <math.h>
#include <algorithm>

#include "cdsp_common.h"
#include "cdsp_gen.h"
#include "cdsp_fir.h"
#include "cdsp_fir_taps.h"
#include "cdsp_timing_recovery.h"
#include "cdsp_agc.h"

#define MOD_PREAMBLE_SIZE 32

#define DEMOD_FR_AFC_SENS 0.01f
#define DEMOD_FR_AFC_BOUNDS 0.05f

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

class cdsp_mod_msk : public cdsp_block<uint8_t, cdsp_complex_t> {
public:
    cdsp_mod_msk(float fs, float datarate);
    ~cdsp_mod_msk();
    void setFs(float fs);
    void setDataRate(float datarate);
    float getDataRate();
    static int IRAM_ATTR requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

private:
    cdsp_gen_sine_complex* _gen;
    float _datarate;
    float _fs;
    float _incr;
    float _ctr = 0;
    uint8_t _curr_data;
    int _preamble_ctr = 0;

    void _do_start() override;
    void _do_stop() override;
};

class cdsp_demod_bfsk : public cdsp_block<cdsp_complex_t, uint8_t> {
public:
    cdsp_demod_bfsk(float fs, float fr0, float fr1, float datarate, float sync_loop_bw, float sync_damping, float sync_relLimit);
    ~cdsp_demod_bfsk();
    void setFs(float fs);
    void setFrs(float fr0, float fr1);
    void setDataRate(float datarate);
    void setLoopBw(float bw);
    static int IRAM_ATTR requestData(void* ctx, uint8_t* data, int samples_cnt);
    static int IRAM_ATTR dcb_requestData(void* ctx, float* data, int samples_cnt);
    static int IRAM_ATTR infilt_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

    float _avgerr = 0;

private:
    int _taps_cnt;
    float* _filt_taps = NULL;
    cdsp_fir<float, float>* _filt = NULL;
    cdsp_dcblock<float>* _dcblock;
    cdsp_maximum_likelihood_tr<float>* _sync = NULL;
    float _datarate = 0;
    float _fs = 0;
    float _incr;
    float _fr0, _fr1;
    float _out_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _in_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _prev_spl;
    float _syncmul;
    float _syncdiv;
    cdsp_fir<float, cdsp_complex_t>* _infilt = NULL;
    float* _infilt_taps = NULL;
    int _infilt_taps_cnt;

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
    void setLoopBw(float bw);
    static int IRAM_ATTR requestData(void* ctx, uint8_t* data, int samples_cnt);
    static int IRAM_ATTR dcb_requestData(void* ctx, float* data, int samples_cnt);
    static int IRAM_ATTR infilt_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

    float _avgerr = 0;

private:
    int _taps_cnt;
    cdsp_dcblock<float>* _dcblock;
    cdsp_maximum_likelihood_tr<float>* _sync = NULL;
    float* _filt_taps = NULL;
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
    cdsp_fir<float, cdsp_complex_t>* _infilt = NULL;
    float* _infilt_taps = NULL;
    int _infilt_taps_cnt;


    void _do_start() override;
    void _do_stop() override;
};

class cdsp_demod_msk : public cdsp_block<cdsp_complex_t, uint8_t> {
public:
    cdsp_demod_msk(float fs, float datarate, float sync_loop_bw, float sync_damping, float sync_relLimit);
    ~cdsp_demod_msk();
    void setFs(float fs);
    void setDataRate(float datarate);
    void setLoopBw(float bw);
    static int IRAM_ATTR requestData(void* ctx, uint8_t* data, int samples_cnt);
    static int IRAM_ATTR dcb_requestData(void* ctx, float* data, int samples_cnt);
    static int IRAM_ATTR infilt_requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

    float _avgerr = 0;


private:
    int _taps_cnt;
    float* _filt_taps = NULL;
    cdsp_fir<float, float>* _filt = NULL;
    cdsp_dcblock<float>* _dcblock;
    cdsp_maximum_likelihood_tr<float>* _sync = NULL;
    float _datarate = 0;
    float _fs = 0;
    float _incr;
    float _froffs = 0;
    float _out_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _in_samples[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t _prev_spl;
    float _syncmul;
    float _syncdiv;
    cdsp_fir<cdsp_complex_t, cdsp_complex_t>* _infilt = NULL;
    cdsp_complex_t* _infilt_taps = NULL;
    int _infilt_taps_cnt;
    float _phacc = 0;
    float _phacc2 = 0;
    cdsp_complex_t _int_buff[CDSP_DEF_BUFF_SIZE*100];
    int _int_buff_ptr = 0;

    void _do_start() override;
    void _do_stop() override;
};
#endif
