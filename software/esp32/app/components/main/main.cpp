#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

#include "libs/pin_mgr.h"
#include "libs/pc_interface.h"

#include "esp_random.h"
#include "esp_timer.h"
#include "esp_task.h"

#define UART_BAUDRATE 921600
#define I2C_SPD_KHZ 800
#define SI5351_I2C_ADDR 0x60
#define SI5351_XTAL_FREQ 25000000
#define ADC_BITWIDTH 12
#define SDR_RX_FIR_TAPS 33
#define SDR_RX_IN_SR 100000
#define SDR_RX_PACKET_SPLS 16
#define SDR_TX_IN_SR 10000
#define SDR_TX_DAC_SR 100000

extern "C" void app_main();

TaskHandle_t dspcore_task_hdl = NULL;
chl_uart_dma* mainuart = NULL;
chl_i2c* maini2c = NULL;
chl_i2sanalog* maini2s = NULL;
chl_ext_si5351* mainsi5351 = NULL;

cdsp_src_adc* dsp_mainadcsrc;
cdsp_sink_combined* dsp_maincombsink;
float* dsp_sdrrxfir_taps;
cdsp_fir<float, cdsp_complex_t>* dsp_sdrrxfir;
cdsp_rational_decimator<cdsp_complex_t>* dsp_sdrrxdecim;


DMA_ATTR uint8_t packet_send_buff[MAX_PC_P_SIZE];
DMA_ATTR uint8_t packet_read_buff[MAX_PC_P_SIZE];
int packet_read_buff_data = 0;
int packet_read_buff_pos = 0;
uint8_t packet_read_data[MAX_PC_P_SIZE];
cdsp_complex_t sdr_rx_data[SDR_RX_PACKET_SPLS];
int16_t sdr_rx_send_data_a[SDR_RX_PACKET_SPLS*2];
int16_t sdr_rx_send_data_b[SDR_RX_PACKET_SPLS*2];
bool curr_sdr_rx_send_data = false;

uint8_t curr_mode = pc_packet_interface::modes::PC_PI_MODE_UNINITED;

bool sdr_receiving = false;
bool sdr_transmitting = false;
float sdr_rx_sr = 1000;

float test_ph = 0;
float fr = 200.0f;
int s = 0;
int IRAM_ATTR TEST_TX_GEN(void* ctx, cdsp_complex_t* data, int cnt) {
    for(int i = 0; i < cnt; i++) {
        test_ph += (2.0f*FL_PI*fr/float(SDR_TX_IN_SR));
        if(test_ph >= 2.0f*FL_PI) {
            test_ph -= 2.0f*FL_PI;
        } else if(test_ph <= -2.0f*FL_PI) {
            test_ph += 2.0f*FL_PI;
        }
        s++;
        if(s >= (0.0005f * SDR_TX_IN_SR)) {
            fr += 50.0f;
            if(fr >= 4500.0f) {
                fr = -4500.0f;
            }
            s = 0;
        }
        data[i].i = cosf(test_ph);
        data[i].q = sinf(test_ph);
    }
    return cnt;
}

void IRAM_ATTR uart_sync_packet_send(pc_packet_interface::pc_packet p) {
    mainuart->waitForTxFinish();
    int bufs = 4+p.len;
    packet_send_buff[0] = pc_packet_interface::startByte;
    packet_send_buff[1] = p.type;
    packet_send_buff[2] = p.len;
    if(p.len > 0) {
        for(int i = 0; i < p.len; i++) {
            packet_send_buff[3+i] = p.data[i];
        }
    }
    packet_send_buff[3+p.len] = 0; //TODO: checksum
    mainuart->transmit_bytes(packet_send_buff, bufs, true);
}

