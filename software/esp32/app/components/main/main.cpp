/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart.h"

extern "C" void app_main();

DMA_ATTR static char dma_tx_buf[512];

void app_main(void)
{
    chl_uart_dma mainuart(0, 115200);
    char test_data[] = "TESTTEST0\r\n";
    char test_rx[16];
    strncpy(dma_tx_buf, test_data, sizeof(dma_tx_buf)-1);
    int x = 48;
    while(1) {
        dma_tx_buf[8] = x;
        x++;
        if(x >= 58) {
            x = 48;
        }
        dma_tx_buf[0] = 'T';
        mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
        mainuart.read_bytes((uint8_t*)test_rx, 1, false);
        printf("%c\r\n", test_rx[0]);
        // dma_tx_buf[0] = 'K';
        // mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
        // dma_tx_buf[0] = 'L';
        // mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
        // dma_tx_buf[0] = 'M';
        // mainuart.transmit_bytes((uint8_t*)dma_tx_buf, sizeof(test_data), true);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
        for(uint32_t i = 0; i < 60000000; i++) {
             __asm__ __volatile__("nop");
        }
    }
}
