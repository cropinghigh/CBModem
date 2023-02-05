#pragma once
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "cdsp_common.h"
#include "cdsp_fir.h"
#include "cdsp_fir_taps.h"
#include "cdsp_rational_resamplers.h"

#include "chl_i2sanalog.h"
#include "chl_timer.h"
#include "chl_ext_si5351.h"

#define CDSP_SINK_TIMEOUT 100 / portTICK_PERIOD_MS

#define CDSP_SINK_DAC_BUFFS_CNT 3
class cdsp_sink_dac : public cdsp_block<float, void> {
public:
    cdsp_sink_dac(chl_i2sanalog* dac_dev, bool dac_out_ch);
    virtual ~cdsp_sink_dac();
    void setOutChannel(bool dac_out_ch);
    static int IRAM_ATTR work(void* ctx, int samples_cnt);

private:
    chl_i2sanalog* _dac_dev;
    chl_i2sanalog_dact* _out_bufs[CDSP_SINK_DAC_BUFFS_CNT];
    float _conv_coeff = 200.0f;
    float _in_buff[CDSP_DEF_BUFF_SIZE];
    int _curr_buff_status[CDSP_SINK_DAC_BUFFS_CNT]; //0-free, 1-transmitting as 1, 2-transmitting as 2
    bool _dac_out_ch; //0 - dac1; 1 - dac2;

    void _do_start() override;
    void _do_stop() override;
};

//SELF-CLOCKING BLOCK! USING TIMER 0!!!
#define CDSP_SINK_SI_TMR_PRESCALER 40
class cdsp_sink_si5351_fr : public cdsp_block<int32_t, void> {
public:
    cdsp_sink_si5351_fr(chl_ext_si5351* si5351_dev, bool pll, int startTimerVal, int timer_rate);
    ~cdsp_sink_si5351_fr();
    void setPll(bool pll);
    void setTimerRate(int timer_rate);
    void setStartTimerVal(int startTimerVal);
    void resetTimer();
    void IRAM_ATTR asyncSetFreq(int32_t newfr);
    int IRAM_ATTR syncSetFreq(int32_t newfr);
    int IRAM_ATTR syncQueueFreqs(int32_t* newfrs, int cnt);
    static void IRAM_ATTR isr_work(void* ctx);
    static int IRAM_ATTR work(void* ctx, int samples_cnt);

private:
    chl_ext_si5351* _si5351_dev;
    chl_timer* _tmr;
    bool _pll;
    SemaphoreHandle_t _write_bsmph;
    int32_t _curr_freq = 0;
    int32_t _freq_buff[CDSP_DEF_BUFF_SIZE];
    int _freq_buff_write_ptr = 0;
    int _freq_buff_read_ptr = 0;

    void _do_start() override;
    void _do_stop() override;
};

class cdsp_sink_combined : public cdsp_block<cdsp_complex_t, void> {
public:
    cdsp_sink_combined(chl_i2sanalog* dac_dev, chl_ext_si5351* si5351_dev, bool dac_out_ch, bool pll, int startTimerVal, int timer_rate, int interp, int taps_cnt);
    ~cdsp_sink_combined();
    static int IRAM_ATTR work(void* ctx, int samples_cnt);
    static int IRAM_ATTR dac_req_func(void* ctx, float* data, int samples_cnt);

    cdsp_sink_dac* sink_dac;
    cdsp_sink_si5351_fr* sink_si;
private:
    cdsp_rational_interpolator<float>* _interpol;
    cdsp_fir<float, float>* _interpol_fir;
    float* _interpol_fir_taps;
    int _taps_cnt;
    int _interp;
    int _timer_rate;
    cdsp_complex_t _in_buff[CDSP_DEF_BUFF_SIZE];
    int32_t _fr_buff[CDSP_DEF_BUFF_SIZE];
    float _prev_ph = 0;
    int _buff_data = 0;

    void _do_start() override;
    void _do_stop() override;
};
