#ifndef H_PACKET_MGR
#define H_PACKET_MGR

#include "checksum.h"

#define PACKETMGR_BUFF_SIZE 256
#define SYNCWORD_32 0x47545A20
#define ALLOWED_SW_ERRORS 2

/*
 Packet structure:
 [preamble] - 32 alternating bits
 [syncword] - 32 predefined bits to trigger receiver
 [typelen] repeated 8 times - 1 byte
     bits 0-5 - length of the data(1-64)
     bits 6 - type(0-data, 1-ack)
     bit 7 - packet parity(changes at every packet)
 [data] - 0-63 bytes
 [checksum] - 1 byte
 */
namespace packet_mgr {

    void init();
    void reset();
    void tx_data_incr();
    void load_tx_data(uint8_t* data, uint8_t cnt, bool parity);
    void load_tx_ack(bool parity);
    int IRAM_ATTR tx_reqfunc(void* ctx, uint8_t* data, int samples_cnt);
    void load_rx_data(uint8_t* data, int cnt);
    void rx_set_cb(void (*new_rx_cb)(void*, int, uint8_t*, int), void *ctx);

    uint8_t tx_data_buff[PACKETMGR_BUFF_SIZE];
    int tx_data_buff_data = 0;
    int tx_data_buff_data_bit = 0;
    int tx_data_buff_data_r = 0;
    int tx_data_buff_data_rbit = 0;

    uint32_t rx_shift_reg;
    uint8_t rx_data_buff[PACKETMGR_BUFF_SIZE];
//    uint8_t rx_len = 0;
    uint8_t rx_typelen = 0;
    uint8_t rx_checksum = 0;
    int rx_typelen_ctr = 0;
    int rx_data_buff_ptr = 0;
    int rx_data_buff_bit = 0;
    int rx_state = 0;
    void (*rx_cb)(void*, int, uint8_t*, int) = NULL;
    void *rx_cb_ctx;
};

void packet_mgr::init() {
    
}

void packet_mgr::reset() {
    tx_data_buff_data = 0;
    tx_data_buff_data_bit = 0;
    tx_data_buff_data_r = 0;
    tx_data_buff_data_rbit = 0;
    rx_typelen_ctr = 0;
    rx_data_buff_ptr = 0;
    rx_data_buff_bit = 0;
    rx_state = 0;
}

void packet_mgr::tx_data_incr() {
    tx_data_buff_data_bit++;
    if(tx_data_buff_data_bit >= 8) {
        tx_data_buff_data_bit = 0;
        if(tx_data_buff_data < PACKETMGR_BUFF_SIZE) {
            tx_data_buff_data++;
        }
    }
}

void packet_mgr::load_tx_data(uint8_t* data, uint8_t cnt, bool parity) {
    if(tx_data_buff_data != 0) { printf("TX DB OVF%d\n", tx_data_buff_data); return; }

    tx_data_buff_data_r = 0;
    tx_data_buff_data_rbit = 0;
    tx_data_buff_data_bit = 0;
    for(int i = 0; i < PACKETMGR_BUFF_SIZE; i++) {
        tx_data_buff[i] = 0;
    }
    for(int i = 0; i < 32; i++) {
        tx_data_buff[tx_data_buff_data] |= (((SYNCWORD_32 & (1 << (i))) >> (i)) << tx_data_buff_data_bit);
        tx_data_incr();
    }
    uint8_t typelen_byte = (cnt & 0b00111111) | (0b0 << 6) | (parity << 7);
    for(int i = 0; i < 8; i++) {
        for(int k = 0; k < 8; k++) {
            tx_data_buff[tx_data_buff_data] |= ((typelen_byte & (1 << k)) >> k ) << tx_data_buff_data_bit;
            tx_data_incr();
        }
    }
    uint8_t checksum = calc_crc8(data, cnt);
    for(int k = 0; k < 8; k++) {
        tx_data_buff[tx_data_buff_data] |= (((checksum & (1 << k)) >> k) << tx_data_buff_data_bit);
        tx_data_incr();
    }
    for(int i = 0; i < cnt; i++) {
        for(int k = 0; k < 8; k++) {
            tx_data_buff[tx_data_buff_data] |= (((data[i] & (1 << k)) >> k) << tx_data_buff_data_bit);
            tx_data_incr();
        }
    }
    for(int i = 0; i < 8; i++) {
        tx_data_incr(); //To make receiver not lose last byte
    }
}

void packet_mgr::load_tx_ack(bool parity) {
    if(tx_data_buff_data != 0) { printf("TX DB OVF%d\n", tx_data_buff_data); return; }

    tx_data_buff_data_r = 0;
    tx_data_buff_data_rbit = 0;
    tx_data_buff_data_bit = 0;
    for(int i = 0; i < PACKETMGR_BUFF_SIZE; i++) {
        tx_data_buff[i] = 0;
    }
    for(int i = 0;i < 32; i++) {
        tx_data_buff[tx_data_buff_data] |= (((SYNCWORD_32 & (1 << (i))) >> (i)) << tx_data_buff_data_bit);
        tx_data_incr();
    }
    uint8_t typelen_byte = (0b1 << 6) | (parity << 7);
    for(int i = 0; i < 8; i++) {
        for(int k = 0; k < 8; k++) {
            tx_data_buff[tx_data_buff_data] |= ((typelen_byte & (1 << k)) >> k ) << tx_data_buff_data_bit;
            tx_data_incr();
        }
    }
    for(int i = 0; i < 8; i++) {
        tx_data_incr(); //To make receiver not lose last byte
    }
}

