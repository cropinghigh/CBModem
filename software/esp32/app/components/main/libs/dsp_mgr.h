#ifndef H_DSP_MGR
#define H_DSP_MGR

#include <bit>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_task.h"

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

#include "params.h"

// #define I2C_SPD_KHZ_LO 400
// #define I2C_SPD_KHZ_HI 900 //si5351 can't handle more than 900 kHz I2c signal
// #define SI5351_I2C_ADDR 0x60
// #define SI5351_XTAL_FREQ 25000000
// #define ADC_BITWIDTH 12
// #define RX_IN_SR 100000
// #define RX_IN_FIR_TAPS 33
// #define TX_IN_SR 10000
// #define TX_IN_SR 5000
// #define TX_FIR_TAPS 33
// #define TX_DAC_SR 160000
// #define TX_SPLS 128
// #define TX_COMPENSA 6
// #define TX_COMPENSB 0
// #define SDR_RX_CB_SPLS 16
#define SDR_TX_BUFF_SIZE CDSP_DEF_BUFF_SIZE*2
// #define N_RX_AGC_RATE 0.6f
// #define N_RX_DCBLOCK_RATE 0.9f
// #define N_RX_SR 10000
// #define N_RX2_SR 2000
#define N_RX_BUFF_SIZE CDSP_DEF_BUFF_SIZE
// #define N_RX_SPLS 4
// #define N_BFSK_TAPS 33
// #define N_MFSK_TAPS 33
#define MAX_AFC_SHIFT 200
#define AFC_RATE 0.5f
/*
 Params:
 dm_rift - Receiving input FIR taps, def=33
 dm_risr - Receiving input sample rate, def=100000
 dm_rdcb - Receiving DC blocker rate, def=0.9
 dm_ragc - Receiving AGC rate, def=0.6
 dm_rnsr - Normal receiving sample rate, def=2000
 dm_tisr - Transmitting input sample rate, def=10000
 dm_tbfkt - BFSK transmitting taps count, def=33
 dm_tmfkt - MFSK transmitting taps count, def=33
 dm_i2cl - Low i2c speed, khz, def=400
 dm_i2ch - High i2c speed, khz, def=900
 dm_siaddr - Si5351 I2c addr, def=0x60
 dm_sixf - Si5351 xtal freq, def=25000000
 dm_adcbw - ADC bitwidth, def=12
 dm_tca - Transmitting compensation coefficient A, def=6
 dm_tcb - Transmitting compensation coefficient B, def=0
 dm_tdsr - Transmitting DAC sample rate, def=100000
 dm_toft - Transmitting filter taps, def=33
 dm_txspls - Samples count per transmitting operation, def=128
 dm_rxspls - Samples count per normal receiving operation, def=4
 dm_sdrrxspls - Samples count per SDR receiving operation, def=16
 dm_loopbw - Receiving timing recovery loop bandwidth, def=0.0005
 */

//Namespace is better than class with only static functions
namespace dsp_mgr {

    void init();
    void reloadParams();
    void IRAM_ATTR dspcore_main(void *arg);
    int IRAM_ATTR sdr_tx_reqfunc(void *ctx, cdsp_complex_t *data, int samples_cnt);
    int IRAM_ATTR sdr_tx_carrier_reqfunc(void *ctx, cdsp_complex_t *data, int samples_cnt);
    void reset();
    void notifyDspTask();
    void IRAM_ATTR lock_dsp_mtx();
    void IRAM_ATTR unlock_dsp_mtx();
    void general_rx_start();
    void general_rx_stop();
    void general_tx_start();
    void general_tx_stop();

    void set_fr(float newfr, bool reset_afc);
    void apply_afc();
    void rx_set_ins(bool ins);
    void sdr_rx_start();
    void sdr_rx_stop(bool from_dspc);
    void sdr_rx_set_sr(float newsr);
    void sdr_rx_set_cb(void (*new_sdr_rx_cb)(void*, cdsp_complex_t*, int), void *ctx);
    void sdr_tx_start();
    void sdr_tx_stop(bool from_dspc);
    void tx_carrier();
    int IRAM_ATTR sdr_tx_put_data_sync(cdsp_complex_t *data, int cnt);
    void n_tx_set_cb(void (*new_n_tx_cb)(void*), void *ctx);
    void n_tx_set_reqfunc(int (*new_n_tx_reqfunc)(void*, uint8_t*, int), void *ctx);
    void n_rx_set_cb(void (*new_n_rx_cb)(void*, uint8_t*, int), void *ctx);
    void n_bfsk_set_params(float fr0, float fr1, float speed);
    void n_bfsk_tx_start();
    void n_bfsk_tx_stop(bool from_dspc);
    void n_bfsk_rx_start();
    void n_bfsk_rx_stop(bool from_dspc);
    void n_mfsk_set_params(int frcnt, float *frs, float speed);
    void n_mfsk_tx_start();
    void n_mfsk_tx_stop(bool from_dspc);
    void n_mfsk_rx_start();
    void n_mfsk_rx_stop(bool from_dspc);
    void n_msk_set_params(float speed);
    void n_msk_tx_start();
    void n_msk_tx_stop(bool from_dspc);
    void n_msk_rx_start();
    void n_msk_rx_stop(bool from_dspc);

