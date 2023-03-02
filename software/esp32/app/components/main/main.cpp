#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "chl_uart.h"

#include "cdsp_common.h"
#include "libs/params.h"
#include "libs/pin_mgr.h"
#include "libs/dsp_mgr.h"
#include "libs/packet_mgr.h"
#include "libs/pc_interface.h"
#include "libs/checksum.h"

extern "C" void app_main();

chl_uart_dma *mainuart = NULL;

DMA_ATTR uint8_t packet_send_buff[MAX_PC_P_SIZE];
DMA_ATTR uint8_t packet_read_buff[MAX_PC_P_SIZE];
int packet_read_buff_data = 0;
int packet_read_buff_pos = 0;
int16_t sdr_rx_send_data_a[128];
int16_t sdr_rx_send_data_b[128];
bool curr_sdr_rx_send_data = false;
cdsp_complex_t sdr_tx_recv_data[128];

uint8_t n_bfsk_rx_data[256];

uint8_t curr_mode = pc_packet_interface::modes::PC_PI_MODE_UNINITED;

bool curr_tx_parity = false;
bool curr_rx_parity = false;

TaskHandle_t acksend_task_hdl = NULL;
bool rightparity = false;

void IRAM_ATTR uart_sync_packet_send(pc_packet_interface::pc_packet p) {
    mainuart->waitForTxFinish();
    int bufs = 5 + p.len;
    packet_send_buff[0] = 0x00;
    packet_send_buff[1] = pc_packet_interface::startByte;
    packet_send_buff[2] = p.type;
    packet_send_buff[3] = p.len;
    if (p.len > 0) {
        for (int i = 0; i < p.len; i++) {
            packet_send_buff[4 + i] = p.data[i];
        }
    }
    packet_send_buff[4 + p.len] = calc_crc8(&packet_send_buff[1], p.len+3);
    mainuart->transmit_bytes(packet_send_buff, bufs, true);
}

//blocks until packet is received; No thread safety!!!
void IRAM_ATTR uart_sync_packet_read(pc_packet_interface::pc_packet *p) {
    int state = 0; //0-receiving startByte; 1-receiving type; 2-receiving len; 3-receiving data; 4-receiving checksum
    int outdata_pos = 0;
    uint8_t checksum_buff[sizeof(pc_packet_interface::pc_packet)];
    checksum_buff[0] = pc_packet_interface::startByte;

    while (1) {
        while (packet_read_buff_data > 0) {
//            if(packet_read_buff[packet_read_buff_pos] != pc_packet_interface::startByte) {
//                printf("%c", packet_read_buff[packet_read_buff_pos]);
//                fflush(stdout);
//            }
            if (state == 0) {
                if (packet_read_buff[packet_read_buff_pos] == pc_packet_interface::startByte) {
//                    printf("[");
                    state = 1;
                }
            } else if (state == 1) {
                p->type = packet_read_buff[packet_read_buff_pos];
                checksum_buff[1] = p->type;
                state = 2;
            } else if (state == 2) {
                p->len = packet_read_buff[packet_read_buff_pos];
                checksum_buff[2] = p->len;
                if (p->len == 0) {
                    state = 4;
                } else {
                    state = 3;
                }
            } else if (state == 3) {
                p->data[outdata_pos] = packet_read_buff[packet_read_buff_pos];
                checksum_buff[3+outdata_pos] = p->data[outdata_pos];
                outdata_pos++;
                if (outdata_pos >= p->len) {
                    state = 4;
                }
            } else if (state == 4) {
                p->checksum = packet_read_buff[packet_read_buff_pos];
                uint8_t calc_checksum = calc_crc8(checksum_buff, 3+outdata_pos);
                packet_read_buff_pos++;
                packet_read_buff_data--;
                if(calc_checksum == p->checksum) {
//                    printf("\nDEV PACKET %d %d\n", p->type, p->len);
                    return; //success
                } else {
                    printf("WRONG CRC(%x %x %d %d)!\n", calc_checksum, p->checksum, p->len, 3+outdata_pos);
                    state = 0;
                    outdata_pos = 0;
                }
            }
            packet_read_buff_pos++;
            packet_read_buff_data--;
        }
        int r = mainuart->read_bytes(packet_read_buff, 4, true);
        if (r < 1) {
            return; //error
        }
        packet_read_buff_data = r;
        packet_read_buff_pos = 0;
    }
}

