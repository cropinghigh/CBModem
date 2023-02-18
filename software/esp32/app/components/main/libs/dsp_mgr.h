#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_task.h"

#include "chl_uart.h"
#include "chl_i2c.h"
#include "chl_gpio.h"
#include "chl_i2sanalog.h"
#include "chl_timer.h"
#include "chl_ext_si5351.h"

#include "cdsp_fir.h"
#include "cdsp_fir_taps.h"
#include "cdsp_source.h"
#include "cdsp_sink.h"
#include "cdsp_rational_resamplers.h"
#include "cdsp_converters.h"
#include "cdsp_modulator.h"
#include "cdsp_agc.h"

#define I2C_SPD_KHZ_LO 400
#define I2C_SPD_KHZ_HI 900 //si5351 can't handle more than 900 kHz I2c signal
#define SI5351_I2C_ADDR 0x60
#define SI5351_XTAL_FREQ 25000000
#define ADC_BITWIDTH 12
#define RX_IN_SR 100000
#define RX_IN_FIR_TAPS 33
// #define TX_IN_SR 10000
#define TX_IN_SR 5000
#define TX_FIR_TAPS 33
#define TX_DAC_SR 160000
#define TX_SPLS 128
#define TX_COMPENSA 6
#define TX_COMPENSB 0
#define SDR_RX_CB_SPLS 16
#define SDR_TX_BUFF_SIZE CDSP_DEF_BUFF_SIZE*2
#define N_RX_AGC_RATE 0.6f
#define N_RX_DCBLOCK_RATE 0.9f
#define N_RX_SR 10000
#define N_RX2_SR 2000
#define N_RX_BUFF_SIZE CDSP_DEF_BUFF_SIZE
#define N_RX_SPLS 4
#define N_BFSK_TAPS 33
#define N_MFSK_TAPS 33

//Namespace is better than class with only static functions
namespace dsp_mgr {

    void init();
    void IRAM_ATTR dspcore_main(void* arg);
    int IRAM_ATTR sdr_tx_reqfunc(void* ctx, cdsp_complex_t* data, int samples_cnt);
    int IRAM_ATTR sdr_tx_carrier_reqfunc(void* ctx, cdsp_complex_t* data, int samples_cnt);
    void reset();
    void notifyDspTask();
    void IRAM_ATTR lock_dsp_mtx();
    void IRAM_ATTR unlock_dsp_mtx();

    void set_fr(float newfr);
    void rx_set_ins(bool ins);
    void general_rx_start();
    void general_rx_stop();
    void general_tx_start();
    void general_tx_stop();
    void sdr_rx_start();
    void sdr_rx_stop(bool from_dspc);
    void sdr_rx_set_sr(float newsr);
    void sdr_rx_set_cb(void (*new_sdr_rx_cb) (void*, cdsp_complex_t*, int), void* ctx);
    void sdr_tx_start();
    void sdr_tx_stop(bool from_dspc);
    void sdr_tx_carrier();
    int sdr_tx_put_data_sync(cdsp_complex_t* data, int cnt);
    void n_bfsk_tx_start(int (*n_bfsk_tx_reqfunc)(void*, uint8_t*, int), void* ctx);
    void n_bfsk_tx_stop(bool from_dspc);
    void n_bfsk_tx_set_params(float fr0, float fr1, float speed);
    void n_bfsk_tx_set_cb(void (*new_n_bfsk_tx_cb) (void*), void* ctx);
    void n_bfsk_rx_start();
    void n_bfsk_rx_stop(bool from_dspc);
    void n_bfsk_rx_set_params(float fr0, float fr1, float speed);
    void n_bfsk_rx_set_cb(void (*new_n_bfsk_rx_cb) (void*, uint8_t*, int), void* ctx);
    void n_mfsk_tx_start(int (*n_mfsk_tx_reqfunc)(void*, uint8_t*, int), void* ctx);
    void n_mfsk_tx_stop(bool from_dspc);
    void n_mfsk_tx_set_params(int frcnt, float* frs, float speed);
    void n_mfsk_tx_set_cb(void (*new_n_mfsk_tx_cb) (void*), void* ctx);
    void n_mfsk_rx_start();
    void n_mfsk_rx_stop(bool from_dspc);
    void n_mfsk_rx_set_params(int frcnt, float* frs, float speed);
    void n_mfsk_rx_set_cb(void (*new_n_mfsk_rx_cb) (void*, uint8_t*, int), void* ctx);
    void n_msk_tx_start(int (*n_msk_tx_reqfunc)(void*, uint8_t*, int), void* ctx);
    void n_msk_tx_stop(bool from_dspc);
    void n_msk_tx_set_params(float speed);
    void n_msk_tx_set_cb(void (*new_n_msk_tx_cb) (void*), void* ctx);
    void n_msk_rx_start();
    void n_msk_rx_stop(bool from_dspc);
    void n_msk_rx_set_params(float speed);
    void n_msk_rx_set_cb(void (*new_n_msk_rx_cb) (void*, uint8_t*, int), void* ctx);


