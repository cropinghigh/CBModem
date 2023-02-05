#pragma once

#include <stdint.h>

class pc_packet_interface {
public:
    //Interface using binary packets, starting from startByte
    //Packet format: [startByte, packetType, packetLen(of data), packetData(if data), packet checksum]
    //Designed to work with some special applications, including SDR programs
    static const uint8_t startByte = 0x7C;
    enum packetType_fromdev {
        PC_PI_PTD_START, //Startup initialization ack(after reset), no data
        PC_PI_PTD_ACK, //Acknowledge, data: 1 byte = 1 or 0, depending on last packet execution result
        PC_PI_PTD_SDR_RX_DATA, //Received data in SDR mode, data: N complex vals(2 int16 - i & q)
    };
    enum packetType_frompc {
        PC_PI_PTP_CHANGE_MODE, //Change device mode, data: 1 byte - new mode
        PC_PI_PTP_SDR_RX_START, //Start sending data to PC in SDR mode, no data
        PC_PI_PTP_SDR_RX_STOP, //Stop sending data to PC in SDR mode, no data
        PC_PI_PTP_SDR_RX_SET_SR, //Change RX sample rate in SDR mode, data: 1 float - new sr
        PC_PI_PTP_SDR_RX_SET_FR, //Change RX center frequency in SDR mode, data: 1 float - new freq
        PC_PI_PTP_SDR_RX_SET_IN_SRC, //Change RX input channels pair in SDR mode, data: 1 byte = 1 or 0, depending on selected channels pair
        PC_PI_PTP_SDR_TX_START,
        PC_PI_PTP_SDR_TX_STOP,
    };
    enum modes {
        PC_PI_MODE_UNINITED,
        PC_PI_MODE_NORMAL,
        PC_PI_MODE_SDR,
    };
    class pc_packet {
    public:
        uint8_t type;
        uint8_t len;
        uint8_t* data;
        uint8_t checksum;
    };
    #define MAX_PC_P_SIZE 1+1+1+256+1
};
