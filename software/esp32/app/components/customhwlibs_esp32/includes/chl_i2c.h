#pragma once
#include <soc/i2c_periph.h>
#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <esp_system.h>
#include <soc/dport_reg.h>
#include <esp_private/periph_ctrl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "chl_gpio.h"

#include <rom/ets_sys.h>

#define I2C_QUEUE_SIZE 30
#define I2C_INTERRUPTS_ENA I2C_TX_SEND_EMPTY_INT_ENA | I2C_RX_REC_FULL_INT_ENA | I2C_ACK_ERR_INT_ENA | I2C_TIME_OUT_INT_ENA | I2C_TRANS_COMPLETE_INT_ENA | I2C_ARBITRATION_LOST_INT_ENA

struct chl_i2c_reg_write_trans {
    uint8_t devaddr;
    uint8_t startregnum;
    unsigned int len;
    bool check_ack;
    uint8_t* buff;
};

class chl_i2c {
public:
    //only master mode
    chl_i2c(int num, int sckspdkhz, int sdagpio, int sclgpio);
    ~chl_i2c();
    void force_gpio(bool force);
    void set_speed(int sckspdkhz);
    int i2c_write_regs(uint8_t devaddr, uint8_t startregnum, uint8_t* buffer, unsigned int len, bool block, bool check_ack);
    int IRAM_ATTR i2c_write_regs_isr(uint8_t devaddr, uint8_t startregnum, uint8_t* buffer, unsigned int len, bool check_ack);
    int i2c_read_regs(uint8_t devaddr, uint8_t startregnum, uint8_t* buffer, unsigned int len, bool check_ack);
    int i2c_queue_write_regs(uint8_t devaddr, uint8_t startregnum, uint8_t* buffer, unsigned int len, bool block, bool check_ack);
    int IRAM_ATTR i2c_queue_write_regs_isr(uint8_t devaddr, uint8_t startregnum, uint8_t* buffer, unsigned int len, bool check_ack, BaseType_t* yield);
    //wait for the current i2c transaction to finish; returns error code: 0-ok;1-nack;2-timeout;3-arb.lost;4-no transaction;
    //if there is no current transaction, returns result of a last one
    int i2c_wait_for_transaction();
private:
    int _number;
    int _sdagpio, _sclgpio;
    int _sckspdkhz;
    int _last_transm_state;
    bool _gpioforce = false;
    uint32_t _last_transm_end;
    intr_handle_t _i2c_intr_hdl;
    SemaphoreHandle_t _i2c_transm_bsmph;
    QueueHandle_t _i2c_transm_queue;
    volatile uint32_t* _ram_data; //32 items

    void IRAM_ATTR _set_commands_tx(uint8_t devaddr, uint8_t startregnum, uint8_t len, bool check_ack);
    void _set_commands_rx(uint8_t devaddr, uint8_t startregnum, uint8_t len, bool check_ack);
    void _reset_module();
    void _config_gpio();
    static void IRAM_ATTR _i2c_intr_hdlr(void* arg);
};