void IRAM_ATTR sdr_rx_cb(void *ctx, cdsp_complex_t *data, int cnt) {
    if (cnt > 0) {
        for (int i = 0; i < cnt; i++) {
            (curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b)[i * 2] = roundf(data[i].i * 0.1f * 32767.0f);
            (curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b)[i * 2 + 1] = roundf(data[i].q * 0.1f * 32767.0f);
        }
        pc_packet_interface::pc_packet data_p;
        data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_SDR_RX_DATA;
        data_p.len = cnt * sizeof(int16_t) * 2;
        for(int i = 0; i < data_p.len; i++) {
            data_p.data[i] = ((uint8_t*) (curr_sdr_rx_send_data ? sdr_rx_send_data_a : sdr_rx_send_data_b))[i];
        }
        uart_sync_packet_send(data_p);
        curr_sdr_rx_send_data = !curr_sdr_rx_send_data;
    }
}

void IRAM_ATTR n_bfsk_tx_cb(void *ctx) {
    pc_packet_interface::pc_packet data_p;
    data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_TRANSMIT_COMPL;
    data_p.len = 0;
    uart_sync_packet_send(data_p);
}

void IRAM_ATTR n_bfsk_rx_cb(void *ctx, uint8_t *data, int cnt) {
    packet_mgr::load_rx_data(data, cnt);
}

void IRAM_ATTR n_packetmgr_rx_cb(void* ctx, int type, uint8_t* data, int cnt) {
    if(type == 0) {
        uint8_t errors = data[0];
        pc_packet_interface::pc_packet data_p;
        data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA;
        data_p.len = 1;
        data_p.data[0] = (errors & 0b111111) | (0b00 << 6);
        uart_sync_packet_send(data_p);
    } else if(type == 1) {
        uint8_t errors = data[0];
        uint8_t typelen = data[1];
        pc_packet_interface::pc_packet data_p;
        data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA;
        data_p.len = 1;
        data_p.data[0] = (errors & 0b111111) | (0b01 << 6);
        uart_sync_packet_send(data_p);
    } else if(type == 3) {
        pc_packet_interface::pc_packet data_p;
        data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA;
        data_p.len = 1;
        data_p.data[0] = (0 & 0b111111) | (0b11 << 6);
        uart_sync_packet_send(data_p);
        uint8_t parity = data[0];
        if(curr_tx_parity == parity) {
            curr_tx_parity = !curr_tx_parity;
        }
    } else if(type == 2 || type == 4) {
        pc_packet_interface::pc_packet data_p;
        data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA;
        data_p.len = cnt+1;
        if((type == 2) == curr_rx_parity) {
            data_p.data[0] = (0 & 0b111111) | (0b10 << 6);
            rightparity = true;
        } else {
            data_p.data[0] = (2 & 0b111111) | (0b10 << 6);
            rightparity = false;
        }
        for(int i = 0; i < cnt; i++) {
            data_p.data[i+1] = data[i];
        }
        uart_sync_packet_send(data_p);
        xTaskNotifyGive(acksend_task_hdl);
    } else if(type == 5) {
        pc_packet_interface::pc_packet data_p;
        data_p.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA;
        data_p.len = cnt+1;
        data_p.data[0] = (1 & 0b111111) | (0b10 << 6);
        for(int i = 0; i < cnt; i++) {
            data_p.data[i+1] = data[i];
        }
        uart_sync_packet_send(data_p);
    }
}

void IRAM_ATTR acksend_main(void *arg) {
    while(true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        switch (curr_mode) {
            case pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK:
                packet_mgr::load_tx_ack(rightparity ? curr_rx_parity : !curr_rx_parity);
                dsp_mgr::n_bfsk_tx_start();
                if(rightparity)
                    curr_rx_parity = !curr_rx_parity;
                break;
            case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK:
                packet_mgr::load_tx_ack(rightparity ? curr_rx_parity : !curr_rx_parity);
                dsp_mgr::n_mfsk_tx_start();
                if(rightparity)
                    curr_rx_parity = !curr_rx_parity;
                break;
            case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK:
                packet_mgr::load_tx_ack(rightparity ? curr_rx_parity : !curr_rx_parity);
                dsp_mgr::n_msk_tx_start();
                if(rightparity)
                    curr_rx_parity = !curr_rx_parity;
                break;
        }
    }
}

