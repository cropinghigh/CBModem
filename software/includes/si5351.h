#pragma once

#include "i2c.h"

#define SI5351_I2C_ADDR 0x60
#define PLLfreq 700000000.0f

const char strI2CError[] = "I2C error: ";

//Report an I2C error to UART
void si5351_print_i2c_error() {
    uart_write_cstring("Si5351 I2C error! Current state: ");
    uart_write_uint(i2c_get_state());
    uart_write_newline();
}

//Find available divider value to get frequency nearest to given
void si5351_calculate_divider(float freq, unsigned int& MSx_a, float& MSx_b, float& MSx_c) {
    MSx_a = 8;
    MSx_b = 0;
    MSx_c = 10000;
    //find MSx_a
    while(MSx_a < 2047) {
        float calc_val = PLLfreq / (MSx_a);
        if(calc_val < freq) {
            break; //we have found a nearest larger integer 'MSx_a' value!
        }
        MSx_a++;
    }
    //find MSx_b
    while(MSx_b < 10000) {
        float calc_val = PLLfreq / (a + MSx_b/MSx_c);
        if(calc_val < freq) {
            break; //we have found a nearest values!
        }
        MSx_b++;
    }
}

//Set frequencies for CLK0 and CLK1 both and set phase offset to +pi/2 on CLK1
bool si5351_set_frequencies(float freq) {
    //Multisynth configuration(according to AN619)
    uint8_t buffer[8];
    unsigned int MS0_a = 0;
    float MS0_b = 0;
    float MS0_c = 0;
    si5351_calculate_divider(freq, MS0_a, MS0_b, MS0_c);
    //Form Multisynth configuration buffer
    uint32_t MS0_P1 = (128 * MS0_a) + floor(128.0f * (MS0_b/MS0_c)) - 512;
    uint32_t MS0_P2 = (128 * MS0_b) - (MS0_c * floor(128.0f * (MS0_b/MS0_c)));
    uint32_t MS0_P3 = MS0_c;
    buffer[0] =  (MS0_P3 >>  8L) & 0xff;                               //MS0_P3[15:8]
    buffer[1] =  (MS0_P3       ) & 0xff;                               //MS0_P3[7:0]
    buffer[2] =  (MS0_P1 >> 16L) & 0x03;                               //R0_DIV[2:0] + MS0_DIVBY4[1:0] + MS0_P1[17:16]
    buffer[3] =  (MS0_P1 >>  8L) & 0xff;                               //MS0_P1[15:8]
    buffer[4] =  (MS0_P1       ) & 0xff;                               //MS0_P1[7:0]
    buffer[5] = ((MS0_P3 >> 12L) & 0xf0) | ((MS0_P2 >> 16L) & 0x0f);   //MS0_P3[19:16] + MS0_P2[19:16]
    buffer[6] =  (MS0_P2 >>  8L) & 0xff;                               //MS0_P2[15:8]
    buffer[7] =  (MS0_P2       ) & 0xff;                               //MS0_P2[7:0]
    if(!i2c_sync_write(SI5351_I2C_ADDR, 42, 8, buffer)) {si5351_print_i2c_error(); return false;} //Write config for MS0
    if(!i2c_sync_write(SI5351_I2C_ADDR, 42, 8, buffer)) {si5351_print_i2c_error(); return false;} //Write config for MS1
    //Setup +pi/2 phase difference on CLK1
    float MS1_phoff = (1.0/freq) / 4; //Period/4 = +pi/2 phase diff
    uint8_t CLK1_PHOFF = round(MS1_phoff*4.0f*PLLfreq);
    if(!i2c_sync_write(SI5351_I2C_ADDR, 165, 1, (uint8_t[]){CLK1_PHOFF & 0x3F})) {si5351_print_i2c_error(); return false;} //Write phase offset
    return true;
}