    TaskHandle_t dspcore_task_hdl = NULL;
    chl_i2c *maini2c = NULL;
    chl_ext_si5351 *mainsi5351 = NULL;
    chl_i2sanalog *maini2s = NULL;

    float *dsp_rxinfir_taps = NULL;
    cdsp_src_adc *dsp_mainadcsrc;
    cdsp_sink_combined *dsp_maincombsink;
    cdsp_fir<float, cdsp_complex_t> *dsp_rxinfir;
    cdsp_rational_decimator<cdsp_complex_t> *dsp_rxindecim;
    cdsp_dcblock<cdsp_complex_t> *dsp_nrxdcb;
    cdsp_agc<cdsp_complex_t> *dsp_nrxagc;
    cdsp_mod_bfsk *dsp_n_bfskmod;
    cdsp_demod_bfsk *dsp_n_bfskdemod;
    cdsp_mod_mfsk *dsp_n_mfskmod;
    cdsp_demod_mfsk *dsp_n_mfskdemod;
    cdsp_mod_msk *dsp_n_mskmod;
    cdsp_demod_msk *dsp_n_mskdemod;

    SemaphoreHandle_t dsp_mtx; //Required to avoid locking DSP core task when stopping rx/tx
    float afc_shift = 0.0f;
    int afc_ctr = 0;
    bool afc_search_mode = true;
    float sdr_rx_sr = 1000;
    void (*sdr_rx_cb)(void*, cdsp_complex_t*, int) = NULL;
    void *sdr_rx_cb_ctx;
    cdsp_complex_t sdr_rx_data[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t sdr_rx_buff_a[CDSP_DEF_BUFF_SIZE];
    cdsp_complex_t sdr_rx_buff_b[CDSP_DEF_BUFF_SIZE];
    bool curr_sdr_rx_buff = false;
    cdsp_complex_t sdr_tx_buff[SDR_TX_BUFF_SIZE];
    int sdr_tx_buff_rptr = 0;
    int sdr_tx_buff_wptr = 0;
    TaskHandle_t sdr_tx_waiting_task_hdl = NULL;
    uint8_t n_rx_data[N_RX_BUFF_SIZE];
    void (*n_tx_cb)(void*) = NULL;
    void *n_tx_cb_ctx;
    void (*n_rx_cb)(void*, uint8_t*, int) = NULL;
    void *n_rx_cb_ctx;
    int (*n_tx_reqfunc)(void*, uint8_t*, int) = NULL;
    void *n_tx_reqfunc_ctx;

    enum mode {
        DSPMGR_MODE_IDLE,
        DSPMGR_MODE_SDR_RX,
        DSPMGR_MODE_SDR_TX,
        DSPMGR_MODE_TX_CARRIER,
        DSPMGR_MODE_BFSK_TX,
        DSPMGR_MODE_BFSK_RX,
        DSPMGR_MODE_MFSK_TX,
        DSPMGR_MODE_MFSK_RX,
        DSPMGR_MODE_MSK_TX,
        DSPMGR_MODE_MSK_RX,
    };

    mode curr_mode = DSPMGR_MODE_IDLE;
    bool rx_paused = false;
}
;

void dsp_mgr::init() {
    if (dspcore_task_hdl != NULL) {
        return;
    } //already initialized

    dsp_mtx = xSemaphoreCreateMutex();
    if (dsp_mtx == NULL) {
        printf("NOT ENOUGH MEMORY FOR DSP MUTEX\r\n");
        return;
    }

    xTaskCreatePinnedToCore(dspcore_main, "DSPMAINTSK", 3584, NULL, ESP_TASK_TIMER_PRIO, &dspcore_task_hdl, 1); //Create main task on DSP core(1)
    while (maini2c == NULL || maini2s == NULL || dsp_mainadcsrc == NULL || dsp_maincombsink == NULL) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    } //wait for the second core to init interrupts
    printf("Main: I2s&I2c inited!\n");

