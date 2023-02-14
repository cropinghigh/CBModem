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
        PC_PI_PTD_N_TRANSMIT_COMPL, //Data transmitting in normal mode completed, no data
        PC_PI_PTD_N_RX_DATA, //Data receiving in normal mode completed, data: N received bytes
    };
    enum packetType_frompc {
        PC_PI_PTP_CHANGE_MODE, //Change device mode, data: 1 byte - new mode
        PC_PI_PTP_SET_FR, //Change center frequency, data: 1 float - new freq
        PC_PI_PTP_RX_SET_IN_SRC, //Change RX input channels pair, data: 1 byte = 1 or 0, depending on selected channels pair
        PC_PI_PTP_SDR_RX_START, //Start sending data to PC in SDR mode, no data
        PC_PI_PTP_SDR_RX_STOP, //Stop sending data to PC in SDR mode, no data
        PC_PI_PTP_SDR_RX_SET_SR, //Change RX sample rate in SDR mode, data: 1 float - new sr
        PC_PI_PTP_SDR_TX_START, //Start transmitting received data from PC, no data
        PC_PI_PTP_SDR_TX_STOP, //Stop transmitting, no data
        PC_PI_PTP_SDR_TX_DATA, //Put tx data to buffer, data: N complex vals(2 int16 - i & q)
        PC_PI_PTP_SDR_TX_CARRIER, //Start transmitting max amplitude carrier wave
        PC_PI_PTP_N_BFSK_TRANSMIT, //Transmit data in normal bfsk mode, data: N bytes - data
        PC_PI_PTP_N_BFSK_START_RX, //Start receiving in normal bfsk mode, no data
        PC_PI_PTP_N_BFSK_STOP_RX, //Stop receiving in normal bfsk mode, no data
        PC_PI_PTP_N_MFSK_TRANSMIT, //Transmit data in normal mfsk mode, data: N bytes - data
        PC_PI_PTP_N_MFSK_START_RX, //Start receiving in normal mfsk mode, no data
        PC_PI_PTP_N_MFSK_STOP_RX, //Stop receiving in normal mfsk mode, no data
    };
    enum modes {
        PC_PI_MODE_UNINITED,
        PC_PI_MODE_NORMAL_BFSK,
        PC_PI_MODE_NORMAL_MFSK,
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