//Initialize Si5351 device
bool si5351_setup(uint8_t address) {
    uint8_t buffer[8] = {0xff};
    while(buffer[0] & ( 1 << 7)) { if(!i2c_sync_read(SI5351_I2C_ADDR, 0, 1, buffer)) {si5351_print_i2c_error(); return false;} }
    if(!i2c_sync_write(SI5351_I2C_ADDR, 3, 1, (uint8_t[]){0xff})) {si5351_print_i2c_error(); return false;} //disable all CLKs outputs
    for(uint8_t reg_addr = 16; reg_addr < 24; reg_addr++) {
        if(!i2c_sync_write(SI5351_I2C_ADDR, reg_addr, 1, (uint8_t[]){0x80})) {si5351_print_i2c_error(); return false;} //write 0x80 to regs 16-23(powerdown all)
    }

    //PLL configuration(according to AN619)
    unsigned int MSNA_a = 28; //Round int 700 MHz PLLA from 25 MHz xtal(25*28=700)
    float MSNA_b = 0;
    float MSNA_c = 1;
    uint32_t MSNA_P1 = (128 * MSNA_a) + floor(128.0f * (MSNA_b/MSNA_c)) - 512;
    uint32_t MSNA_P2 = (128 * MSNA_b) - (MSNA_c * floor(128.0f * (MSNA_b/MSNA_c)));
    uint32_t MSNA_P3 = MSNA_c;
    //Form PLL configuration buffer
    buffer[0] =  (MSNA_P3 >>  8L) & 0xff;                               //MSNA_P3[15:8]
    buffer[1] =  (MSNA_P3       ) & 0xff;                               //MSNA_P3[7:0]
    buffer[2] =  (MSNA_P1 >> 16L) & 0x03;                               //Reserved + MSNA_P1[17:16]
    buffer[3] =  (MSNA_P1 >>  8L) & 0xff;                               //MSNA_P1[15:8]
    buffer[4] =  (MSNA_P1       ) & 0xff;                               //MSNA_P1[7:0]
    buffer[5] = ((MSNA_P3 >> 12L) & 0xf0) | ((MSNA_P2 >> 16L) & 0x0f);  //MSNA_P3[19:16] + MSNA_P2[19:16]
    buffer[6] =  (MSNA_P2 >>  8L) & 0xff;                               //MSNA_P2[15:8]
    buffer[7] =  (MSNA_P2       ) & 0xff;                               //MSNA_P2[7:0]

    if(!i2c_sync_write(SI5351_I2C_ADDR, 22, 1, (uint8_t[]){0xC0})) {si5351_print_i2c_error(); return false;} //Enable PLL A integer division mode(FBA_INT=1)
    if(!i2c_sync_write(SI5351_I2C_ADDR, 23, 1, (uint8_t[]){0xC0})) {si5351_print_i2c_error(); return false;} //Enable PLL B integer division mode(FBB_INT=1)
    if(!i2c_sync_write(SI5351_I2C_ADDR, 26, 8, buffer)) {si5351_print_i2c_error(); return false;} //Write config for PLLA
    if(!i2c_sync_write(SI5351_I2C_ADDR, 34, 8, buffer)) {si5351_print_i2c_error(); return false;} //Write same config for PLLB(to not leave it random)
    if(!i2c_sync_write(SI5351_I2C_ADDR, 15, 1, (uint8_t[]){0x00})) {si5351_print_i2c_error(); return false;} //Select XTAL source for both PLLs

    if(!i2c_sync_write(SI5351_I2C_ADDR, 16, 1, (uint8_t[]){0x0F})) {si5351_print_i2c_error(); return false;} //configure CLK0(PLLA)
    if(!i2c_sync_write(SI5351_I2C_ADDR, 17, 1, (uint8_t[]){0x0F})) {si5351_print_i2c_error(); return false;} //configure CLK1(PLLA)
    if(!i2c_sync_write(SI5351_I2C_ADDR, 24, 1, (uint8_t[]){0x00})) {si5351_print_i2c_error(); return false;} //configure CLKs disable state

    if(!si5351_set_frequencies(27135000)) {return false;} //Set default frequency

    if(!i2c_sync_write(SI5351_I2C_ADDR, 177, 1, (uint8_t[]){0xAC})) {si5351_print_i2c_error(); return false;} //soft reset PLLs
    if(!i2c_sync_write(SI5351_I2C_ADDR, 3, 1, (uint8_t[]){0xff})) {si5351_print_i2c_error(); return false;} //disable all CLKs outputs
    return true;
}

//Enable or disable both clk0 and clk1
bool si5351_set_clk_enabled(bool enabled) {
    if(enabled) {
        if(!i2c_sync_write(SI5351_I2C_ADDR, 3, 1, (uint8_t[]){0xfc})) {si5351_print_i2c_error(); return false;} //enable CLK0 and CLK1
    } else {
        if(!i2c_sync_write(SI5351_I2C_ADDR, 3, 1, (uint8_t[]){0xff})) {si5351_print_i2c_error(); return false;} //disable all CLKs outputs
    }
    return true;
}