    TaskHandle_t dspcore_task_hdl = NULL;
    chl_i2c* maini2c = NULL;
    chl_ext_si5351* mainsi5351 = NULL;
    chl_i2sanalog* maini2s = NULL;

    cdsp_src_adc* dsp_mainadcsrc;
    cdsp_sink_combined* dsp_maincombsink;
    float* dsp_rxinfir_taps;
    cdsp_fir<float, cdsp_complex_t>* dsp_rxinfir;
    cdsp_rational_decimator<cdsp_complex_t>* dsp_rxindecim;
    float* dsp_nrx2fir_taps;
    cdsp_fir<float, cdsp_complex_t>* dsp_nrx2fir;
    cdsp_rational_decimator<cdsp_complex_t>* dsp_nrx2decim;
    cdsp_dcblock<cdsp_complex_t>* dsp_nrxdcb;
    cdsp_agc<cdsp_complex_t>* dsp_nrxagc;
    cdsp_mod_bfsk* dsp_n_bfskmod;
    cdsp_demod_bfsk* dsp_n_bfskdemod;
    cdsp_mod_mfsk* dsp_n_mfskmod;
    cdsp_demod_mfsk* dsp_n_mfskdemod;
    cdsp_mod_msk* dsp_n_mskmod;
    cdsp_demod_msk* dsp_n_mskdemod;

    SemaphoreHandle_t dsp_mtx; //Required to avoid locking DSP core task when stopping rx/tx
    float sdr_rx_sr = 1000;
    void (*sdr_rx_cb) (void*, cdsp_complex_t*, int) = NULL;
    void* sdr_rx_cb_ctx;
    cdsp_complex_t sdr_rx_data[SDR_RX_CB_SPLS];
    cdsp_complex_t sdr_rx_buff_a[SDR_RX_CB_SPLS];
    cdsp_complex_t sdr_rx_buff_b[SDR_RX_CB_SPLS];
    bool curr_sdr_rx_buff = false;
    cdsp_complex_t sdr_tx_buff[SDR_TX_BUFF_SIZE];
    int sdr_tx_buff_rptr = 0;
    int sdr_tx_buff_wptr = 0;
    TaskHandle_t sdr_tx_waiting_task_hdl = NULL;
    bool sdr_receiving = false;
    bool sdr_transmitting = false;
    uint8_t n_rx_data[N_RX_BUFF_SIZE];
    void (*n_bfsk_tx_cb) (void*) = NULL;
    void* n_bfsk_tx_cb_ctx;
    void (*n_bfsk_rx_cb) (void*, uint8_t*, int) = NULL;
    void* n_bfsk_rx_cb_ctx;
    void (*n_mfsk_tx_cb) (void*) = NULL;
    void* n_mfsk_tx_cb_ctx;
    void (*n_mfsk_rx_cb) (void*, uint8_t*, int) = NULL;
    void* n_mfsk_rx_cb_ctx;
    void (*n_msk_tx_cb) (void*) = NULL;
    void* n_msk_tx_cb_ctx;
    void (*n_msk_rx_cb) (void*, uint8_t*, int) = NULL;
    void* n_msk_rx_cb_ctx;
    bool n_bfsk_transmitting = false;
    bool n_bfsk_receiving = false;
    bool n_mfsk_transmitting = false;
    bool n_mfsk_receiving = false;
    bool n_msk_transmitting = false;
    bool n_msk_receiving = false;
};

