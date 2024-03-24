#pragma once

#include <iostream>
#include <map>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <thread>
#include <stdio.h>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <semaphore>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <pthread.h>
#include <poll.h>
#include <format>
#include <cassert>
#include <iomanip>

#define READ_TIMEOUT_MS 400
#define RX2_BYTES_LIMIT 512
#define RX_DATA_RETSHIFT_DUP 2000
#define RX_DATA_RETSHIFT_BADCRC 1000
#define RX_DATA_RET_SW -10
#define RX_DATA_RET_LEN -11
#define RX_DATA_RET_ACK -13

namespace cbmodem {

    //VERSION=1.3
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

    //VERSION=1.1
    const uint8_t crc8_polynom = 0xD5;

    uint8_t calc_crc8(uint8_t* data, int cnt);

    template<typename T>
    class ModemPILockingQueue {
        public:
            ~ModemPILockingQueue();
            bool push(T const &_data);
            bool empty();
            size_t size();
            int tryPop(T &_value);
            bool waitAndPop(T &_value);
            int tryWaitAndPop(T &_value, int _milli);
            void setWorking(bool newwork);

        private:
            std::queue<T> queue;
            mutable std::mutex guard;
            std::condition_variable signal;
            bool working = false;
    };

    class ModemPacketInterface {
        public:
            ~ModemPacketInterface();
            void init(std::string serial_dev);
            bool start();
            void stop();
            void reset_mcu();
            void change_mode(pc_packet_interface::modes newmode, bool reset);
            void set_fr(float fr);
            void set_inputs(uint8_t hi);
            void set_speed(float speed, float frdiff, int frcnt);
            void start_rx();
            void stop_rx();
            void start_tx();
            void start_tx_wait();
            void stop_tx();
            void start_tx_carrier();
            void put_sdr_tx_samples(int16_t *iq_data, int cnt);
            void put_tx_data(char *data, uint8_t cnt);
            int receive_sdr_rx_samples(int16_t *iq_data);
            int receive_rx_data(char *data, int maxsz);
            void wait_for_tx();
            int read_param(std::string name, void *data, int len);
            void write_param(std::string name, void *data, int len);
            void store_params();
            int read_rx2_char(char &c);

        private:
            static void _rx_thread_func(void *ctx);
            static void _tx_thread_func(void *ctx);
            int _read_packet(pc_packet_interface::pc_packet &p); //Blocks until packet is read or timeout
            bool _send_packet(pc_packet_interface::pc_packet &p);
            uint8_t _read_packet_ack();

            std::string _serial_dev = "";
            int _fd = -1;
            struct termios _tty;
            bool _working = false;
            std::mutex _tty_mtx;
            ModemPILockingQueue<pc_packet_interface::pc_packet> _rx_queue;
            ModemPILockingQueue<char> _rx2_queue;
            ModemPILockingQueue<pc_packet_interface::pc_packet> _tx_queue;
            std::thread *_tx_thr;
            std::thread *_rx_thr;
            uint8_t _packet_read_buff[MAX_PC_P_SIZE];
            const int _none_flag = 0;
            const int _RTS_flag = TIOCM_RTS;
            const int _DTR_flag = TIOCM_DTR;
            const int _RTSDTR_flag = TIOCM_RTS | TIOCM_DTR;
            int _packet_read_buff_data = 0;
            int _packet_read_buff_pos = 0;

    };

}