//packet buff should be at least MAX_PC_P_SIZE; blocks until packet is received; No thread safety!!!
void IRAM_ATTR uart_sync_packet_read(pc_packet_interface::pc_packet* p) {
    int state = 0; //0-receiving startByte; 1-receiving type; 2-receiving len; 3-receiving data; 4-receiving checksum
    int outdata_pos = 0;

    while(1) {
        while(packet_read_buff_data > 0) {
            if(state == 0) {
                if(packet_read_buff[packet_read_buff_pos] == pc_packet_interface::startByte) {
                    state = 1;
                }
            } else if(state == 1) {
                p->type = packet_read_buff[packet_read_buff_pos];
                state = 2;
            } else if(state == 2) {
                p->len = packet_read_buff[packet_read_buff_pos];
                if(p->len == 0) {
                    state = 4;
                } else {
                    state = 3;
                }
            } else if(state == 3) {
                p->data[outdata_pos] = packet_read_buff[packet_read_buff_pos];
                outdata_pos++;
                if(outdata_pos >= p->len) {
                    state = 4;
                }
            } else if(state == 4) {
                p->checksum = packet_read_buff[packet_read_buff_pos];
                //TODO: checksum check
                packet_read_buff_pos++;
                packet_read_buff_data--;
                return; //success
            }
            packet_read_buff_pos++;
            packet_read_buff_data--;
        }
        int r = mainuart->read_bytes(packet_read_buff, 4, true);
        if(r < 1) {
            return; //error
        }
        packet_read_buff_data = r;
        packet_read_buff_pos = 0;
    }
}

void IRAM_ATTR dspcore_main(void* arg) {
    printf("DSP core: %d\n", xPortGetCoreID());
    //Move interrupts to the DSP core for speed
    maini2c = new chl_i2c(0, I2C_SPD_KHZ, MAINI2C_SDA_GPIO, MAINI2C_SCL_GPIO);
    maini2s = new chl_i2sanalog();
    while(true) {
        if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
            while(sdr_receiving) {
                int r = dsp_sdrrxdecim->requestData(dsp_sdrrxdecim, sdr_rx_data, SDR_RX_PACKET_SPLS);
                if(r > 0) {
                    for(int i = 0; i < SDR_RX_PACKET_SPLS; i++) {
                        (curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b)[i*2] = roundf(sdr_rx_data[i].i * 32767.0f);
                        (curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b)[i*2 + 1] = roundf(sdr_rx_data[i].q * 32767.0f);
                    }
                    pc_packet_interface::pc_packet data_p;
                    data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_SDR_RX_DATA;
                    data_p.len = SDR_RX_PACKET_SPLS * sizeof(int16_t) * 2;
                    data_p.data = (uint8_t*)(curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b);
                    uart_sync_packet_send(data_p);
                    curr_sdr_rx_send_data = !curr_sdr_rx_send_data;
                } else {
                    printf("RX ERR %d!\n", r);
                    mainsi5351->set_output_enabled(false, false);
                    dsp_sdrrxdecim->stop(true);
                    pin_mgr::set_rxled_enable(false);
                    sdr_receiving = false;
                }
            }
            while(sdr_transmitting) {
                // int r = dsp_maindacsnk->work(dsp_maindacsnk, SDR_TX_DAC_SPLS);
                int r = dsp_maincombsink->work(dsp_maincombsink, 128);
                if(r > 0) {
                    
                } else {
                    if(r != -3) { //Normal timeout
                        printf("TX ERR %d!\n", r);
                    }
                    sdr_transmitting = false;
                    dsp_maincombsink->stop(true);
                    mainsi5351->set_output_enabled(false, false);
                    pin_mgr::set_txled_enable(false);
                }
            }
        } else {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }
}