    dsp_rxinfir_taps = new float[params::readParam("dm_rift", 33)];
    dsp_rxinfir = new cdsp_fir<float, cdsp_complex_t>(params::readParam("dm_rift", 33), dsp_rxinfir_taps);
    dsp_rxindecim = new cdsp_rational_decimator<cdsp_complex_t>(params::readParam("dm_risr", 100000) / sdr_rx_sr);
    uint32_t dcbr_def = std::bit_cast < uint32_t > (0.9f);
    uint32_t agcr_def = std::bit_cast < uint32_t > (0.9f);
    uint32_t raw_dcbr = params::readParam("dm_rdcb", dcbr_def);
    uint32_t raw_agcr = params::readParam("dm_ragc", agcr_def);
    float dcb_rate = std::bit_cast<float>(raw_dcbr);
    float agc_rate = std::bit_cast<float>(raw_agcr);
    dsp_nrxdcb = new cdsp_dcblock<cdsp_complex_t>(dcb_rate);
    dsp_nrxagc = new cdsp_agc<cdsp_complex_t>(agc_rate);
    dsp_n_bfskmod = new cdsp_mod_bfsk(params::readParam("dm_tisr", 10000), -200.0f, 200.0f, 10.0f, params::readParam("dm_tbfkt", 33));
    dsp_n_bfskdemod = new cdsp_demod_bfsk(params::readParam("dm_rnsr", 2000), -200.0f, 200.0f, 100.0f, 0.001f, 0.707f, 0.01f);
    float frs[2] = { -200.0f, 200.0f };
    dsp_n_mfskmod = new cdsp_mod_mfsk(params::readParam("dm_tisr", 10000), 2, frs, 10.0f, params::readParam("dm_tmfkt", 33));
    dsp_n_mfskdemod = new cdsp_demod_mfsk(params::readParam("dm_rnsr", 2000), 2, frs, 100.0f, 0.02f, 0.707f, 0.01f);
    dsp_n_mskmod = new cdsp_mod_msk(params::readParam("dm_tisr", 10000), 200.0f);
    dsp_n_mskdemod = new cdsp_demod_msk(params::readParam("dm_rnsr", 2000), 100.0f, 0.001f, 0.707f, 0.01f);
    dsp_rxindecim->setInputBlk(dsp_rxinfir, dsp_rxinfir->requestData);
    dsp_rxinfir->setInputBlk(dsp_mainadcsrc, dsp_mainadcsrc->requestData);
    printf("Main: dsp mgr init success! Free memory: %lu bytes\n", esp_get_free_heap_size());
}

void dsp_mgr::reloadParams() {
    lock_dsp_mtx();
    if (dsp_rxinfir_taps != NULL) {
        delete[] dsp_rxinfir_taps;
        dsp_rxinfir_taps = NULL;
    }
    dsp_rxinfir_taps = new float[params::readParam("dm_rift", 33)];
    dsp_rxinfir->setTaps(params::readParam("dm_rift", 33), dsp_rxinfir_taps);
    dsp_rxindecim->setDecimation(params::readParam("dm_risr", 100000) / sdr_rx_sr);
    uint32_t dcbr_def = std::bit_cast < uint32_t > (0.9f);
    uint32_t agcr_def = std::bit_cast < uint32_t > (0.9f);
    uint32_t raw_dcbr = params::readParam("dm_rdcb", dcbr_def);
    uint32_t raw_agcr = params::readParam("dm_ragc", agcr_def);
    float dcb_rate = std::bit_cast<float>(raw_dcbr);
    float agc_rate = std::bit_cast<float>(raw_agcr);
    dsp_nrxdcb->setRate(dcb_rate);
    dsp_nrxagc->setRate(agc_rate);
    dsp_n_bfskmod->setFs(params::readParam("dm_tisr", 10000));
    dsp_n_bfskdemod->setFs(params::readParam("dm_rnsr", 2000));
    dsp_n_mfskmod->setFs(params::readParam("dm_tisr", 10000));
    dsp_n_mfskdemod->setFs(params::readParam("dm_rnsr", 2000));
    dsp_n_mskmod->setFs(params::readParam("dm_tisr", 10000));
    dsp_n_mskdemod->setFs(params::readParam("dm_rnsr", 2000));
    maini2c->set_speed(params::readParam("dm_i2cl", 400));
    mainsi5351->set_addr(params::readParam("dm_siaddr", 0x60));
    mainsi5351->set_xtal_freq(params::readParam("dm_sixf", 25000000));
    dsp_mainadcsrc->set_bitwidth(params::readParam("dm_adcbw", 12));
    dsp_maincombsink->setCompens(params::readParam("dm_tca", 6), params::readParam("dm_tcb", 0));
    dsp_maincombsink->setSr(params::readParam("dm_tisr", 10000), params::readParam("dm_tdsr", 100000) / params::readParam("dm_tisr", 10000));
    dsp_maincombsink->setTaps(params::readParam("dm_toft", 33));
    unlock_dsp_mtx();
}

