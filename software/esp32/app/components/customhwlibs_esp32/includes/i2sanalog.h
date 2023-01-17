#pragma once
#include <soc/i2s_periph.h>
#include <soc/adc_periph.h>
#include <soc/dac_periph.h>
#include <soc/regi2c_apll.h>
#include <soc/syscon_periph.h>
#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <esp_system.h>
#include <soc/dport_reg.h>
#include <esp_private/periph_ctrl.h>
#include <esp_private/regi2c_ctrl.h>
#include <clk_ctrl_os.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/stream_buffer.h>
#include <rom/lldesc.h>

#include "gpio.h"

#include <rom/ets_sys.h>

#define DMA_I2SANALOG_RX_STREAM_CNT 256
#define DMA_I2SANALOG_RX_LINK_BUFF_CNT 64

#define I2SANALOG_INTERRUPTS_ENA (I2S_IN_SUC_EOF_INT_ENA | I2S_OUT_DONE_INT_ENA)
#define I2SANALOG_INTERRUPTS_RAW (I2S_IN_SUC_EOF_INT_RAW | I2S_OUT_DONE_INT_RAW)

struct chl_i2sanalog_type1 {
    uint16_t dataI: 12;
    uint8_t channelI: 4;
    uint16_t dataQ: 12;
    uint8_t channelQ: 4;
};

struct chl_i2sanalog_dact {
    uint8_t ch1pad;
    uint8_t ch1d;
    uint8_t ch2pad;
    uint8_t ch2d;
};

//only I2S0 could work with ADC1/DACs
class chl_i2sanalog {
public:
    chl_i2sanalog();
    ~chl_i2sanalog();
    void configureADC(int ichan, int qchan, int bitwidth, int attenuation);
    //min-1000, max-4M(theoretically)
    int setSampleRate(int sr);
    void startRx();
    void stopRx();
    void startTx();
    void stopTx();
    void clearBuffers();
    int read_samples(chl_i2sanalog_type1* buf, unsigned int count, bool block);
    int set_tx_buffer(chl_i2sanalog_dact* buf, unsigned int count, bool block);
    int getRxSamplesCount();
private:
    SemaphoreHandle_t _rx_streambuffer_mtx;
    SemaphoreHandle_t _tx_links_mtx;
    TaskHandle_t _tx_links_task_hdl;
    StreamBufferHandle_t _xRxStreamBuffer;
    intr_handle_t _i2sanalog_intr_hdl;
    chl_i2sanalog_type1* _chl_i2sanalog_dma_rx_buff; //DMA_I2SANALOG_RX_LINK_BUFF_CNT x 2
    lldesc_t* _chl_i2sanalog_dma_inlinks; //2
    int _chl_i2sanalog_curr_inlink;
    lldesc_t* _chl_i2sanalog_dma_outlinks; //2
    int _chl_i2sanalog_curr_outlink;

    void _reset_module();
    static void IRAM_ATTR _i2sanalog_intr_hdlr(void* arg);
};