void IRAM_ATTR app_main(void) {
    printf("Main core: %d\n", xPortGetCoreID());
    xTaskCreatePinnedToCore(dspcore_main, "DSPMAINTSK", 3584, NULL, ESP_TASK_TIMER_PRIO, &dspcore_task_hdl, 1);
    while(maini2c == NULL || maini2s == NULL) {vTaskDelay(1 / portTICK_PERIOD_MS);} //wait for the second core to init interrupts
    mainuart = new chl_uart_dma(0, UART_BAUDRATE, MAINUART_RXD_GPIO, MAINUART_TXD_GPIO);
    mainsi5351 = new chl_ext_si5351(SI5351_I2C_ADDR, maini2c, SI5351_XTAL_FREQ);
    pin_mgr::init();
    pin_mgr::set_txled_enable(true);

    dsp_mainadcsrc = new cdsp_src_adc(maini2s, MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH, ADC_BITWIDTH);
    dsp_maincombsink = new cdsp_sink_combined(maini2s, mainsi5351, MAINI2S_DAC_CH, false, 0, SDR_TX_IN_SR, SDR_TX_DAC_SR/SDR_TX_IN_SR, 33);
    dsp_sdrrxfir_taps = new float[SDR_RX_FIR_TAPS];
    dsp_sdrrxfir = new cdsp_fir<float, cdsp_complex_t>(SDR_RX_FIR_TAPS, dsp_sdrrxfir_taps);
    dsp_sdrrxdecim = new cdsp_rational_decimator<cdsp_complex_t>(SDR_RX_IN_SR/sdr_rx_sr);

    pc_packet_interface::pc_packet startp;
    startp.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_START;
    startp.len = 0;
    uart_sync_packet_send(startp);

    pc_packet_interface::pc_packet p;
    p.data = packet_read_data;
    uint8_t ackbyte = 0;
    pc_packet_interface::pc_packet ackp;
    ackp.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_ACK;
    ackp.len = 1;
    ackp.data = &ackbyte;
    while(true) {
        uart_sync_packet_read(&p);
        ackbyte = 0;
        switch(p.type) {
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_CHANGE_MODE: {
                curr_mode = p.data[0];
                if(curr_mode != pc_packet_interface::modes::PC_PI_MODE_UNINITED) {
                    pin_mgr::set_usbled_enable(true);
                } else {
                    pin_mgr::set_usbled_enable(false);
                }
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    
                }
                sdr_receiving = false;
                sdr_transmitting = false;
                dsp_maincombsink->stop(true);
                pin_mgr::set_txled_enable(false);
                pin_mgr::set_dcd_enable(false);
                pin_mgr::set_txled_enable(false);
                pin_mgr::set_rxled_enable(false);
                mainsi5351->set_output_enabled(false, false);
                ackbyte = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_START: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR && !sdr_transmitting) {
                    if(!sdr_receiving) {
                        mainsi5351->set_output_enabled(false, true);
                        int realsr = maini2s->setSampleRate(SDR_RX_IN_SR);
                        dsp_sdrrxdecim->setDecimation(realsr/sdr_rx_sr);
                        cdsp_calc_taps_lpf_float(dsp_sdrrxfir_taps, SDR_RX_FIR_TAPS, SDR_RX_IN_SR, sdr_rx_sr/2.0f, true);
                        dsp_sdrrxdecim->setInputBlk(dsp_sdrrxfir, dsp_sdrrxfir->requestData);
                        dsp_sdrrxfir->setInputBlk(dsp_mainadcsrc, dsp_mainadcsrc->requestData);
                        dsp_sdrrxdecim->start(true);
                        sdr_receiving = true;
                        pin_mgr::set_rxled_enable(true);
                        ackbyte = 1;
                    }
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_STOP: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    if(sdr_receiving) {
                        sdr_receiving = false;
                        mainsi5351->set_output_enabled(false, false);
                        dsp_sdrrxdecim->stop(true);
                        pin_mgr::set_rxled_enable(false);
                        ackbyte = 1;
                    }
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_SET_SR: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    if(!sdr_receiving) {
                        sdr_rx_sr = *((float*)(p.data));
                        ackbyte = 1;
                    }
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_SET_FR: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    float* newfr = (float*)p.data;
                    mainsi5351->set_frequency(false, roundf(*newfr));
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_SET_IN_SRC: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    if(p.data[0] == 0) {
                        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_LOW_CH, MAINI2S_Q_LOW_CH);
                    } else {
                        dsp_mainadcsrc->setAdcChannels(MAINI2S_I_HIGH_CH, MAINI2S_Q_HIGH_CH);
                    }
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_TX_START: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR && !sdr_receiving) {
                    if(!sdr_transmitting) {
                        mainsi5351->set_output_enabled(true, true);
                        int realsr = maini2s->setSampleRate(SDR_TX_DAC_SR);
                        dsp_maincombsink->setInputFunc(NULL, TEST_TX_GEN);
                        dsp_maincombsink->start(true);
                        sdr_transmitting = true;
                        pin_mgr::set_txled_enable(true);
                        ackbyte = 1;
                    }
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_TX_STOP: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    if(sdr_transmitting) {
                        sdr_transmitting = false;
                        dsp_maincombsink->stop(true);
                        mainsi5351->set_output_enabled(false, false);
                        pin_mgr::set_txled_enable(false);
                        ackbyte = 1;
                    }
                }
                break;
            }
        }
        uart_sync_packet_send(ackp);
        xTaskNotifyGive(dspcore_task_hdl);
    }
}
