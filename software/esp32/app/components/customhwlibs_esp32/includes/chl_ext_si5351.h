#pragma once

#include <math.h>
#include "chl_i2c.h"

#define SI5351_MIN_PLL_FREQ 600000000
#define SI5351_MAX_PLL_FREQ 900000000
#define SI5351_FRACTIONAL_DIVIER 1000000 //1e6 is maximum round value
#define SI5351_DEFAULT_FREQ 27000000
#define SI5351_REG_STATUS 0
#define SI5351_REG_OUTPUT_DIS 3
#define SI5351_REG_OUTPUT_CTRL_BASE 16
#define SI5351_REG_OUTPUT_DIS_STATE 24
#define SI5351_REG_PLL_SOURCE 15
#define SI5351_REG_XTAL_LOAD 183
#define SI5351_REG_PHASE_OFFSET_BASE 165
#define SI5351_REG_PLL_RESET 177
#define SI5351_REG_PLLA_MUL_BASE 26
#define SI5351_REG_PLLB_MUL_BASE 34
#define SI5351_REG_MS0_DIV_BASE 42
#define SI5351_REG_MS1_DIV_BASE 50
#define SI5351_REG_MS2_DIV_BASE 58
#define SI5351_MS_CFG_PLLA 0b00001111
#define SI5351_MS_CFG_PLLB 0b00101111

class chl_ext_si5351 {
public:
    chl_ext_si5351(uint8_t dev_addr, chl_i2c* drv, unsigned int xtal_freq);
    ~chl_ext_si5351();
    int set_frequency(bool pllB, unsigned int target_freq);
    bool reset_pll(int pll);
    bool set_pll_frequency_shift(bool pllB, int32_t target_shift_hz);
    bool set_output_enabled(bool tx, bool enabled);
    void reset_chip();
    float IRAM_ATTR get_curr_freq_step();
    float IRAM_ATTR get_curr_center_freq();
    void IRAM_ATTR set_outputs_pll(bool pllB, int* yield);
    void IRAM_ATTR set_tx_output_enabled_isr(bool enabled, int* yield);
    void IRAM_ATTR set_pll_frequency_fast(bool pllB, int32_t target_shift_hz, int* yield);

private:
    bool initialized = false;
    uint8_t _addr;
    unsigned int _xtal_freq;
    chl_i2c* _i2c_drv;
    uint32_t _curr_plla_freq = 0;
    uint32_t _curr_pllb_freq = 0;
    uint32_t _curr_center_out_freq = 0;
    double _curr_freq_step = 0;
    //Cache, to not write duplicate values
    uint32_t _prev_plla_p1;
    uint32_t _prev_plla_a = 0;
    uint32_t _prev_plla_b = 0;
    uint32_t _prev_pllb_p1;
    uint32_t _prev_pllb_a = 0;
    uint32_t _prev_pllb_b = 0;
    uint32_t _center_b = 500000;

    //Write a+b/c (c always = SI5351_FRACTIONAL_DIVIER) divider data to the registers(according to AN619)
    bool _sync_set_divider(int pll, uint8_t base_reg, uint32_t a, uint32_t b);

    //Fast PLL multipler's fractional part change for modulation; WARNING: _sync_set_divider for target pll should be called first!!!
    void IRAM_ATTR _fast_change_pll_mul(bool pllB, uint32_t a, uint32_t b, int* yield);
};