void dsp_mgr::init() {
    if(dspcore_task_hdl != NULL) { return; } //already initialized

    dsp_mtx = xSemaphoreCreateMutex();
    if(dsp_mtx == NULL) {
        printf("NOT ENOUGH MEMORY FOR DSP MUTEX\r\n");
        return;
    }

    xTaskCreatePinnedToCore(dspcore_main, "DSPMAINTSK", 3584, NULL, ESP_TASK_TIMER_PRIO, &dspcore_task_hdl, 1); //Create main task on DSP core(1)
    while(maini2c == NULL || maini2s == NULL || dsp_mainadcsrc == NULL || dsp_maincombsink == NULL) {vTaskDelay(1 / portTICK_PERIOD_MS);} //wait for the second core to init interrupts
    printf("Main: I2s&I2c inited!\n");

    dsp_rxinfir_taps = new float[RX_IN_FIR_TAPS];
    dsp_nrx2fir_taps = new float[RX_IN_FIR_TAPS];
    dsp_rxinfir = new cdsp_fir<float, cdsp_complex_t>(RX_IN_FIR_TAPS, dsp_rxinfir_taps);
    dsp_rxindecim = new cdsp_rational_decimator<cdsp_complex_t>(RX_IN_SR/sdr_rx_sr);
    dsp_nrx2fir = new cdsp_fir<float, cdsp_complex_t>(RX_IN_FIR_TAPS, dsp_nrx2fir_taps);
    dsp_nrx2decim = new cdsp_rational_decimator<cdsp_complex_t>(N_RX_SR/N_RX2_SR);
    dsp_nrxdcb = new cdsp_dcblock<cdsp_complex_t>(N_RX_DCBLOCK_RATE);
    dsp_nrxagc = new cdsp_agc<cdsp_complex_t>(N_RX_AGC_RATE);
    dsp_n_bfskmod = new cdsp_mod_bfsk(TX_IN_SR, -200.0f, 200.0f, 10.0f, N_BFSK_TAPS);
    dsp_n_bfskdemod = new cdsp_demod_bfsk(N_RX2_SR, -200.0f, 200.0f, 100.0f, 0.001f, 0.707f, 0.01f);
    float frs[2] = {-200.0f, 200.0f};
    dsp_n_mfskmod = new cdsp_mod_mfsk(TX_IN_SR, 2, frs, 10.0f, N_MFSK_TAPS);
    dsp_n_mfskdemod = new cdsp_demod_mfsk(N_RX2_SR, 2, frs, 100.0f, 0.02f, 0.707f, 0.01f);
    dsp_n_mskmod = new cdsp_mod_msk(TX_IN_SR, 200.0f);
    dsp_n_mskdemod = new cdsp_demod_msk(N_RX2_SR, 100.0f, (N_RX2_SR/100.0f)*0.0002f, 0.707f, 0.01f);
    printf("Main: dsp mgr init success! Free memory: %lu bytes\n", esp_get_free_heap_size());
}