void dsp_mgr::dspcore_main(void *arg) {
    printf("DSP core: %d\n", xPortGetCoreID());
    //Move interrupts to the DSP core for speed
    maini2c = new chl_i2c(0, params::readParam("dm_i2cl", 400), MAINI2C_SDA_GPIO, MAINI2C_SCL_GPIO);
    maini2s = new chl_i2sanalog();
    mainsi5351 = new chl_ext_si5351(params::readParam("dm_siaddr", 0x60), maini2c, params::readParam("dm_sixf", 25000000));
    dsp_mainadcsrc = new cdsp_src_adc(maini2s, MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH, params::readParam("dm_adcbw", 12));
    dsp_maincombsink = new cdsp_sink_combined(maini2s, mainsi5351, params::readParam("dm_tca", 6), params::readParam("dm_tcb", 0), MAINI2S_DAC_CH, false, params::readParam("dm_tisr", 10000), params::readParam("dm_tdsr", 100000) / params::readParam("dm_tisr", 10000), params::readParam("dm_toft", 33));
    while (true) {
        uint32_t sdr_rx_spls = params::readParam("dm_sdrrxspls", 16);
        uint32_t rx_spls = params::readParam("dm_rxspls", 16);
        uint32_t tx_spls = params::readParam("dm_txspls", 128);
        while (true) {
            lock_dsp_mtx();
            if (curr_mode == DSPMGR_MODE_IDLE) {
                unlock_dsp_mtx();
                taskYIELD();
                break;
            } else {
                switch (curr_mode) {
                    case DSPMGR_MODE_SDR_RX: {
                        int r = dsp_rxindecim->requestData(dsp_rxindecim, sdr_rx_data, sdr_rx_spls);
//                    	int r = dsp_nrxagc->requestData(dsp_nrxagc, sdr_rx_data, sdr_rx_spls);
                        if (r > 0) {
                            for (int i = 0; i < r; i++) {
                                (curr_sdr_rx_buff ? sdr_rx_buff_a : sdr_rx_buff_b)[i] = sdr_rx_data[i];
                            }
                            if (sdr_rx_cb != NULL) {
                                sdr_rx_cb(sdr_rx_cb_ctx, (
                                        curr_sdr_rx_buff ? sdr_rx_buff_a : sdr_rx_buff_b), r);
                            }
                            curr_sdr_rx_buff = !curr_sdr_rx_buff;
                        } else if (r < 0) {
                            printf("RX ERR %d!\n", r);
                            sdr_rx_stop(true);
                        } else {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    case DSPMGR_MODE_TX_CARRIER:
                    case DSPMGR_MODE_SDR_TX: {
                        int r = dsp_maincombsink->work(dsp_maincombsink, tx_spls);
                        if (r < 0) {
                            printf("TX ERR %d!\n", r);
                            sdr_tx_stop(true);
                        } else if (r == 0) {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    case DSPMGR_MODE_BFSK_TX: {
                        int r = dsp_maincombsink->work(dsp_maincombsink, tx_spls);
                        if (r < 0) {
                            if (r == -10) {
                                //normal tx completion
                                if (n_tx_cb != NULL) {
                                    n_tx_cb(n_tx_cb_ctx);
                                }
                            } else {
                                printf("TX ERR %d!\n", r);
                            }
                            n_bfsk_tx_stop(true);
                        } else if (r == 0) {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    case DSPMGR_MODE_BFSK_RX: {
                        int r = dsp_n_bfskdemod->requestData(dsp_n_bfskdemod, n_rx_data, rx_spls);
                        if (r > 0) {
                            if(afc_search_mode) {
                                afc_ctr++;
                                if(afc_ctr >= 32/rx_spls) {
                                    afc_ctr = 0;
                                    apply_afc();
                                }
                            }
                            if (n_rx_cb != NULL) {
                                n_rx_cb(n_rx_cb_ctx, n_rx_data, r);
                            }
                        } else if (r < 0) {
                            printf("RX ERR %d!\n", r);
                            n_bfsk_rx_stop(true);
                        } else {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    case DSPMGR_MODE_MFSK_TX: {
                        int r = dsp_maincombsink->work(dsp_maincombsink, tx_spls);
                        if (r < 0) {
                            if (r == -10) {
                                //normal tx completion
                                if (n_tx_cb != NULL) {
                                    n_tx_cb(n_tx_cb_ctx);
                                }
                            } else {
                                printf("TX ERR %d!\n", r);
                            }
                            n_mfsk_tx_stop(true);
                        } else if (r == 0) {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    case DSPMGR_MODE_MFSK_RX: {
                        int r = dsp_n_mfskdemod->requestData(dsp_n_mfskdemod, n_rx_data, rx_spls);
                        if (r > 0) {
                            if(afc_search_mode) {
                                afc_ctr++;
                                if(afc_ctr >= 32/rx_spls) {
                                    afc_ctr = 0;
                                    apply_afc();
                                }
                            }
                            if (n_rx_cb != NULL) {
                                n_rx_cb(n_rx_cb_ctx, n_rx_data, r);
                            }
                        } else if (r < 0) {
                            printf("RX ERR %d!\n", r);
                            n_mfsk_rx_stop(true);
                        } else {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    case DSPMGR_MODE_MSK_TX: {
                        int r = dsp_maincombsink->work(dsp_maincombsink, tx_spls);
                        if (r < 0) {
                            if (r == -10) {
                                //normal tx completion
                                if (n_tx_cb != NULL) {
                                    n_tx_cb(n_tx_cb_ctx);
                                }
                            } else {
                                printf("TX ERR %d!\n", r);
                            }
                            n_msk_tx_stop(true);
                        } else if (r == 0) {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    case DSPMGR_MODE_MSK_RX: {
                        int r = dsp_n_mskdemod->requestData(dsp_n_mskdemod, n_rx_data, rx_spls);
                        if (r > 0) {
                            if(afc_search_mode) {
                                afc_ctr++;
                                if(afc_ctr >= 32/rx_spls) {
                                    afc_ctr = 0;
                                    apply_afc();
                                }
                            }
                            if (n_rx_cb != NULL) {
                                n_rx_cb(n_rx_cb_ctx, n_rx_data, r);
                            }
                        } else if (r < 0) {
                            printf("RX ERR %d!\n", r);
                            n_msk_rx_stop(true);
                        } else {
                            printf("WARN: 0 DATA!\n");
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            unlock_dsp_mtx();
            taskYIELD();
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

int dsp_mgr::sdr_tx_reqfunc(void *ctx, cdsp_complex_t *data, int samples_cnt) {
    for (int i = 0; i < samples_cnt; i++) {
        data[i] = sdr_tx_buff[sdr_tx_buff_rptr];
        if (sdr_tx_buff_rptr != sdr_tx_buff_wptr) {
            sdr_tx_buff_rptr = (sdr_tx_buff_rptr + 1) % SDR_TX_BUFF_SIZE;
        } else {
            printf("BU\n");
        }
    }
    if (sdr_tx_waiting_task_hdl != NULL) {
        xTaskNotifyGive(sdr_tx_waiting_task_hdl);
    }
    return samples_cnt;
}

int dsp_mgr::sdr_tx_carrier_reqfunc(void *ctx, cdsp_complex_t *data, int samples_cnt) {
    for (int i = 0; i < samples_cnt; i++) {
        data[i].i = 1.0f;
        data[i].q = 0.0f;
    }
    return samples_cnt;
}

void dsp_mgr::reset() {
    lock_dsp_mtx();
    pin_mgr::set_txp_enable(false);
    curr_mode = DSPMGR_MODE_IDLE;
    dsp_maincombsink->stop(true);
    dsp_rxindecim->stop(true);
    dsp_n_bfskdemod->stop(true);
    dsp_n_mfskdemod->stop(true);
    dsp_n_mskdemod->stop(true);
    pin_mgr::set_txled_enable(false);
    pin_mgr::set_rxled_enable(false);
    mainsi5351->set_output_enabled(false, false);
    rx_paused = false;
    afc_shift = 0;
    afc_ctr = 0;
    afc_search_mode = true;
    unlock_dsp_mtx();
}

void dsp_mgr::notifyDspTask() {
    xTaskNotifyGive(dspcore_task_hdl);
}

void dsp_mgr::lock_dsp_mtx() {
    if (xSemaphoreTake(dsp_mtx, portMAX_DELAY) != pdTRUE) {
        printf("DSP MTX LOCK FAIL!\r\n");
    }
}

void dsp_mgr::unlock_dsp_mtx() {
    xSemaphoreGive(dsp_mtx);
}

void dsp_mgr::set_fr(float newfr, bool reset_afc) {
    if(reset_afc) {
        afc_shift = 0;
    }
    mainsi5351->set_frequency(false, roundf(newfr));
    mainsi5351->set_pll_frequency_shift(false, afc_shift * params::readParam("dm_rnsr", 2000));
}

void dsp_mgr::apply_afc() {
    float relfrshift = 0;
    if(curr_mode == DSPMGR_MODE_BFSK_RX) {
        relfrshift = -dsp_n_bfskdemod->_avgerr;
    } else if(curr_mode == DSPMGR_MODE_MFSK_RX) {
//        printf("FRE %f\n", dsp_n_mfskdemod->_avgerr);
        relfrshift = -dsp_n_mfskdemod->_avgerr;
    } else if(curr_mode == DSPMGR_MODE_MSK_RX) {
//        printf("FRE %f\n", dsp_n_mskdemod->_avgerr);
//        relfrshift = -dsp_n_mskdemod->_avgerr; //AFC is broken for MSK
    }
    if(relfrshift != 0) {
        afc_shift += relfrshift * AFC_RATE;
        int fr_shift_hz = afc_shift * params::readParam("dm_rnsr", 2000);
        if(fabsf(fr_shift_hz) >= fabsf(MAX_AFC_SHIFT)) {
            printf("FR SHIFT TOO LARGE: %d, RESETTING\n", fr_shift_hz);
            afc_shift = 0;
            fr_shift_hz = 0;
        }
        mainsi5351->set_pll_frequency_shift(false, afc_shift * params::readParam("dm_rnsr", 2000));
    }
}

void dsp_mgr::rx_set_ins(bool ins) {
    lock_dsp_mtx();
    if (!ins) {
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_LOW_CH, MAINI2S_Q_LOW_CH);
    } else {
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
    }
    unlock_dsp_mtx();
}

void dsp_mgr::general_rx_start() {
    mainsi5351->set_output_enabled(false, true);
    maini2s->setSampleRate(params::readParam("dm_risr", 100000));
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
    maini2c->set_speed(params::readParam("dm_i2ch", 900));
    maini2s->setSampleRate(params::readParam("dm_tdsr", 100000));
    dsp_maincombsink->start(true);
    pin_mgr::set_txled_enable(true);
    notifyDspTask();
}

void dsp_mgr::general_tx_stop() {
    pin_mgr::set_txp_enable(false);
    dsp_maincombsink->stop(true);
    // maini2c->force_gpio(false);
    maini2c->set_speed(params::readParam("dm_i2cl", 400));
    mainsi5351->set_output_enabled(false, false);
    set_fr(mainsi5351->get_curr_center_freq(), false);
    pin_mgr::set_txled_enable(false);
}

void dsp_mgr::sdr_rx_start() {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        dsp_rxindecim->setDecimation(roundf(params::readParam("dm_risr", 100000) / sdr_rx_sr));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, params::readParam("dm_rift", 33), params::readParam("dm_risr", 100000), sdr_rx_sr / 2.0f, true);
        //Make the chain
//        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
//		dsp_nrxdcb->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        //Start last block
        general_rx_start();
        dsp_rxindecim->start(true);
//        dsp_nrxagc->start(true);
        curr_mode = DSPMGR_MODE_SDR_RX;
        notifyDspTask();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::sdr_rx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_SDR_RX) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        curr_mode = DSPMGR_MODE_IDLE;
        dsp_rxindecim->stop(true);
//        dsp_nrxagc->stop(true);
        general_rx_stop();
        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::sdr_rx_set_sr(float newsr) {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        sdr_rx_sr = newsr;
        unlock_dsp_mtx();
    } 
}

void dsp_mgr::sdr_rx_set_cb(void (*new_sdr_rx_cb)(void*, cdsp_complex_t*, int), void *ctx) {
    lock_dsp_mtx();
    sdr_rx_cb = new_sdr_rx_cb;
    sdr_rx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::sdr_tx_start() {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        //Make the chain
        dsp_maincombsink->setInputFunc(NULL, sdr_tx_reqfunc);
        //Start last block
        curr_mode = DSPMGR_MODE_SDR_TX;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::sdr_tx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_SDR_TX || curr_mode == DSPMGR_MODE_TX_CARRIER) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        curr_mode = DSPMGR_MODE_IDLE;
        general_tx_stop();
        if (sdr_tx_waiting_task_hdl != NULL) {
            xTaskNotifyGive(sdr_tx_waiting_task_hdl);
        }
        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::tx_carrier() {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        //Make the chain
        dsp_maincombsink->setInputFunc(NULL, sdr_tx_carrier_reqfunc);
        //Start last block
        curr_mode = DSPMGR_MODE_TX_CARRIER;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

int dsp_mgr::sdr_tx_put_data_sync(cdsp_complex_t *data, int cnt) {
    for (int i = 0; i < cnt; i++) {
        while ((sdr_tx_buff_wptr + 1) % SDR_TX_BUFF_SIZE != sdr_tx_buff_rptr) {
            if (curr_mode != DSPMGR_MODE_SDR_TX || sdr_tx_waiting_task_hdl != NULL) {
                printf("BO\n");
                return i;
            }
            sdr_tx_waiting_task_hdl = xTaskGetCurrentTaskHandle();
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            sdr_tx_waiting_task_hdl = NULL;
        }
        sdr_tx_buff[sdr_tx_buff_wptr] = data[i];
        sdr_tx_buff_wptr = (sdr_tx_buff_wptr + 1) % SDR_TX_BUFF_SIZE;
    }
    return cnt;
}

void dsp_mgr::n_tx_set_cb(void (*new_n_tx_cb)(void*), void *ctx) {
    lock_dsp_mtx();
    n_tx_cb = new_n_tx_cb;
    n_tx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::n_tx_set_reqfunc(int (*new_n_tx_reqfunc)(void*, uint8_t*, int), void *ctx) {
    lock_dsp_mtx();
    n_tx_reqfunc = new_n_tx_reqfunc;
    n_tx_reqfunc_ctx = ctx;
    dsp_n_bfskmod->setInputFunc(n_tx_reqfunc_ctx, n_tx_reqfunc);
    dsp_n_mfskmod->setInputFunc(n_tx_reqfunc_ctx, n_tx_reqfunc);
    dsp_n_mskmod->setInputFunc(n_tx_reqfunc_ctx, n_tx_reqfunc);
    unlock_dsp_mtx();
}

void dsp_mgr::n_rx_set_cb(void (*new_n_rx_cb)(void*, uint8_t*, int), void *ctx) {
    lock_dsp_mtx();
    n_rx_cb = new_n_rx_cb;
    n_rx_cb_ctx = ctx;
    unlock_dsp_mtx();
}

void dsp_mgr::n_bfsk_set_params(float fr0, float fr1, float speed) {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        dsp_n_bfskmod->setFrs(fr0, fr1);
        dsp_n_bfskmod->setDataRate(speed);
        dsp_n_bfskdemod->setFrs(fr0, fr1);
        dsp_n_bfskdemod->setDataRate(speed);
        uint32_t loopbw_def = std::bit_cast < uint32_t > (0.0005f);
        uint32_t raw_loopbw = params::readParam("dm_loopbw", loopbw_def);
        float loopbw = std::bit_cast<float>(raw_loopbw);
        dsp_n_bfskdemod->setLoopBw((params::readParam("dm_rnsr", 2000) / speed) * loopbw);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_bfsk_tx_start() {
    if (curr_mode == DSPMGR_MODE_BFSK_RX) {
        n_bfsk_rx_stop(false);
        rx_paused = true;
    }
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        if (n_tx_reqfunc != NULL) {
            //Configure blocks, calculate taps
            //Make the chain
            dsp_maincombsink->setInputBlk(dsp_n_bfskmod, dsp_n_bfskmod->requestData);
            //Start last block
            curr_mode = DSPMGR_MODE_BFSK_TX;
            general_tx_start();
        } else {
            printf("ERROR: TX REQFUNC NULL!\n");
        }
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_bfsk_tx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_BFSK_TX) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        curr_mode = DSPMGR_MODE_IDLE;
        general_tx_stop();
        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
    if(rx_paused) {
        n_bfsk_rx_start();
        rx_paused = false;
    }
}

void dsp_mgr::n_bfsk_rx_start() {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        if(!rx_paused)
            lock_dsp_mtx();
        //Configure blocks, calculate taps
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
        dsp_rxindecim->setDecimation(roundf(params::readParam("dm_risr", 100000) / params::readParam("dm_rnsr", 2000)));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, params::readParam("dm_rift", 33), params::readParam("dm_risr", 100000), params::readParam("dm_rnsr", 2000) / 2.0f, true);
        //Make the chain
        dsp_n_bfskdemod->setInputBlk(dsp_nrxagc, dsp_nrxagc->requestData);
        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
        dsp_nrxdcb->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        //Start last block
        general_rx_start();
        dsp_n_bfskdemod->start(true);
        curr_mode = DSPMGR_MODE_BFSK_RX;
        if(!rx_paused) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_bfsk_rx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_BFSK_RX) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        curr_mode = DSPMGR_MODE_IDLE;
        dsp_n_bfskdemod->stop(true);
        general_rx_stop();
        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_mfsk_set_params(int frcnt, float *frs, float speed) {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        dsp_n_mfskmod->setFrs(frcnt, frs);
        dsp_n_mfskmod->setDataRate(speed);
        dsp_n_mfskdemod->setFrs(frcnt, frs);
        dsp_n_mfskdemod->setDataRate(speed);
        uint32_t loopbw_def = std::bit_cast < uint32_t > (0.0005f);
        uint32_t raw_loopbw = params::readParam("dm_loopbw", loopbw_def);
        float loopbw = std::bit_cast<float>(raw_loopbw);
        dsp_n_mfskdemod->setLoopBw((params::readParam("dm_rnsr", 2000) / speed) * loopbw);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_mfsk_tx_start() {
    if (curr_mode == DSPMGR_MODE_MFSK_RX) {
        n_mfsk_rx_stop(false);
        rx_paused = true;
    }
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        //Make the chain
        dsp_maincombsink->setInputBlk(dsp_n_mfskmod, dsp_n_mfskmod->requestData);
        //Start last block
        curr_mode = DSPMGR_MODE_MFSK_TX;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_mfsk_tx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_MFSK_TX) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        curr_mode = DSPMGR_MODE_IDLE;
        general_tx_stop();

        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
    if(rx_paused) {
        n_mfsk_rx_start();
        rx_paused = false;
    }
}

void dsp_mgr::n_mfsk_rx_start() {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        if(!rx_paused)
            lock_dsp_mtx();
        //Configure blocks, calculate taps
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
        dsp_rxindecim->setDecimation(roundf(params::readParam("dm_risr", 100000) / params::readParam("dm_rnsr", 2000)));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, params::readParam("dm_rift", 33), params::readParam("dm_risr", 100000), params::readParam("dm_rnsr", 2000) / 2.0f, true);
        //Make the chain
        dsp_n_mfskdemod->setInputBlk(dsp_nrxagc, dsp_nrxagc->requestData);
        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
        dsp_nrxdcb->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        //Start last block
        general_rx_start();
        dsp_n_mfskdemod->start(true);
        curr_mode = DSPMGR_MODE_MFSK_RX;
        if(!rx_paused) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_mfsk_rx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_MFSK_RX) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        curr_mode = DSPMGR_MODE_IDLE;
        dsp_n_mfskdemod->stop(true);
        general_rx_stop();
        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_msk_set_params(float speed) {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        dsp_n_mskmod->setDataRate(speed);
        dsp_n_mskdemod->setDataRate(speed);
        uint32_t loopbw_def = std::bit_cast < uint32_t > (0.0005f);
        uint32_t raw_loopbw = params::readParam("dm_loopbw", loopbw_def);
        float loopbw = std::bit_cast<float>(raw_loopbw) / 2.5f;
        dsp_n_mskdemod->setLoopBw((params::readParam("dm_rnsr", 2000) / speed) * loopbw);
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_msk_tx_start() {
    if (curr_mode == DSPMGR_MODE_MSK_RX) {
        n_msk_rx_stop(false);
        rx_paused = true;
    }
    if (curr_mode == DSPMGR_MODE_IDLE) {
        lock_dsp_mtx();
        //Configure blocks, calculate taps
        //Make the chain
        dsp_maincombsink->setInputBlk(dsp_n_mskmod, dsp_n_mskmod->requestData);
        curr_mode = DSPMGR_MODE_MSK_TX;
        general_tx_start();
        unlock_dsp_mtx();
    }
}

void dsp_mgr::n_msk_tx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_MSK_TX) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        curr_mode = DSPMGR_MODE_IDLE;
        general_tx_stop();
        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
    if(rx_paused) {
        n_msk_rx_start();
        rx_paused = false;
    }
}

void dsp_mgr::n_msk_rx_start() {
    if (curr_mode == DSPMGR_MODE_IDLE) {
        if(!rx_paused)
            lock_dsp_mtx();
        //Configure blocks, calculate taps
        set_fr(mainsi5351->get_curr_center_freq() - (dsp_n_mskmod->getDataRate() + 100.0f), false); //required because analog receiver circuit works awfully on F<~100 Hz
        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
        dsp_rxindecim->setDecimation(roundf(params::readParam("dm_risr", 100000) / params::readParam("dm_rnsr", 2000)));
        cdsp_calc_taps_lpf_float(dsp_rxinfir_taps, params::readParam("dm_rift", 33), params::readParam("dm_risr", 100000), params::readParam("dm_rnsr", 2000) / 2.0f, true);
        //Make the chain
        dsp_n_mskdemod->setInputBlk(dsp_nrxagc, dsp_nrxagc->requestData);
        dsp_nrxagc->setInputBlk(dsp_nrxdcb, dsp_nrxdcb->requestData);
        dsp_nrxdcb->setInputBlk(dsp_rxindecim, dsp_rxindecim->requestData);
        //Start last block
        general_rx_start();
        dsp_n_mskdemod->start(true);
        curr_mode = DSPMGR_MODE_MSK_RX;
        if(!rx_paused) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

void dsp_mgr::n_msk_rx_stop(bool from_dspc) {
    if (curr_mode == DSPMGR_MODE_MSK_RX) {
        if (!from_dspc) {
            lock_dsp_mtx();
        }
        set_fr(mainsi5351->get_curr_center_freq() + (dsp_n_mskmod->getDataRate() + 100.0f), false); //Return frequency back
        curr_mode = DSPMGR_MODE_IDLE;
        dsp_n_mskdemod->stop(true);
        general_rx_stop();
        if (!from_dspc) {
            notifyDspTask();
            unlock_dsp_mtx();
        }
    }
}

#endif
