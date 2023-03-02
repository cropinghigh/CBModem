#include "chl_ext_si5351.h"

chl_ext_si5351::chl_ext_si5351(uint8_t dev_addr, chl_i2c* drv, unsigned int xtal_freq) {
    _addr = dev_addr;
    _i2c_drv = drv;
    _xtal_freq = xtal_freq;
    reset_chip();
}

chl_ext_si5351::~chl_ext_si5351() {}

void chl_ext_si5351::set_addr(uint8_t dev_addr) {
	_addr = dev_addr;
}

void chl_ext_si5351::set_xtal_freq(uint32_t newfr) {
	_xtal_freq = newfr;
}

int chl_ext_si5351::set_frequency(bool pllB, unsigned int target_freq) {
    uint8_t tmp[3];
    uint32_t curr_pll_a = (SI5351_MIN_PLL_FREQ / _xtal_freq);
    if(target_freq >= 26020000 && target_freq <= 26980000) {
        _center_b = target_freq - 26000000; //To make it round
    } else if(target_freq >= 27020000 && target_freq <= 27980000) {
        _center_b = target_freq - 27000000;
    } else {
        _center_b = 500000;
    }
    uint32_t curr_pll_b = _center_b;
    uint32_t curr_ms_a = ((_xtal_freq * (curr_pll_a + curr_pll_b/SI5351_FRACTIONAL_DIVIER)) / target_freq);
    uint32_t curr_ms_b = (((_xtal_freq * (curr_pll_a + curr_pll_b/SI5351_FRACTIONAL_DIVIER)) / target_freq) * SI5351_FRACTIONAL_DIVIER) % SI5351_FRACTIONAL_DIVIER;
    if(curr_ms_a == 0 && curr_ms_b == 0) {
        return -1;
    }
    float last_freq_err = fabsf(target_freq - ((_xtal_freq * (curr_pll_a + curr_pll_b/SI5351_FRACTIONAL_DIVIER)) / (curr_ms_a + curr_ms_b/SI5351_FRACTIONAL_DIVIER)));
    for(int i = (SI5351_MIN_PLL_FREQ / _xtal_freq); i < (SI5351_MAX_PLL_FREQ / _xtal_freq); i++) {
        float pll_freq = ((float)_xtal_freq * ((float)i + (float)curr_pll_b/(float)SI5351_FRACTIONAL_DIVIER));
        if(target_freq == 0) {
            continue;
        }
        uint32_t ms_a = (pll_freq / target_freq);
        uint32_t ms_b = ((uint64_t)((pll_freq / target_freq) * SI5351_FRACTIONAL_DIVIER)) % SI5351_FRACTIONAL_DIVIER;
        if(ms_a == 0 && ms_b == 0) {
            continue;
        }
        if(ms_a > 8 && ms_a < 2048) {
            float freq = pll_freq / (ms_a + ms_b/SI5351_FRACTIONAL_DIVIER);
            float freq_err = fabsf(target_freq - freq);
            // printf("pllf %f(%d %lu), fr %f(%lu %lu), err %f\n", pll_freq, i, curr_pll_b, freq, ms_a, ms_b, freq_err);
            if(freq_err < last_freq_err) {
                curr_pll_a = i;
                curr_ms_a = ms_a;
                curr_ms_b = ms_b;
                last_freq_err = freq_err;
                if(freq_err == 0.0f) {
                    break;
                }
            }
        }
    }
    float pll_freq = (_xtal_freq * ((float)curr_pll_a + (float)curr_pll_b/(float)SI5351_FRACTIONAL_DIVIER));
    float freq = pll_freq / ((float)curr_ms_a + (float)curr_ms_b/(float)SI5351_FRACTIONAL_DIVIER);
    if(!_sync_set_divider((pllB ? 2 : 1), (pllB ? SI5351_REG_PLLB_MUL_BASE : SI5351_REG_PLLA_MUL_BASE), curr_pll_a, curr_pll_b)) { return -1;} //set pll coeffs
    if(!_sync_set_divider(0, SI5351_REG_MS0_DIV_BASE, curr_ms_a, curr_ms_b)) { return -1;} //set ms0 coeffs
    if(!_sync_set_divider(0, SI5351_REG_MS1_DIV_BASE, curr_ms_a, curr_ms_b)) { return -1;} //set ms1 coeffs
    if(!_sync_set_divider(0, SI5351_REG_MS2_DIV_BASE, curr_ms_a, curr_ms_b)) { return -1;} //set ms2 coeffs
    tmp[0] = (pllB ? SI5351_MS_CFG_PLLB : SI5351_MS_CFG_PLLA); tmp[1] = tmp[0]; tmp[2] = tmp[0];
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_OUTPUT_CTRL_BASE, tmp, 3, true, true); if(_i2c_drv->i2c_wait_for_transaction() != 0) {printf("si5351 set_frequency: i2c error 0\n");return -1;} //configure CLK0,1,2
    tmp[0] = 0x00; tmp[1] = 0x00; tmp[2] = 0x00;
    _i2c_drv->i2c_queue_write_regs(_addr, (SI5351_REG_PHASE_OFFSET_BASE), tmp, 3, true, true); if(_i2c_drv->i2c_wait_for_transaction() != 0) {printf("si5351 set_frequency: i2c error 1\n");return -1;} //0 phase offset for outs 0, 1,2
    tmp[0] = ((uint8_t)roundf((1.0/freq) / 4.0f * 4.0f * pll_freq)) & 0x7F; 
    _i2c_drv->i2c_queue_write_regs(_addr, (SI5351_REG_PHASE_OFFSET_BASE+1), tmp, 1, true, true); if(_i2c_drv->i2c_wait_for_transaction() != 0) {printf("si5351 set_frequency: i2c error 2\n");return -1;} //+pi/2 phase difference on CLK1
    tmp[0] = pllB ? 0x80 : 0x20;
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_PLL_RESET, tmp, 1, true, true); if(_i2c_drv->i2c_wait_for_transaction() != 0) {printf("si5351 set_frequency: i2c error 3\n");return -1;} //reset PLL
    pllB ? _curr_pllb_freq=pll_freq : _curr_plla_freq=pll_freq;
    _curr_center_out_freq = freq;
    _curr_freq_step = (float)_xtal_freq * (1.0f/(float)SI5351_FRACTIONAL_DIVIER) / (((float)curr_ms_a + (float)curr_ms_b/(float)SI5351_FRACTIONAL_DIVIER));
    // printf("Newoutfr %lu, step %f(%lu %lu %lu %lu), err %f\n", _curr_center_out_freq, _curr_freq_step, curr_pll_a, curr_pll_b, curr_ms_a, curr_ms_b, last_freq_err);
    return freq;
}