void dsp_mgr::dspcore_main(void* arg) {
    printf("DSP core: %d\n", xPortGetCoreID());
    //Move interrupts to the DSP core for speed
    maini2c = new chl_i2c(0, I2C_SPD_KHZ_LO, MAINI2C_SDA_GPIO, MAINI2C_SCL_GPIO);
    maini2s = new chl_i2sanalog();
    mainsi5351 = new chl_ext_si5351(SI5351_I2C_ADDR, maini2c, SI5351_XTAL_FREQ);
    dsp_mainadcsrc = new cdsp_src_adc(maini2s, MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH, ADC_BITWIDTH);
    dsp_maincombsink = new cdsp_sink_combined(maini2s, mainsi5351, TX_COMPENSA, TX_COMPENSB, MAINI2S_DAC_CH, false, TX_IN_SR, TX_DAC_SR/TX_IN_SR, TX_FIR_TAPS);
    while(true) {
        while(sdr_receiving) {
            lock_dsp_mtx();
            if(!sdr_receiving) { unlock_dsp_mtx(); break; } //RX Stopped
            // int r = dsp_rxindecim->requestData(dsp_rxindecim, sdr_rx_data, SDR_RX_CB_SPLS);
            int r = dsp_nrxagc->requestData(dsp_nrxagc, sdr_rx_data, SDR_RX_CB_SPLS);
            if(r > 0) {
                for(int i = 0; i < r; i++) {
                    (curr_sdr_rx_buff ? sdr_rx_buff_a : sdr_rx_buff_b)[i] = sdr_rx_data[i];
                }
                if(sdr_rx_cb != NULL) {
                    sdr_rx_cb(sdr_rx_cb_ctx, (curr_sdr_rx_buff ? sdr_rx_buff_a : sdr_rx_buff_b), r);
                }
                curr_sdr_rx_buff = !curr_sdr_rx_buff;
            } else if(r < 0) {
                printf("RX ERR %d!\n", r);
                sdr_rx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        while(sdr_transmitting) {
            lock_dsp_mtx();
            if(!sdr_transmitting) { unlock_dsp_mtx(); break; } //TX Stopped
            int r = dsp_maincombsink->work(dsp_maincombsink, TX_SPLS);
            if(r > 0) {} else if(r < 0) {
                printf("TX ERR %d!\n", r);
                sdr_tx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        while(n_bfsk_transmitting) {
            lock_dsp_mtx();
            if(!n_bfsk_transmitting) { unlock_dsp_mtx(); break; } //TX Stopped
            int r = dsp_maincombsink->work(dsp_maincombsink, TX_SPLS);
            if(r > 0) {} else if(r < 0) {
                if(r == -10) {
                    //normal tx completion
                    if(n_bfsk_tx_cb != NULL) {
                        n_bfsk_tx_cb(n_bfsk_tx_cb_ctx);
                    }
                } else {
                    printf("TX ERR %d!\n", r);
                }
                n_bfsk_tx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        while(n_bfsk_receiving) {
            lock_dsp_mtx();
            if(!n_bfsk_receiving) { unlock_dsp_mtx(); break; } //RX Stopped
            int r = dsp_n_bfskdemod->requestData(dsp_n_bfskdemod, n_rx_data, N_RX_SPLS);
            if(r > 0) {
                if(n_bfsk_rx_cb != NULL) {
                    n_bfsk_rx_cb(n_bfsk_rx_cb_ctx, n_rx_data, r);
                }
            } else if(r < 0) {
                printf("RX ERR %d!\n", r);
                n_bfsk_rx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        while(n_mfsk_transmitting) {
            lock_dsp_mtx();
            if(!n_mfsk_transmitting) { unlock_dsp_mtx(); break; } //TX Stopped
            int r = dsp_maincombsink->work(dsp_maincombsink, TX_SPLS);
            if(r > 0) {} else if(r < 0) {
                if(r == -10) {
                    //normal tx completion
                    if(n_mfsk_tx_cb != NULL) {
                        n_mfsk_tx_cb(n_mfsk_tx_cb_ctx);
                    }
                } else {
                    printf("TX ERR %d!\n", r);
                }
                n_mfsk_tx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        while(n_mfsk_receiving) {
            lock_dsp_mtx();
            if(!n_mfsk_receiving) { unlock_dsp_mtx(); break; } //RX Stopped
            int r = dsp_n_mfskdemod->requestData(dsp_n_mfskdemod, n_rx_data, N_RX_SPLS);
            if(r > 0) {
                if(n_mfsk_rx_cb != NULL) {
                    n_mfsk_rx_cb(n_mfsk_rx_cb_ctx, n_rx_data, r);
                }
            } else if(r < 0) {
                printf("RX ERR %d!\n", r);
                n_mfsk_rx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        while(n_msk_transmitting) {
            lock_dsp_mtx();
            if(!n_msk_transmitting) { unlock_dsp_mtx(); break; } //TX Stopped
            int r = dsp_maincombsink->work(dsp_maincombsink, TX_SPLS);
            if(r > 0) {} else if(r < 0) {
                if(r == -10) {
                    //normal tx completion
                    if(n_msk_tx_cb != NULL) {
                        n_msk_tx_cb(n_mfsk_tx_cb_ctx);
                    }
                } else {
                    printf("TX ERR %d!\n", r);
                }
                n_msk_tx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        while(n_msk_receiving) {
            lock_dsp_mtx();
            if(!n_msk_receiving) { unlock_dsp_mtx(); break; } //RX Stopped
            int r = dsp_n_mskdemod->requestData(dsp_n_mskdemod, n_rx_data, N_RX_SPLS);
            if(r > 0) {
                if(n_msk_rx_cb != NULL) {
                    n_msk_rx_cb(n_mfsk_rx_cb_ctx, n_rx_data, r);
                }
            } else if(r < 0) {
                printf("RX ERR %d!\n", r);
                n_msk_rx_stop(true);
            } else {
                printf("WARN: 0 DATA!\n");
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

int dsp_mgr::sdr_tx_reqfunc(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    for(int i = 0; i < samples_cnt; i++) {
        data[i] = sdr_tx_buff[sdr_tx_buff_rptr];
        if(sdr_tx_buff_rptr != sdr_tx_buff_wptr) {
            sdr_tx_buff_rptr = (sdr_tx_buff_rptr+1) % SDR_TX_BUFF_SIZE;
        } else {
            printf("BU\n");
        }
    }
    if(sdr_tx_waiting_task_hdl != NULL) {
        xTaskNotifyGive(sdr_tx_waiting_task_hdl);
    }
    return samples_cnt;
}

int dsp_mgr::sdr_tx_carrier_reqfunc(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    for(int i = 0; i < samples_cnt; i++) {
        data[i].i = 1.0f;
        data[i].q = 0.0f;
    }
    return samples_cnt;
}

void dsp_mgr::reset() {
    lock_dsp_mtx();
    pin_mgr::set_txp_enable(false);
    sdr_receiving = false;
    sdr_transmitting = false;
    n_bfsk_transmitting = false;
    n_bfsk_receiving = false;
    n_mfsk_transmitting = false;
    n_mfsk_receiving = false;
    n_msk_transmitting = false;
    n_msk_receiving = false;
    dsp_maincombsink->stop(true);
    dsp_rxindecim->stop(true);
    pin_mgr::set_txled_enable(false);
    pin_mgr::set_rxled_enable(false);
    mainsi5351->set_output_enabled(false, false);
    unlock_dsp_mtx();
}

void dsp_mgr::notifyDspTask() {
    xTaskNotifyGive(dspcore_task_hdl);
}

void dsp_mgr::lock_dsp_mtx() {
    if(xSemaphoreTake(dsp_mtx, portMAX_DELAY) != pdTRUE) {
        printf("DSP MTX LOCK FAIL!\r\n");
    }
}

void dsp_mgr::unlock_dsp_mtx() {
    xSemaphoreGive(dsp_mtx);
}

void dsp_mgr::set_fr(float newfr) {
    mainsi5351->set_frequency(false, roundf(newfr));
}

void dsp_mgr::rx_set_ins(bool ins) {
    lock_dsp_mtx();
    if(!ins) {
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_LOW_CH, MAINI2S_Q_LOW_CH);
    } else {
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
    }
    unlock_dsp_mtx();
}

void dsp_mgr::general_rx_start() {
    mainsi5351->set_output_enabled(false, true);
    int realsr = maini2s->setSampleRate(RX_IN_SR);
    pin_mgr::set_rxled_enable(true);
}

void dsp_mgr::general_rx_stop() {
    mainsi5351->set_output_enabled(false, false);
    pin_mgr::set_rxled_enable(false);
}

void dsp_mgr::general_tx_start() {
    pin_mgr::set_txp_enable(true);
    mainsi5351->set_output_enabled(true, true);
    // maini2c->force_gpio(true);
    maini2c->set_speed(I2C_SPD_KHZ_HI);
    int realsr = maini2s->setSampleRate(TX_DAC_SR);
    dsp_maincombsink->start(true);
    pin_mgr::set_txled_enable(true);
    notifyDspTask();
}

void dsp_mgr::general_tx_stop() {
    pin_mgr::set_txp_enable(false);
    dsp_maincombsink->stop(true);
    // maini2c->force_gpio(false);
    maini2c->set_speed(I2C_SPD_KHZ_LO);
    mainsi5351->set_output_enabled(false, false);
    pin_mgr::set_txled_enable(false);
}

void dsp_mgr::sdr_rx_start() {
    if(!sdr_receiving && !sdr_transmitting) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        // dsp_rxindecim->setDecimation(roundf(RX_IN_SR/sdr_rx_sr));
        // cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, RX_IN_FIR_TAPS, RX_IN_SR, sdr_rx_sr/2.0f, true);
        // // //Make the chain
        // dsp_rxindecim->setInputBlk(dsp_rxinfir, dsp_rxinfir->requestData);
        // dsp_rxinfir->setInputBlk(dsp_mainadcsrc, dsp_mainadcsrc->requestData);
        // // //Start last block
        // dsp_rxindecim->start(true);
        
        //Configure blocks, calculate taps
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
        dsp_rxindecim->setDecimation(roundf(RX_IN_SR/N_RX_SR));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, RX_IN_FIR_TAPS, RX_IN_SR, N_RX_SR/2.0f, true);
        dsp_nrx2decim->setDecimation(roundf(N_RX_SR/N_RX2_SR));
        cdsp_calc_taps_lpf_float(dsp_nrx2fir_taps, RX_IN_FIR_TAPS, N_RX_SR, N_RX2_SR/2.0f, true);
        //Make the chain
        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
        dsp_nrxdcb->setInputBlk(dsp_nrx2decim, dsp_nrx2decim->requestData);
        dsp_nrx2decim->setInputBlk(dsp_nrx2fir, dsp_nrx2fir->requestData);
        dsp_nrx2fir->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        dsp_rxindecim->setInputBlk(dsp_rxinfir, dsp_rxinfir->requestData);
        dsp_rxinfir->setInputBlk(dsp_mainadcsrc, dsp_mainadcsrc->requestData);
        //Start last block
        general_rx_start();
        dsp_nrxagc->start(true);
        sdr_receiving = true;
        notifyDspTask();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::sdr_rx_stop(bool from_dspc) {
    if(sdr_receiving) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        sdr_receiving = false;
        dsp_rxindecim->stop(true);
        // dsp_nrxagc->stop(true);
        general_rx_stop();
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::sdr_rx_set_sr(float newsr) {
    if(!sdr_receiving) {
        lock_dsp_mtx();
        sdr_rx_sr = newsr;
        unlock_dsp_mtx();
    }
}

void dsp_mgr::sdr_rx_set_cb(void (*new_sdr_rx_cb) (void*, cdsp_complex_t*, int), void* ctx) {
    lock_dsp_mtx();
    sdr_rx_cb = new_sdr_rx_cb;
    sdr_rx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::sdr_tx_start() {
    if(!sdr_transmitting && !sdr_receiving) {
        lock_dsp_mtx();
        dsp_maincombsink->setInputFunc(NULL, sdr_tx_reqfunc);
        sdr_transmitting = true;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::sdr_tx_stop(bool from_dspc) {
    if(sdr_transmitting) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        sdr_transmitting = false;
        general_tx_stop();
        if(sdr_tx_waiting_task_hdl != NULL) {
            xTaskNotifyGive(sdr_tx_waiting_task_hdl);
        }
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::sdr_tx_carrier() {
    if(!sdr_transmitting && !sdr_receiving) {
        lock_dsp_mtx();
        dsp_maincombsink->setInputFunc(NULL, sdr_tx_carrier_reqfunc);
        sdr_transmitting = true;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

int dsp_mgr::sdr_tx_put_data_sync(cdsp_complex_t* data, int cnt) {
    for(int i = 0; i < cnt; i++) {
        while((sdr_tx_buff_wptr+1)%SDR_TX_BUFF_SIZE != sdr_tx_buff_rptr) {
            if(!sdr_transmitting || sdr_tx_waiting_task_hdl != NULL) {
                printf("BO\n");
                return i;
            }
            sdr_tx_waiting_task_hdl = xTaskGetCurrentTaskHandle();
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            sdr_tx_waiting_task_hdl = NULL;
        }
        sdr_tx_buff[sdr_tx_buff_wptr] = data[i];
        sdr_tx_buff_wptr = (sdr_tx_buff_wptr+1) % SDR_TX_BUFF_SIZE;
    }
    return cnt;
}

void dsp_mgr::n_bfsk_tx_start(int (*n_bfsk_tx_reqfunc)(void*, uint8_t*, int), void* ctx) {
    if(!n_bfsk_transmitting && !n_bfsk_receiving) {
        lock_dsp_mtx();
        dsp_n_bfskmod->setInputFunc(ctx, n_bfsk_tx_reqfunc);
        dsp_maincombsink->setInputBlk(dsp_n_bfskmod, dsp_n_bfskmod->requestData);
        n_bfsk_transmitting = true;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_bfsk_tx_stop(bool from_dspc) {
    if(n_bfsk_transmitting) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        n_bfsk_transmitting = false;
        general_tx_stop();
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_bfsk_tx_set_params(float fr0, float fr1, float speed) {
    if(!n_bfsk_transmitting) {
        lock_dsp_mtx();
        dsp_n_bfskmod->setFrs(fr0, fr1);
        dsp_n_bfskmod->setDataRate(speed);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_bfsk_tx_set_cb(void (*new_n_bfsk_tx_cb) (void*), void* ctx) {
    lock_dsp_mtx();
    n_bfsk_tx_cb = new_n_bfsk_tx_cb;
    n_bfsk_tx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::n_bfsk_rx_start() {
    if(!n_bfsk_transmitting && !n_bfsk_receiving) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
        dsp_rxindecim->setDecimation(roundf(RX_IN_SR/N_RX_SR));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, RX_IN_FIR_TAPS, RX_IN_SR, N_RX_SR/2.0f, true);
        dsp_nrx2decim->setDecimation(roundf(N_RX_SR/N_RX2_SR));
        cdsp_calc_taps_lpf_float(dsp_nrx2fir_taps, RX_IN_FIR_TAPS, N_RX_SR, N_RX2_SR/2.0f, true);
        //Make the chain
        dsp_n_bfskdemod->setInputBlk(dsp_nrxagc, dsp_nrxagc->requestData);
        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
        dsp_nrxdcb->setInputBlk(dsp_nrx2decim, dsp_nrx2decim->requestData);
        dsp_nrx2decim->setInputBlk(dsp_nrx2fir, dsp_nrx2fir->requestData);
        dsp_nrx2fir->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        dsp_rxindecim->setInputBlk(dsp_rxinfir, dsp_rxinfir->requestData);
        dsp_rxinfir->setInputBlk(dsp_mainadcsrc, dsp_mainadcsrc->requestData);
        //Start last block
        general_rx_start();
        dsp_n_bfskdemod->start(true);
        n_bfsk_receiving = true;
        notifyDspTask();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_bfsk_rx_stop(bool from_dspc) {
    if(n_bfsk_receiving) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        n_bfsk_receiving = false;
        dsp_n_bfskdemod->stop(true);
        general_rx_stop();
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_bfsk_rx_set_params(float fr0, float fr1, float speed) {
    if(!n_bfsk_receiving) {
        lock_dsp_mtx();
        dsp_n_bfskdemod->setFrs(fr0, fr1);
        dsp_n_bfskdemod->setDataRate(speed);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_bfsk_rx_set_cb(void (*new_n_bfsk_rx_cb) (void*, uint8_t*, int), void* ctx) {
    lock_dsp_mtx();
    n_bfsk_rx_cb = new_n_bfsk_rx_cb;
    n_bfsk_rx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::n_mfsk_tx_start(int (*n_mfsk_tx_reqfunc)(void*, uint8_t*, int), void* ctx) {
    if(!n_mfsk_transmitting) {
        lock_dsp_mtx();
        dsp_n_mfskmod->setInputFunc(ctx, n_mfsk_tx_reqfunc);
        dsp_maincombsink->setInputBlk(dsp_n_mfskmod, dsp_n_mfskmod->requestData);
        n_mfsk_transmitting = true;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_mfsk_tx_stop(bool from_dspc) {
    if(n_mfsk_transmitting) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        n_mfsk_transmitting = false;
        general_tx_stop();
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_mfsk_tx_set_params(int frcnt, float* frs, float speed) {
    if(!n_mfsk_transmitting) {
        lock_dsp_mtx();
        dsp_n_mfskmod->setFrs(frcnt, frs);
        dsp_n_mfskmod->setDataRate(speed);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_mfsk_tx_set_cb(void (*new_n_mfsk_tx_cb) (void*), void* ctx) {
    lock_dsp_mtx();
    n_mfsk_tx_cb = new_n_mfsk_tx_cb;
    n_mfsk_tx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::n_mfsk_rx_start() {
    if(!n_mfsk_transmitting && !n_mfsk_receiving) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
        dsp_rxindecim->setDecimation(roundf(RX_IN_SR/N_RX_SR));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, RX_IN_FIR_TAPS, RX_IN_SR, N_RX_SR/2.0f, true);
        dsp_nrx2decim->setDecimation(roundf(N_RX_SR/N_RX2_SR));
        cdsp_calc_taps_lpf_float(dsp_nrx2fir_taps, RX_IN_FIR_TAPS, N_RX_SR, N_RX2_SR/2.0f, true);
        //Make the chain
        dsp_n_mfskdemod->setInputBlk(dsp_nrxagc, dsp_nrxagc->requestData);
        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
        dsp_nrxdcb->setInputBlk(dsp_nrx2decim, dsp_nrx2decim->requestData);
        dsp_nrx2decim->setInputBlk(dsp_nrx2fir, dsp_nrx2fir->requestData);
        dsp_nrx2fir->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        dsp_rxindecim->setInputBlk(dsp_rxinfir, dsp_rxinfir->requestData);
        dsp_rxinfir->setInputBlk(dsp_mainadcsrc, dsp_mainadcsrc->requestData);
        //Start last block
        general_rx_start();
        dsp_n_mfskdemod->start(true);
        n_mfsk_receiving = true;
        notifyDspTask();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_mfsk_rx_stop(bool from_dspc) {
    if(n_mfsk_receiving) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        n_mfsk_receiving = false;
        dsp_n_mfskdemod->stop(true);
        general_rx_stop();
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_mfsk_rx_set_params(int frcnt, float* frs, float speed) {
    if(!n_mfsk_receiving) {
        lock_dsp_mtx();
        dsp_n_mfskdemod->setFrs(frcnt, frs);
        dsp_n_mfskdemod->setDataRate(speed);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_mfsk_rx_set_cb(void (*new_n_mfsk_rx_cb) (void*, uint8_t*, int), void* ctx) {
    lock_dsp_mtx();
    n_mfsk_rx_cb = new_n_mfsk_rx_cb;
    n_mfsk_rx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::n_msk_tx_start(int (*n_msk_tx_reqfunc)(void*, uint8_t*, int), void* ctx) {
    if(!n_msk_transmitting) {
        lock_dsp_mtx();
        set_fr(mainsi5351->get_curr_center_freq() + dsp_n_mskmod->getDataRate()+100.0f);
        dsp_n_mskmod->setInputFunc(ctx, n_msk_tx_reqfunc);
        dsp_maincombsink->setInputBlk(dsp_n_mskmod, dsp_n_mskmod->requestData);
        n_msk_transmitting = true;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_msk_tx_stop(bool from_dspc) {
    if(n_msk_transmitting) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        n_msk_transmitting = false;
        general_tx_stop();
        set_fr(mainsi5351->get_curr_center_freq() - dsp_n_mskmod->getDataRate() - 100.0f);
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_msk_tx_set_params(float speed) {
    if(!n_msk_transmitting) {
        lock_dsp_mtx();
        dsp_n_mskmod->setDataRate(speed);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_msk_tx_set_cb(void (*new_n_msk_tx_cb) (void*), void* ctx) {
    lock_dsp_mtx();
    n_msk_tx_cb = new_n_msk_tx_cb;
    n_msk_tx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::n_msk_rx_start() {
    if(!n_msk_transmitting && !n_msk_receiving) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
        dsp_rxindecim->setDecimation(roundf(RX_IN_SR/N_RX_SR));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, RX_IN_FIR_TAPS, RX_IN_SR, N_RX_SR/2.0f, true);
        dsp_nrx2decim->setDecimation(roundf(N_RX_SR/N_RX2_SR));
        cdsp_calc_taps_lpf_float(dsp_nrx2fir_taps, RX_IN_FIR_TAPS, N_RX_SR, N_RX2_SR/2.0f, true);
        //Make the chain
        dsp_n_mskdemod->setInputBlk(dsp_nrxagc, dsp_nrxagc->requestData);
        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
        dsp_nrxdcb->setInputBlk(dsp_nrx2decim, dsp_nrx2decim->requestData);
        dsp_nrx2decim->setInputBlk(dsp_nrx2fir, dsp_nrx2fir->requestData);
        dsp_nrx2fir->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        dsp_rxindecim->setInputBlk(dsp_rxinfir, dsp_rxinfir->requestData);
        dsp_rxinfir->setInputBlk(dsp_mainadcsrc, dsp_mainadcsrc->requestData);
        //Start last block
        general_rx_start();
        dsp_n_mskdemod->start(true);
        n_msk_receiving = true;
        notifyDspTask();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_msk_rx_stop(bool from_dspc) {
    if(n_msk_receiving) {
        if(!from_dspc) {
            lock_dsp_mtx();
        }
        n_msk_receiving = false;
        dsp_n_mskdemod->stop(true);
        general_rx_stop();
        if(!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_msk_rx_set_params(float speed) {
    if(!n_msk_transmitting) {
        lock_dsp_mtx();
        dsp_n_mskdemod->setDataRate(speed);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_msk_rx_set_cb(void (*new_n_msk_rx_cb) (void*, uint8_t*, int), void* ctx) {
    lock_dsp_mtx();
    n_msk_rx_cb = new_n_msk_rx_cb;
    n_msk_rx_cb_ctx = ctx;
    unlock_dsp_mtx();
}




