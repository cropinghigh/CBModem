#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "chl_uart.h"

#include "libs/pin_mgr.h"
#include "libs/dsp_mgr.h"
#include "libs/packet_mgr.h"
#include "libs/pc_interface.h"

#define UART_BAUDRATE 921600

extern "C" void app_main();

chl_uart_dma* mainuart = NULL;

DMA_ATTR uint8_t packet_send_buff[MAX_PC_P_SIZE];
DMA_ATTR uint8_t packet_read_buff[MAX_PC_P_SIZE];
int packet_read_buff_data = 0;
int packet_read_buff_pos = 0;
uint8_t packet_read_data[MAX_PC_P_SIZE];
int16_t sdr_rx_send_data_a[128];
int16_t sdr_rx_send_data_b[128];
bool curr_sdr_rx_send_data = false;
cdsp_complex_t sdr_tx_recv_data[128];

uint8_t n_bfsk_rx_data[256];

uint8_t curr_mode = pc_packet_interface::modes::PC_PI_MODE_UNINITED;

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

void IRAM_ATTR sdr_rx_cb(void* ctx, cdsp_complex_t* data, int cnt) {
    if(cnt > 0) {
        for(int i = 0; i < cnt; i++) {
            (curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b)[i*2] = roundf(data[i].i * 0.1f * 32767.0f);
            (curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b)[i*2 + 1] = roundf(data[i].q * 0.1f * 32767.0f);
        }
        pc_packet_interface::pc_packet data_p;
        data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_SDR_RX_DATA;
        data_p.len = cnt * sizeof(int16_t) * 2;
        data_p.data = (uint8_t*)(curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b);
        uart_sync_packet_send(data_p);
        curr_sdr_rx_send_data = !curr_sdr_rx_send_data;
    }
}

void IRAM_ATTR n_bfsk_tx_cb(void* ctx) {
    pc_packet_interface::pc_packet data_p;
    data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_TRANSMIT_COMPL;
    data_p.len = 0;
    uart_sync_packet_send(data_p);
}

void IRAM_ATTR n_bfsk_rx_cb(void* ctx, uint8_t* data, int cnt) {
    packet_mgr::load_rx_data(data, cnt);
    // for(int i = 0; i < cnt; i++) {
    //     n_bfsk_rx_data[i] = data[i];
    // }
    // pc_packet_interface::pc_packet data_p;
    // data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA;
    // data_p.len = cnt;
    // data_p.data = n_bfsk_rx_data;
    // uart_sync_packet_send(data_p);
}

void IRAM_ATTR app_main(void) {
    mainuart = new chl_uart_dma(0, UART_BAUDRATE, MAINUART_RXD_GPIO, MAINUART_TXD_GPIO);
    printf("Main core: %d\n", xPortGetCoreID());
    pin_mgr::init();
    dsp_mgr::init();
    dsp_mgr::sdr_rx_set_cb(sdr_rx_cb, NULL);
    dsp_mgr::n_bfsk_tx_set_cb(n_bfsk_tx_cb, NULL);
    dsp_mgr::n_bfsk_rx_set_cb(n_bfsk_rx_cb, NULL);
    dsp_mgr::n_mfsk_tx_set_cb(n_bfsk_tx_cb, NULL);
    dsp_mgr::n_mfsk_rx_set_cb(n_bfsk_rx_cb, NULL);
    packet_mgr::init();
    printf("Starting packet interface...\n");

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
                pin_mgr::set_dcd_enable(false);
                dsp_mgr::reset();
                ackbyte = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SET_FR: {
                dsp_mgr::set_fr(*((float*)p.data));
                ackbyte = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_SET_IN_SRC: {
                dsp_mgr::rx_set_ins(p.data[0]);
                ackbyte = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_START: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    dsp_mgr::sdr_rx_start();
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_STOP: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    dsp_mgr::sdr_rx_stop(false);
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_RX_SET_SR: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    dsp_mgr::sdr_rx_set_sr(*((float*)p.data));
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_TX_START: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    dsp_mgr::sdr_tx_start();
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_TX_STOP: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    dsp_mgr::sdr_tx_stop(false);
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_TX_DATA: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    for(int i = 0; i < p.len/4; i++) {
                        sdr_tx_recv_data[i].i = roundf(p.data[i*4] * (1.0f/32767.0f));
                        sdr_tx_recv_data[i].q = roundf(p.data[i*4+2] * (1.0f/32767.0f));
                    }
                    ackbyte = dsp_mgr::sdr_tx_put_data_sync(sdr_tx_recv_data, p.len/4);
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SDR_TX_CARRIER: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_SDR) {
                    dsp_mgr::sdr_tx_carrier();
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_N_BFSK_TRANSMIT: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK) {
                    if(p.len > 0) {
                        packet_mgr::load_tx_data(p.data, p.len);
                        dsp_mgr::n_bfsk_tx_set_params(-200.0f, 200.0f, 200.0f);
                        dsp_mgr::n_bfsk_tx_start(packet_mgr::tx_reqfunc, NULL);
                        ackbyte = 1;
                    }
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_N_BFSK_START_RX: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK) {
                    dsp_mgr::n_bfsk_rx_set_params(-200.0f, 200.0f, 200.0f);
                    dsp_mgr::n_bfsk_rx_start();
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_N_BFSK_STOP_RX: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK) {
                    dsp_mgr::n_bfsk_rx_stop(false);
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_N_MFSK_TRANSMIT: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK) {
                    if(p.len > 0) {
                        int frcnt = 8;
                        float frs[frcnt] = {-500.0f, -375.0f, -250.0f, -125.0f, 125.0f, 250.0f, 375.0f, 500.0f};
                        packet_mgr::load_tx_data(p.data, p.len);
                        dsp_mgr::n_mfsk_tx_set_params(frcnt, frs, 50.0f);
                        dsp_mgr::n_mfsk_tx_start(packet_mgr::tx_reqfunc, NULL);
                        ackbyte = 1;
                    }
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_N_MFSK_START_RX: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK) {
                    int frcnt = 8;
                    float frs[frcnt] = {-500.0f, -375.0f, -250.0f, -125.0f, 125.0f, 250.0f, 375.0f, 500.0f};
                    dsp_mgr::n_mfsk_rx_set_params(frcnt, frs, 50.0f);
                    dsp_mgr::n_mfsk_rx_start();
                    ackbyte = 1;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_N_MFSK_STOP_RX: {
                if(curr_mode == pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK) {
                    dsp_mgr::n_mfsk_rx_stop(false);
                    ackbyte = 1;
                }
                break;
            }
        }
        uart_sync_packet_send(ackp);
    }
}
