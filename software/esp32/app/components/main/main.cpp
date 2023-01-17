/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart.h"
#include "i2c.h"
#include "gpio.h"
#include "i2sanalog.h"
#include "timer.h"

#include "esp_timer.h"

extern "C" void app_main();

DMA_ATTR static char dma_tx_buf[512];
#define chl_i2sanalog_bufs 64
DMA_ATTR static chl_i2sanalog_dact dac_buff_1[chl_i2sanalog_bufs];
DMA_ATTR static chl_i2sanalog_dact dac_buff_2[chl_i2sanalog_bufs];
DMA_ATTR static chl_i2sanalog_dact dac_buff_3[chl_i2sanalog_bufs];

void IRAM_ATTR on_timer(void* ctx) {
    // ets_printf("TMR: %d\n", (int)ctx);
}

void app_main(void)
{
    chl_uart_dma mainuart(0, 115200, 3, 1);
    chl_i2c maini2c(0, 1000, 27, 14);
    chl_i2sanalog maini2s;
    chl_timer maintimer(0);
    const int realSR = maini2s.setSampleRate(100000);
    printf("Real SR: %d\n", realSR);
    maini2s.configureADC(0, 3, 12, 11);

    maintimer.configurePrescaler(80);
    maintimer.configureTimer(true, true, 0, 1000000ULL);
    maintimer.setCallback(on_timer, (void*)2);
    maintimer.forceReloadDefVal();
    maintimer.setInterruptEnabled(true);
    maintimer.setTimerRunning(true);

    int samplesCnt = realSR/1000.0f;
    uint8_t sinArr[samplesCnt];
    for(int i = 0; i < samplesCnt; i++) {
        sinArr[i] = fabsf(sinf(2.0f*3.1415f*1000.0f*i/float(realSR))) * 255;
    }
    int currArrIdx = 0;
    int ph = 0;
    for(int i = 0; i < chl_i2sanalog_bufs; i++) {
        // dac_buff_1[i].ch1d = 0;
        dac_buff_1[i].ch1d = 0;
        dac_buff_1[i].ch2d = 0;
    }
    for(int i = 0; i < chl_i2sanalog_bufs; i++) {
        // dac_buff_2[i].ch1d = 0;
        dac_buff_2[i].ch1d = 0;
        dac_buff_2[i].ch2d = 0;
    }
    for(int i = 0; i < chl_i2sanalog_bufs; i++) {
        // dac_buff_2[i].ch1d = 0;
        dac_buff_3[i].ch1d = 0;
        dac_buff_3[i].ch2d = 0;
    }
    maini2s.clearBuffers();
    if(maini2s.set_tx_buffer(dac_buff_1, chl_i2sanalog_bufs, false) != chl_i2sanalog_bufs) { 
        printf("Buff 1 i write err!\n");
    }
    if(maini2s.set_tx_buffer(dac_buff_2, chl_i2sanalog_bufs, false) != chl_i2sanalog_bufs) {
        printf("Buff 2 i write err!\n");
    }
    chl_i2sanalog_dact* nextBuf = dac_buff_3;
    printf("IBW\n");
    maini2s.startTx();
    printf("TXS\n");
    while(true) {
        for(int i = 0; i < chl_i2sanalog_bufs; i++) {
            // nextBuf[i].ch1d = fabsf(sinf(ph)) * 255;
            nextBuf[i].ch1d = sinArr[currArrIdx];
            currArrIdx = (currArrIdx + 1) % samplesCnt;
            // ph = (ph+1) % 2;
            // nextBuf[i].ch1d = (ph * 255);
        }
        if(maini2s.set_tx_buffer(nextBuf, chl_i2sanalog_bufs, true) != chl_i2sanalog_bufs) {
            printf("Buff write err!\n");
        }
        if(nextBuf == dac_buff_3) {
            nextBuf = dac_buff_1;
        } else if(nextBuf == dac_buff_1) {
            nextBuf = dac_buff_2;
        } else {
            nextBuf = dac_buff_3;
        }
    }
    // maini2s.startRx();
    // char test_data[] = "TESTTEST0\r\n";
    // strncpy(dma_tx_buf, test_data, sizeof(dma_tx_buf)-1);
    // uint8_t testi2ctx[4] = {0x01, 0x02, 0x03, 0x04};
    // uint8_t testi2crx[4];
    // chl_i2sanalog_type1 testi2srx[256];
    // int x = 48;
    // int k = 0;
    // while(1) {
    //     dma_tx_buf[8] = x;
    //     x++;
    //     if(x >= 58) {
    //         x = 48;
    //     }
    //     dma_tx_buf[0] = 'T';
    //     mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
    //     mainuart.read_bytes((uint8_t*)dma_tx_buf+4, 1, false);
    //     testi2ctx[0]++;
    //     testi2ctx[1]++;
    //     testi2ctx[2]++;
    //     testi2ctx[3]++;
    //     maini2c.i2c_write_regs(0x60, 32, (uint8_t*)testi2ctx, 4, true, true);
    //     k = maini2c.i2c_wait_for_transaction();
    //     if(k != 0) {
    //         printf("t %d\r\n", k);
    //     } else {
    //         //printf("t ok\r\n");
    //     }
    //     maini2c.i2c_read_regs(0x60, 32, (uint8_t*)testi2crx, 4, true);
    //     k = maini2c.i2c_wait_for_transaction();
    //     if(k != 0) {
    //         printf("r %d\r\n", k);
    //     } else {
    //         printf("r %#x %#x %#x %#x\r\n", testi2crx[0], testi2crx[1], testi2crx[2], testi2crx[3]);
    //     }
    //     int i2scnt = maini2s.getRxSamplesCount();
    //     printf("i2s smpl cnt %d\r\n", i2scnt);
    //     maini2s.read_samples(testi2srx, 256*2, false);
    //     unsigned long long st = esp_timer_get_time();
    //     while(maini2s.getRxSamplesCount() < 256) {}
    //     int sampl = maini2s.read_samples(testi2srx, 256, true);
    //     unsigned long long end = esp_timer_get_time();
    //     printf("i2s 256 read time %llu us, sr %llu hz\r\n", (end-st), ((256000000ULL/(end-st))));
    //     unsigned long long int ch0avg = 0;
    //     int ch0cnt = 1;
    //     unsigned long long int ch3avg = 0;
    //     int ch3cnt = 1;
    //     for(int i = 0; i < sampl; i++) {
    //         if(testi2srx[i].channelI == 0) {
    //             ch0avg += testi2srx[i].dataI;
    //             ch0cnt++;
    //         } else if(testi2srx[i].channelI == 3) {
    //             ch3avg += testi2srx[i].dataI;
    //             ch3cnt++;
    //         } else {
    //             printf("wrongi %d\r\n", testi2srx[i].channelI);
    //         }
    //         if(testi2srx[i].channelQ == 0) {
    //             ch0avg += testi2srx[i].dataQ;
    //             ch0cnt++;
    //         } else if(testi2srx[i].channelQ == 3) {
    //             ch3avg += testi2srx[i].dataQ;
    //             ch3cnt++;
    //         } else {
    //             printf("wrongq %d\r\n", testi2srx[i].channelQ);
    //         }
    //     }
    //     printf("i2s channel 0 cnt %d avg %llu 3 cnt %d avg %llu\r\n", ch0cnt, (ch0avg/ch0cnt), ch3cnt, (ch3avg/ch3cnt));
    //     // dma_tx_buf[0] = 'K';
    //     // mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
    //     // dma_tx_buf[0] = 'L';
    //     // mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
    //     // dma_tx_buf[0] = 'M';
    //     // mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
    //     //vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     for(uint32_t i = 0; i < 15000000; i++) {
    //          __asm__ __volatile__("nop");
    //     }
    // }
}