bool chl_ext_si5351::reset_pll(int pll) {
    uint8_t tmp[1];
    tmp[0] = (pll&0b1 ? 0x20 : 0x00) | (pll&0b10 ? 0x80 : 0x00);
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_PLL_RESET, tmp, 1, true, true); if(_i2c_drv->i2c_wait_for_transaction() != 0) {printf("si5351 reset_pll: i2c error\n");return false;} //reset PLL
    return true;
}

bool chl_ext_si5351::set_pll_frequency_shift(bool pllB, int32_t target_shift_hz) {
    if(!initialized) { return false;}
    int32_t bdiff = target_shift_hz / _curr_freq_step;
    if(abs(bdiff) >= _center_b-1) {
        bdiff = (bdiff > 0) ? _center_b-1 : -(_center_b-1);
    }
    uint32_t newb = _center_b + bdiff;
    int err = 0;
    if(!_sync_set_divider((pllB ? 2 : 1), (pllB ? SI5351_REG_PLLB_MUL_BASE : SI5351_REG_PLLA_MUL_BASE), (pllB ? _prev_pllb_a : _prev_plla_a), newb)) { printf("si5351 set_pll_frequency_shift: i2c error %d\n", err);return false; } //set pll coeffs
    return true;
}

bool chl_ext_si5351::set_output_enabled(bool tx, bool enabled) {
    if(!initialized) { return false;}
    uint8_t tmp;
    if(enabled) {
        if(!tx) {
            tmp = 0xfc; //Enable clk0,1
        } else {
            tmp = 0xfb; //Enable clk2
        }
    } else {
        tmp = 0xff; //Disable all outputs
    }
    int err = 0;
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_OUTPUT_DIS, &tmp, 1, true, true); if((err =_i2c_drv->i2c_wait_for_transaction()) != 0) { printf("si5351 set_output_enabled: i2c error %d\n", err);return false; }
    return true;
}

