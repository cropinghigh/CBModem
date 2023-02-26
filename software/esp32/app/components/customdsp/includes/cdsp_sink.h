#ifndef H_CDSP_SINK
#define H_CDSP_SINK
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
    ~cdsp_sink_dac();
    void setOutChannel(bool dac_out_ch);
    static int IRAM_ATTR work(void* ctx, int samples_cnt);

private:
    chl_i2sanalog* _dac_dev;
    chl_i2sanalog_dact* _out_bufs[CDSP_SINK_DAC_BUFFS_CNT];
    float _conv_coeff = 170.0f;
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
    cdsp_sink_si5351_fr(chl_ext_si5351* si5351_dev, bool pll, int timer_rate);
    ~cdsp_sink_si5351_fr();
    void setPll(bool pll);
    void setTimerRate(int timer_rate);
    void setTimerVal(uint64_t timer_val);
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

#define CDSP_SINK_COMB_FREQ_DELAY_SIZE 64
class cdsp_sink_combined : public cdsp_block<cdsp_complex_t, void> {
public:
    cdsp_sink_combined(chl_i2sanalog* dac_dev, chl_ext_si5351* si5351_dev, int compensA, int compensB, bool dac_out_ch, bool pll, int timer_rate, int interp, int taps_cnt);
    ~cdsp_sink_combined();
    void setCompens(int compensA, int compensB);
    void setSr(int timer_rate, int interp);
    void setTaps(int taps_cnt);
    static int IRAM_ATTR work(void* ctx, int samples_cnt);
    static int IRAM_ATTR amplf_req_func(void* ctx, float* data, int samples_cnt);

    cdsp_sink_dac* sink_dac;
    cdsp_sink_si5351_fr* sink_si;
    // cdsp_sink_ledc* sink_ledc;
private:
    // chl_ext_si5351* _si5351_dev;
    cdsp_rational_interpolator<float>* _interpol;
    cdsp_fir<float, float>* _interpol_fir;
    float* _interpol_fir_taps;
    int _frsetctr = 0;
    int _taps_cnt;
    int _interp;
    int _timer_rate;
    int _compensA, _compensB;
    cdsp_complex_t _in_buff[CDSP_DEF_BUFF_SIZE];
    int32_t _fr_buff[CDSP_DEF_BUFF_SIZE];
    float _prev_ph = 0;
    float _fr_delay_buff[CDSP_SINK_COMB_FREQ_DELAY_SIZE];
    int _fr_delay_buff_rptr = 0;
    int _fr_delay_buff_wptr = 0;
    // uint16_t _ampl_delay_buff[CDSP_SINK_COMB_FREQ_DELAY_SIZE];
    // int _ampl_delay_buff_rptr = 0;
    // int _ampl_delay_buff_wptr = 0;
    // float _curr_in_ampl = 0;
    // float _ampl_filt_buff[10];

    // uint16_t _curr_ampl = 0;
    // uint16_t _ampl_buff[CDSP_DEF_BUFF_SIZE*10];
    // int32_t _curr_freq = 0;
    // int32_t _freq_buff[CDSP_DEF_BUFF_SIZE];
    // int _af_buff_write_ptr = 0;
    // int _af_buff_read_ptr = 0;
    // SemaphoreHandle_t _write_bsmph;

    void _do_start() override;
    void _do_stop() override;
};
#endif
