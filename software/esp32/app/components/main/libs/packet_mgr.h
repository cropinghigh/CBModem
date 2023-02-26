#ifndef H_PACKET_MGR
#define H_PACKET_MGR

// #include "params.h"

#define PACKETMGR_BUFF_SIZE 256
#define SYNCWORD_32 0x47545A20

namespace packet_mgr {

    void init();
    void tx_data_incr();
    void load_tx_data(uint8_t* data, int cnt);
    int IRAM_ATTR tx_reqfunc(void* ctx, uint8_t* data, int samples_cnt);
    void load_rx_data(uint8_t* data, int cnt);

    uint8_t tx_data_buff[PACKETMGR_BUFF_SIZE];
    int tx_data_buff_data = 0;
    int tx_data_buff_data_bit = 0;
    int tx_data_buff_data_r = 0;
    int tx_data_buff_data_rbit = 0;

    uint32_t rx_shift_reg;
    uint8_t rx_data_buff[PACKETMGR_BUFF_SIZE];
    uint8_t rx_len = 0;
    int rx_data_buff_ptr = 0;
    int rx_data_buff_bit = 0;
    int rx_state = 0;
};

void packet_mgr::init() {
    
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

void packet_mgr::load_tx_data(uint8_t* data, int cnt) {
    if(tx_data_buff_data != 0) { printf("TX DB OVF%d\n", tx_data_buff_data); return; }
    // int tmp = 0;
    tx_data_buff_data_r = 0;
    tx_data_buff_data_rbit = 0;
    tx_data_buff_data_bit = 0;
    for(int i = 0; i < PACKETMGR_BUFF_SIZE; i++) {
        tx_data_buff[i] = 0;
    }
    // for(int i = 0; i < PACKETMGR_PREAMBLE_SIZE; i++) {
    //     tx_data_buff[tx_data_buff_data] |= (tmp << tx_data_buff_data_bit);
    //     tx_data_incr();
    //     tmp = 1 - tmp;
    // }
    for(int i = 0;i < 32; i++) {
        tx_data_buff[tx_data_buff_data] |= (((SYNCWORD_32 & (1 << (i))) >> (i)) << tx_data_buff_data_bit);
        tx_data_incr();
    }
    for(int i = 0;i < 8; i++) {
        for(int k = 0; k < 8; k++) { //Make every bit a byte for more reliable length transmission, because it's very important value
            tx_data_buff[tx_data_buff_data] |= (((((uint8_t)cnt) & (1 << (i))) >> (i)) << tx_data_buff_data_bit);
            tx_data_incr();
        }
    }
    for(int i = 0;i < cnt; i++) {
        for(int k = 0; k < 8; k++) {
            tx_data_buff[tx_data_buff_data] |= (((data[i] & (1 << k)) >> k) << tx_data_buff_data_bit);
            tx_data_incr();
        }
    }
}

int packet_mgr::tx_reqfunc(void* ctx, uint8_t* data, int samples_cnt) {
    //1bit in byte
    for(int i = 0; i < samples_cnt; i++) {
        if(tx_data_buff_data_r == tx_data_buff_data || tx_data_buff_data==0) {
            tx_data_buff_data = 0;
            // printf("ret2 %d %d\n", i, tx_data_buff_data_rbit);
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
    // printf("ret1 %d\n", samples_cnt);
    return samples_cnt;
}

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
        if(rx_state == 1) { //Reading length
            rx_data_buff[rx_data_buff_ptr] += bit;
            rx_data_buff_bit++;
            if(rx_data_buff_bit >= 8) {
                rx_data_buff_bit = 0;
                rx_data_buff_ptr++;
                if(rx_data_buff_ptr >= 8) {
                    rx_len = 0;
                    for(int k = 0; k < 8; k++) {
                        rx_len |= (rx_data_buff[k] >= 4) << k;
                    }
                    rx_state = 2;
                    printf("LEN READ: %d(%d %d %d %d %d %d %d %d)\n", rx_len, rx_data_buff[0], rx_data_buff[1], rx_data_buff[2], rx_data_buff[3], rx_data_buff[4], rx_data_buff[5], rx_data_buff[6], rx_data_buff[7]);
                    rx_data_buff_bit = 0;
                    rx_data_buff_ptr = 0;
                    for(int k = 0; k < rx_len; k++) {
                        rx_data_buff[k] = 0;
                    }
                }
            }
        } else if(rx_state == 2) { //Reading data
            rx_data_buff[rx_data_buff_ptr] |= (bit << rx_data_buff_bit++);
            if(rx_data_buff_bit >= 8) {
                rx_data_buff_bit = 0;
                rx_data_buff_ptr++;
                if(rx_data_buff_ptr >= rx_len) {
                    rx_state = 0;
                    printf("DATA READ:");
                    for(int k = 0; k < rx_len; k++) {
                        printf(" %c", rx_data_buff[k]);
                    }
                    printf("\n");
                }
            }
        } else { //Searching for syncword
            int errs = find_bit_diffs(rx_shift_reg, SYNCWORD_32);
            if(errs < 3) {
                printf("SW FOUND(%d errs)!\n", errs);
                rx_state = 1;
                rx_data_buff_ptr = 0;
                rx_data_buff_bit = 0;
                for(int k = 0; k < 8; k++) {
                    rx_data_buff[k] = 0;
                }
            }
        }
    }
//     printf("\n");
}



#endif