void chl_ext_si5351::reset_chip() {
    uint8_t tmp[3] = {0xff};
    int err;
    while(tmp[0] & ( 1 << 7)) { _i2c_drv->i2c_read_regs(_addr, SI5351_REG_STATUS, tmp, 1, true);if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 reset_chip: i2c init error %d\n", err);return;}}
    tmp[0] = 0xff;
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_OUTPUT_DIS, tmp, 1, true, true); if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 reset_chip: i2c error 0 %d\n", err);return;} //Disable all outputs
    tmp[0] = 0x80;
    for(uint8_t reg_addr = SI5351_REG_OUTPUT_CTRL_BASE; reg_addr < (SI5351_REG_OUTPUT_CTRL_BASE+8); reg_addr++) {
        _i2c_drv->i2c_queue_write_regs(_addr, reg_addr, tmp, 1, true, true); if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 reset_chip: i2c error 1 %d\n", err);return;} //write 0x80 to regs 16-23(powerdown all)
    }
    tmp[0] = 0x00;
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_OUTPUT_DIS_STATE, tmp, 1, true, true); if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 reset_chip: i2c error 2 %d\n", err);return;} //configure CLKs disable state
    tmp[0] = 0x0F; tmp[1] = 0x0F; tmp[2] = 0x0F;
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_OUTPUT_CTRL_BASE, tmp, SI5351_REG_OUTPUT_DIS, true, true); if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 reset_chip: i2c error 3 %d\n", err);return;} //configure CLK0,1,2(PLLA)
    tmp[0] = 0x00;
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_PLL_SOURCE, tmp, 1, true, true); if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 reset_chip: i2c error 6 %d\n", err);return;} //Select XTAL source for both PLLs
    tmp[0] = 0x52;
    _i2c_drv->i2c_queue_write_regs(_addr, SI5351_REG_XTAL_LOAD, tmp, 1, true, true); if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 reset_chip: i2c error 7 %d\n", err);return;} //Set 6pf(min) crystal load
    set_frequency(true, SI5351_DEFAULT_FREQ); //reset both PLLs
    int realf = set_frequency(false, SI5351_DEFAULT_FREQ);
    if(realf == -1) {
        printf("si5351 reset_chip: freq set error\n");
        return;
    }
    initialized = true;
}

float chl_ext_si5351::get_curr_freq_step() {
    return _curr_freq_step;
}

float chl_ext_si5351::get_curr_center_freq() {
    return _curr_center_out_freq;
}

void chl_ext_si5351::set_outputs_pll(bool pllB, int* yield) {
    if(!initialized) { return;}
    uint8_t tmp[3];
    // tmp[0] = pllB ? SI5351_MS_CFG_PLLB : SI5351_MS_CFG_PLLA; tmp[1] = tmp[0]; tmp[2] = tmp[0];
    // _i2c_drv->i2c_queue_write_regs_isr(_addr, SI5351_REG_OUTPUT_CTRL_BASE, tmp, 3, true);
    tmp[0] = pllB ? SI5351_MS_CFG_PLLB : SI5351_MS_CFG_PLLA;
    _i2c_drv->i2c_queue_write_regs_isr(_addr, SI5351_REG_OUTPUT_CTRL_BASE+2, tmp, 1, true, yield);
}

void chl_ext_si5351::set_tx_output_enabled_isr(bool enabled, int* yield) {
    if(!initialized) { return;}
    uint8_t tmp;
    if(enabled) {
        tmp = 0xfb; //Enable clk2
    } else {
        tmp = 0xff; //Disable all outputs
    }
    _i2c_drv->i2c_queue_write_regs_isr(_addr, SI5351_REG_OUTPUT_DIS, &tmp, 1, true, yield);
}

void chl_ext_si5351::set_pll_frequency_fast(bool pllB, int32_t target_shift_hz, int* yield) {
    if(!initialized) { return;}
    int32_t bdiff = target_shift_hz / _curr_freq_step;
    if(abs(bdiff) >= _center_b-1) {
        bdiff = (bdiff > 0) ? _center_b-1 : -(_center_b-1);
    }
    uint32_t newb = _center_b + bdiff;
    _fast_change_pll_mul(pllB, (pllB ? _prev_pllb_a : _prev_plla_a), newb, yield);
}

//Write a+b/c (c always = SI5351_FRACTIONAL_DIVIER) divider data to the registers(according to AN619)
bool chl_ext_si5351::_sync_set_divider(int pll, uint8_t base_reg, uint32_t a, uint32_t b) {
    uint8_t buffer[8];
    //Form configuration buffer
    uint32_t MS0_P1 = (128 * a) + ((b*128)/SI5351_FRACTIONAL_DIVIER) - 512;
    uint32_t MS0_P2 = (128 * b) - (SI5351_FRACTIONAL_DIVIER * ((b*128)/SI5351_FRACTIONAL_DIVIER));
    uint32_t MS0_P3 = SI5351_FRACTIONAL_DIVIER;
    if(pll == 1) {
        _prev_plla_p1 = MS0_P1;
        _prev_plla_a = a;
        _prev_plla_b = b;
        // printf("plla a:%lu b:%lu\n", _prev_plla_a, _prev_plla_b);
    } else if(pll == 2) {
        _prev_pllb_p1 = MS0_P1;
        _prev_pllb_a = a;
        _prev_pllb_b = b;
        // printf("pllb a:%lu b:%lu\n", _prev_pllb_a, _prev_pllb_b);
    }
    buffer[0] =  (MS0_P3 >>  8L) & 0xff;                               //x_P3[15:8]
    buffer[1] =  (MS0_P3       ) & 0xff;                               //x_P3[7:0]
    buffer[2] =  (MS0_P1 >> 16L) & 0x03;                               //x_DIV[2:0] + x_DIVBY4[1:0] + x_P1[17:16]
    buffer[3] =  (MS0_P1 >>  8L) & 0xff;                               //x_P1[15:8]
    buffer[4] =  (MS0_P1       ) & 0xff;                               //x_P1[7:0]
    buffer[5] = ((MS0_P3 >> 12L) & 0xf0) | ((MS0_P2 >> 16L) & 0x0f);   //x_P3[19:16] + x_P2[19:16]
    buffer[6] =  (MS0_P2 >>  8L) & 0xff;                               //x_P2[15:8]
    buffer[7] =  (MS0_P2       ) & 0xff;                               //x_P2[7:0]
    int err = 0;
    _i2c_drv->i2c_queue_write_regs(_addr, base_reg, buffer, 8, true, true); if((err = _i2c_drv->i2c_wait_for_transaction()) != 0) {printf("si5351 _sync_set_divider: i2c error %d\n", err);return false;} //Write config for x
    return true;
}

