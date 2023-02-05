#pragma once

#include "chl_gpio.h"

#define MAINUART_RXD_GPIO 3
#define MAINUART_TXD_GPIO 1
#define MAINI2C_SDA_GPIO 27
#define MAINI2C_SCL_GPIO 14
#define TXP_ENABLE_GPIO 2
#define DCD_GPIO 5
#define TX_LED_GPIO 12
#define USB_LED_GPIO 32
#define RX_LED_GPIO 33

#define MAINI2S_I_HIGH_CH 0
#define MAINI2S_Q_HIGH_CH 3
#define MAINI2S_I_LOW_CH 6
#define MAINI2S_Q_LOW_CH 7

#define MAINI2S_DAC_CH 1

class pin_mgr {
public:
    static void init() {
        chl_gpio_iomux_select_func(TXP_ENABLE_GPIO, PIN_FUNC_GPIO);
        chl_gpio_iomux_select_func(DCD_GPIO, PIN_FUNC_GPIO);
        chl_gpio_iomux_select_func(TX_LED_GPIO, PIN_FUNC_GPIO);
        chl_gpio_iomux_select_func(USB_LED_GPIO, PIN_FUNC_GPIO);
        chl_gpio_iomux_select_func(RX_LED_GPIO, PIN_FUNC_GPIO);
        chl_gpio_set_level(TXP_ENABLE_GPIO, false);
        chl_gpio_set_level(DCD_GPIO, false);
        chl_gpio_set_level(TX_LED_GPIO, false);
        chl_gpio_set_level(USB_LED_GPIO, false);
        chl_gpio_set_level(RX_LED_GPIO, false);
        chl_gpio_connect_out(TXP_ENABLE_GPIO, SIG_GPIO_OUT_IDX, false);
        chl_gpio_connect_out(DCD_GPIO, SIG_GPIO_OUT_IDX, false);
        chl_gpio_connect_out(TX_LED_GPIO, SIG_GPIO_OUT_IDX, false);
        chl_gpio_connect_out(USB_LED_GPIO, SIG_GPIO_OUT_IDX, false);
        chl_gpio_connect_out(RX_LED_GPIO, SIG_GPIO_OUT_IDX, false);
        chl_gpio_set_direction(TXP_ENABLE_GPIO, false, true, false, false, false);
        chl_gpio_set_direction(DCD_GPIO, false, true, false, false, false);
        chl_gpio_set_direction(TX_LED_GPIO, false, true, false, false, false);
        chl_gpio_set_direction(USB_LED_GPIO, false, true, false, false, false);
        chl_gpio_set_direction(RX_LED_GPIO, false, true, false, false, false);
    }

    static void set_txp_enable(bool en) {
        chl_gpio_set_level(TXP_ENABLE_GPIO, en);
    }
    static void set_dcd_enable(bool en) {
        chl_gpio_set_level(DCD_GPIO, en);
    }
    static void set_txled_enable(bool en) {
        chl_gpio_set_level(TX_LED_GPIO, en);
    }
    static void set_usbled_enable(bool en) {
        chl_gpio_set_level(USB_LED_GPIO, en);
    }
    static void set_rxled_enable(bool en) {
        chl_gpio_set_level(RX_LED_GPIO, en);
    }
};