int packet_mgr::tx_reqfunc(void* ctx, uint8_t* data, int samples_cnt) {
    //1bit in byte
    for(int i = 0; i < samples_cnt; i++) {
        if(tx_data_buff_data_r == tx_data_buff_data || tx_data_buff_data==0) {
            tx_data_buff_data = 0;
            return (i == 0) ? -10 : i;
        }
        data[i] = ((tx_data_buff[tx_data_buff_data_r] & (1 << tx_data_buff_data_rbit)) >> tx_data_buff_data_rbit);
        tx_data_buff_data_rbit++;
        if(tx_data_buff_data_rbit >= 8) {
            tx_data_buff_data_rbit = 0;
            if(tx_data_buff_data_r < tx_data_buff_data) {
                tx_data_buff_data_r++;
            }
        }
    }
    return samples_cnt;
}

float frerr = 0;

int find_bit_diffs(uint32_t a, uint32_t b) {
    int ret = 0;
    for(int i = 0; i < 32; i++) {
        if((a & (1 << i)) != (b & (1 << i))) {
            ret++;
        }
    }
    return ret;
}

void packet_mgr::load_rx_data(uint8_t* data, int cnt) {
//     printf("BITS ");
    for(int i = 0; i < cnt; i++) {
        int bit = (data[i] & 0b1);
//         printf("%d", bit);
        rx_shift_reg = rx_shift_reg >> 1;
        rx_shift_reg |= (bit << 31);
        if(rx_state == 1) { //Reading typelen byte
            rx_data_buff[rx_data_buff_bit] += bit;
            rx_data_buff_bit++;
            if(rx_data_buff_bit >= 8) {
                rx_data_buff_bit = 0;
                rx_data_buff_ptr++;
                if(rx_data_buff_ptr >= 8) {
                    rx_typelen = 0;
                    uint8_t errs = 0;
                    for(int k = 0; k < 8; k++) {
                        rx_typelen |= (rx_data_buff[k] > 4) << k;
                        errs += abs(rx_data_buff[k] - ((rx_data_buff[k] > 4) ? 8 : 0));
                    }
                    uint8_t len = rx_typelen & 0b00111111;
                    uint8_t type = (rx_typelen & 0b1000000) >> 6;
                    if(type == 0) {
                        rx_state = 3;
                        rx_checksum = 0;
                        rx_data_buff_bit = 0;
                        rx_data_buff_ptr = 0;
                        for(int k = 0; k < len; k++) {
                            rx_data_buff[k] = 0;
                        }
                        if(rx_cb != NULL) {
                            uint8_t st[2];
                            st[0] = errs;
                            st[1] = rx_typelen;
                            rx_cb(rx_cb_ctx, 1, st, 2); //Report successfull typelen reading
                        }
                    } else if(type == 1) {
                        rx_state = 0;
                        if(rx_cb != NULL) {
                            uint8_t st[2];
                            st[0] = errs;
                            st[1] = rx_typelen;
                            uint8_t parity = (rx_typelen & 0b10000000) >> 7;
                            rx_cb(rx_cb_ctx, 1, st, 2); //Report successfull typelen reading
                            rx_cb(rx_cb_ctx, 3, &parity, 1); //Report successfull ack reading
                        }
                    }
//                    printf("TYPELEN READ: %d(%d errors)\n", rx_typelen, errs);
                }
            }
        } else if(rx_state == 3) { //Receiving checksum
            rx_checksum |= (bit << rx_data_buff_bit++);
            if(rx_data_buff_bit >= 8) {
                rx_data_buff_bit = 0;
                rx_state = 2;
            }
        } else if(rx_state == 2) { //Reading data
            uint8_t len = rx_typelen & 0b00111111;
            rx_data_buff[rx_data_buff_ptr] |= (bit << rx_data_buff_bit++);
            if(rx_data_buff_bit >= 8) {
                rx_data_buff_bit = 0;
                rx_data_buff_ptr++;
                if(rx_data_buff_ptr >= len) {
                    rx_state = 0;
//                    uint8_t checksum = rx_data_buff[len];
                    uint8_t parity = (rx_typelen & 0b10000000) >> 7;
                    uint8_t calc_checksum = calc_crc8(rx_data_buff, len);
                    if(rx_cb != NULL) {
                        if(calc_checksum == rx_checksum) {
                            rx_cb(rx_cb_ctx, parity ? 2 : 4, rx_data_buff, len); //Report successfull data reading
                        } else {
                            rx_cb(rx_cb_ctx, 5, rx_data_buff, len); //Report bad crc data reading
                        }
                    }
//                    printf("DATA READ:");
//                    for(int k = 0; k < len; k++) {
//                        printf(" %c", rx_data_buff[k]);
//                    }
//                    printf("\n");
                }
            }
        } else { //Searching for syncword
            uint8_t errs = find_bit_diffs(rx_shift_reg, SYNCWORD_32);
            if(errs <= ALLOWED_SW_ERRORS) {
                rx_state = 1;
                rx_data_buff_ptr = 0;
                rx_data_buff_bit = 0;
                rx_typelen_ctr = 0;
                for(int k = 0; k < 8; k++) {
                    rx_data_buff[k] = 0;
                }
//                printf("SW FOUND(%d errs)!\n", errs);
//                printf("ERRORS %f %f\n", dsp_n_bfskdemod._avgerr0, dsp_n_bfskdemod._avgerr1);
                if(rx_cb != NULL) {
                    rx_cb(rx_cb_ctx, 0, &errs, 1); //Report successfull SW reading
                }
            }
        }
    }
//     printf("\n");
}

void packet_mgr::rx_set_cb(void (*new_rx_cb)(void*, int, uint8_t*, int), void *ctx) {
    rx_cb = new_rx_cb;
    rx_cb_ctx = ctx;
}



#endif