//Fast PLL multipler's fractional part change for modulation; WARNING: _sync_set_divider for target pll should be called first!!!
void chl_ext_si5351::_fast_change_pll_mul(bool pllB, uint32_t a, uint32_t b, int* yield) {
    if((b == (pllB ? _prev_pllb_b : _prev_plla_b)) && (a == (pllB ? _prev_pllb_a : _prev_plla_a))) {
        //Nothing changed
        return;
    }
    // ets_printf("FR %d\n", b);
    uint8_t buffer[6];
    uint32_t new_pll_P1 = (128 * a) + ((b*128)/SI5351_FRACTIONAL_DIVIER) - 512;
    uint32_t new_pll_P2 = (128 * b) - (SI5351_FRACTIONAL_DIVIER * ((b*128)/SI5351_FRACTIONAL_DIVIER));
    uint32_t pll_P3 = SI5351_FRACTIONAL_DIVIER;
    // if((pllB ? _prev_pllb_p1 : _prev_plla_p1) == new_pll_P1) {
    if(false) {
        //changing only 3 values from (base+5) to (base+8)
        buffer[0] = ((pll_P3 >>     12L) & 0xf0) | ((new_pll_P2 >> 16L) & 0x0f); //x_P3[19:16] + x_P2[19:16]
        buffer[1] =  (new_pll_P2 >>  8L) & 0xff;                                 //x_P2[15:8]
        buffer[2] =  (new_pll_P2       ) & 0xff;                                 //x_P2[7:0]
        if(xPortInIsrContext()) {
            if(_i2c_drv->i2c_queue_write_regs_isr(_addr, ((pllB ? SI5351_REG_PLLB_MUL_BASE : SI5351_REG_PLLA_MUL_BASE)+5), buffer, 3, true, yield) != ESP_OK) {
                ets_printf("FSE\n");
            }
        } else {
            _i2c_drv->i2c_queue_write_regs(_addr, ((pllB ? SI5351_REG_PLLB_MUL_BASE : SI5351_REG_PLLA_MUL_BASE)+5), buffer, 3, true, true);
        }
    } else {
        //changing only 4 values from (base+4) to (base+8) (assuming that P1's 2nd and 3st bytes are constant)
        // buffer[0] =  (new_pll_P1 >>     16L) & 0x03;                               //x_DIV[2:0] + x_DIVBY4[1:0] + x_P1[17:16]
        // buffer[1] =  (new_pll_P1 >>      8L) & 0xff;                               //x_P1[15:8]
        buffer[0] =  (new_pll_P1           ) & 0xff;                               //x_P1[7:0]
        buffer[1] = ((pll_P3 >> 12L) & 0xf0) | ((new_pll_P2 >> 16L) & 0x0f);       //x_P3[19:16] + x_P2[19:16]
        buffer[2] =  (new_pll_P2 >>      8L) & 0xff;                               //x_P2[15:8]
        buffer[3] =  (new_pll_P2           ) & 0xff;                               //x_P2[7:0]
        if(xPortInIsrContext()) {
            if(_i2c_drv->i2c_queue_write_regs_isr(_addr, ((pllB ? SI5351_REG_PLLB_MUL_BASE : SI5351_REG_PLLA_MUL_BASE)+4), buffer, 4, true, yield) != ESP_OK) {
                ets_printf("FSE\n");
            }
        } else {
            _i2c_drv->i2c_queue_write_regs(_addr, ((pllB ? SI5351_REG_PLLB_MUL_BASE : SI5351_REG_PLLA_MUL_BASE)+4), buffer, 4, true, true);
        }
    }
    (pllB ? _prev_pllb_b : _prev_plla_b) = b;
    (pllB ? _prev_pllb_a : _prev_plla_a) = a;
    (pllB ? _prev_pllb_p1 : _prev_plla_p1) = new_pll_P1;
}
