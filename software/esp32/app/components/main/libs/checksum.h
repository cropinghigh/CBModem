#ifndef H_CHECKSUM
#define H_CHECKSUM

#include <stdint.h>

//VERSION=1.1
const uint8_t crc8_polynom = 0xD5;

uint8_t calc_crc8(uint8_t* data, int cnt) {
    uint8_t out = 0;
    for(int i = 0; i < cnt; i++) {
        out = out ^ data[i];
        for(int k = 0; k < 8; k++) {
            bool x = out & (1 << 7);
            out = (out << 1);
            if(x) {
                out = out ^ crc8_polynom;
            }
        }
    }
    return out;
}



#endif /* COMPONENTS_MAIN_LIBS_CHECKSUM_H_ */