void IRAM_ATTR app_main(void) {
    mainuart = new chl_uart_dma(0, UART_BAUDRATE, MAINUART_RXD_GPIO, MAINUART_TXD_GPIO);
    printf("Main core: %d\n", xPortGetCoreID());
    params::init();
    pin_mgr::init();
    dsp_mgr::init();
    packet_mgr::init();
    packet_mgr::rx_set_cb(n_packetmgr_rx_cb, NULL);
    dsp_mgr::sdr_rx_set_cb(sdr_rx_cb, NULL);
    dsp_mgr::n_rx_set_cb(n_bfsk_rx_cb, NULL);
    dsp_mgr::n_tx_set_cb(n_bfsk_tx_cb, NULL);
    dsp_mgr::n_tx_set_reqfunc(packet_mgr::tx_reqfunc, NULL);
    xTaskCreatePinnedToCore(acksend_main, "ACKSENDTSK", 3584, NULL, ESP_TASK_TIMER_PRIO, &acksend_task_hdl, 0); //Create main task on DSP core(1)
    printf("Starting packet interface...\n");

    pc_packet_interface::pc_packet startp;
    startp.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_START;
    startp.len = 0;
    uart_sync_packet_send(startp);

    pc_packet_interface::pc_packet p;
    pc_packet_interface::pc_packet ackp;
    ackp.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_ACK;
    ackp.len = 1;
    ackp.data[0] = 0;
    while (true) {
        uart_sync_packet_read(&p);
        ackp.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_ACK;
        ackp.len = 1;
        ackp.data[0] = 0;
        switch (p.type) {
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_CHANGE_MODE: {
                curr_mode = p.data[0];
                if (curr_mode != pc_packet_interface::modes::PC_PI_MODE_UNINITED) {
                    pin_mgr::set_usbled_enable(true);
                } else {
                    pin_mgr::set_usbled_enable(false);
                }
                pin_mgr::set_dcd_enable(false);
                dsp_mgr::reset();
                packet_mgr::reset();
                curr_tx_parity = false;
                curr_rx_parity = false;
                ackp.data[0] = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SET_FR: {
                dsp_mgr::set_fr(*((float*) p.data));
                ackp.data[0] = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_SET_IN_SRC: {
                dsp_mgr::rx_set_ins(p.data[0]);
                ackp.data[0] = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_SET_SPD: {
                ackp.data[0] = 1;
                switch (curr_mode) {
                    case pc_packet_interface::modes::PC_PI_MODE_SDR: {
                        dsp_mgr::sdr_rx_set_sr(((float*) p.data)[0]);
                        break;
                    }
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK: {
                        float speed = ((float*) p.data)[0];
                        float frdiff = ((float*) p.data)[1];
                        dsp_mgr::n_bfsk_set_params(-frdiff / 2, frdiff / 2, speed);
                        break;
                    }
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK: {
                        float speed = ((float*) p.data)[0];
                        float frdiff = ((float*) p.data)[1];
                        int frcnt = roundf(((float*) p.data)[2]);
                        float space = frdiff / ((float) frcnt);
                        float frs[frcnt];
                        for (int i = 0; i < frcnt; i++) {
                            frs[i] = (space * ((i < frcnt / 2) ? i : i + 1)) - (frdiff / 2.0f);
                        }
                        dsp_mgr::n_mfsk_set_params(frcnt, frs, speed);
                        break;
                    }
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK: {
                        float speed = ((float*) p.data)[0];
                        dsp_mgr::n_msk_set_params(speed);
                        break;
                    }
                    default:
                        ackp.data[0] = 0;
                        break;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_START: {
                ackp.data[0] = 1;
                switch (curr_mode) {
                    case pc_packet_interface::modes::PC_PI_MODE_SDR:
                        dsp_mgr::sdr_rx_start();
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK:
                        dsp_mgr::n_bfsk_rx_start();
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK:
                        dsp_mgr::n_mfsk_rx_start();
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK:
                        dsp_mgr::n_msk_rx_start();
                        break;
                    default:
                        ackp.data[0] = 0;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_STOP: {
                ackp.data[0] = 1;
                switch (curr_mode) {
                    case pc_packet_interface::modes::PC_PI_MODE_SDR:
                        dsp_mgr::sdr_rx_stop(false);
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK:
                        dsp_mgr::n_bfsk_rx_stop(false);
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK:
                        dsp_mgr::n_mfsk_rx_stop(false);
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK:
                        dsp_mgr::n_msk_rx_stop(false);
                        break;
                    default:
                        ackp.data[0] = 0;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_START: {
                ackp.data[0] = 1;
                switch (curr_mode) {
                    case pc_packet_interface::modes::PC_PI_MODE_SDR:
                        dsp_mgr::sdr_tx_start();
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK:
                        dsp_mgr::n_bfsk_tx_start();
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK:
                        dsp_mgr::n_mfsk_tx_start();
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK:
                        dsp_mgr::n_msk_tx_start();
                        break;
                    default:
                        ackp.data[0] = 0;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_STOP: {
                ackp.data[0] = 1;
                switch (curr_mode) {
                    case pc_packet_interface::modes::PC_PI_MODE_SDR:
                        dsp_mgr::sdr_tx_stop(false);
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK:
                        dsp_mgr::n_bfsk_tx_stop(false);
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK:
                        dsp_mgr::n_mfsk_tx_stop(false);
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK:
                        dsp_mgr::n_msk_tx_stop(false);
                        break;
                    default:
                        ackp.data[0] = 0;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_DATA: {
                ackp.data[0] = 1;
                switch (curr_mode) {
                    case pc_packet_interface::modes::PC_PI_MODE_SDR:
                        for (int i = 0; i < p.len / 4; i++) {
                            sdr_tx_recv_data[i].i = (p.data[i * 4] * (1.0f / 32767.0f));
                            sdr_tx_recv_data[i].q = (p.data[i * 4 + 2] * (1.0f / 32767.0f));
                        }
                        ackp.data[0] = dsp_mgr::sdr_tx_put_data_sync(sdr_tx_recv_data, p.len / 4);
                        break;
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK:
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK:
                    case pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK:
                        packet_mgr::load_tx_data(p.data, p.len, curr_tx_parity);
                        break;
                    default:
                        ackp.data[0] = 0;
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_CARRIER: {
                dsp_mgr::tx_carrier();
                ackp.data[0] = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_READ: {
                if (p.len < 2) {
                    break;
                }
                uint8_t namelen = p.data[0];
                char name[16];
                for (int i = 0; i < namelen; i++) {
                    name[i] = p.data[i + 1];
                }
                name[namelen] = '\0';
                char def_val;
                char val[255];
                int len = params::readParam(std::string(name), val, &def_val, 255, 0);
                if (len == 0) {
                    uint32_t def = 0;
                    uint32_t rval = params::readParam(std::string(name), def);
                    ackp.data[0] = 1;
                    uart_sync_packet_send(ackp);
//                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    ackp.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_PARAM_DATA;
                    ackp.len = 4;
                    for(int i = 0; i < ackp.len; i++) {
                        ackp.data[i] = ((uint8_t*) &rval)[i];
                    }
                } else {
                    ackp.data[0] = 1;
                    uart_sync_packet_send(ackp);
//                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    ackp.type = pc_packet_interface::packetType_fromdev::PC_PI_PTD_PARAM_DATA;
                    ackp.len = len;
                    for(int i = 0; i < ackp.len; i++) {
                        ackp.data[i] = ((uint8_t*) &val)[i];
                    }
                }
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_WRITE: {
                if (p.len < 4) {
                    break;
                }
                uint8_t namelen = p.data[0];
                char name[16];
                for (int i = 0; i < namelen; i++) {
                    name[i] = p.data[i + 1];
                }
                name[namelen] = '\0';
                uint8_t datalen = p.data[namelen + 1];
                uint8_t data[datalen];
                for (int i = 0; i < datalen; i++) {
                    data[i] = p.data[namelen + 2 + i];
                }
                if (datalen == 4) {
                    uint32_t val = *(uint32_t*) data;
                    params::writeParam(name, val);
                } else {
                    params::writeParam(name, data, datalen);
                }
                ackp.data[0] = 1;
                break;
            }
            case pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_STORE: {
                params::store();
                dsp_mgr::reloadParams();
                ackp.data[0] = 1;
                break;
            }
        }
        uart_sync_packet_send(ackp);
    }
}
