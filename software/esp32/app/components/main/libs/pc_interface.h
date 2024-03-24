#ifndef H_PC_INTERFACE
#define H_PC_INTERFACE
#include <stdint.h>

#define UART_BAUDRATE 115200

//VERSION=1.2
class pc_packet_interface {
    public:
        //Interface using binary packets, starting from startByte
        //Packet format: [startByte, packetType, packetLen(of data), packetData(if data), packet checksum]
        //Designed to work with special applications, including SDR software
        static const uint8_t startByte = 0x7C;
        enum packetType_fromdev {
            PC_PI_PTD_START, //Startup initialization ack(after reset), no data
            PC_PI_PTD_ACK, //Acknowledge, data: 1 byte = 1 or 0, depending on last packet execution result
            PC_PI_PTD_SDR_RX_DATA, //Received data in SDR mode, data: N complex vals(2 int16 - i & q)
            PC_PI_PTD_N_TRANSMIT_COMPL, //Data transmitting in normal mode completed, no data
            PC_PI_PTD_N_RX_DATA, //Data receiving in normal mode completed, data: 1 byte - info(6 low bits-signal quality, 2 high bits-status(0-SW read, 1-type+len read, 2-data read, 3-ack read)) + N received bytes
            PC_PI_PTD_PARAM_DATA, //Requested parameter value, data: N bytes - data
        };
        enum packetType_frompc {
            PC_PI_PTP_CHANGE_MODE, //Change device mode, data: 1 byte - new mode
            PC_PI_PTP_SET_FR, //Change center frequency, data: 1 float - new freq
            PC_PI_PTP_RX_SET_IN_SRC, //Change RX input channels pair, data: 1 byte = 1 or 0, depending on selected channels pair
            PC_PI_PTP_SET_SPD, //Change RX/TX sample rate in SDR mode, bit rate in normal mode(+frequencies), data: in SDR mode 1 float - new sr, in normal bfsk mode 2 floats - new bit rate, frequency difference, in mfsk mode 3 floats - new bit rate, frequency difference, frequency count, in msk mode 1 float - new bit rate
            PC_PI_PTP_RX_START, //Start receiving in current selected mode, no data
            PC_PI_PTP_RX_STOP, //Stop receiving in current selected mode, no data
            PC_PI_PTP_TX_START, //Start transmitting in current selected mode, no data
            PC_PI_PTP_TX_STOP, //Stop transmitting in current selected mode, no data
            PC_PI_PTP_TX_DATA, //Put data to tx buffer, data: in SDR mode N complex vals(2 int16 - i & q), in normal mode N bytes of packet data
            PC_PI_PTP_TX_CARRIER, //Start transmitting max amplitude carrier wave, no data
            CURSED,
            PC_PI_PTP_PARAM_READ, //Request reading parameter, data: 1 byte-name length(1-15), N bytes - name
            PC_PI_PTP_PARAM_WRITE, //Write parameter to device, data: 1 byte-name length(1-15), N bytes - name, 1 byte-value length, K bytes - value
            PC_PI_PTP_PARAM_STORE, //Store written params to flash, no data
        };
        enum modes {
            PC_PI_MODE_UNINITED,
            PC_PI_MODE_NORMAL_BFSK,
            PC_PI_MODE_NORMAL_MFSK,
            PC_PI_MODE_NORMAL_MSK,
            PC_PI_MODE_SDR,
        };
        class pc_packet {
            public:
                uint8_t type;
                uint8_t len;
                uint8_t data[256];
                uint8_t checksum;
        };
#define MAX_PC_P_SIZE 1+1+1+256+1
};

#endif
