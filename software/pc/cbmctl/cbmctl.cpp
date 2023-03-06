#include <iostream>
#include <map>
#include "../../esp32/app/components/main/libs/pc_interface.h"
#include "../../esp32/app/components/main/libs/checksum.h"
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

template<typename T>
class LockingQueue {
    public:
        bool push(T const &_data) {
            if (!working) {
                return false;
            }
            {
                std::lock_guard<std::mutex> lock(guard);
                queue.push(_data);
            }
            signal.notify_one();
            return true;
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(guard);
            return queue.empty() || !working;
        }

        int tryPop(T &_value) {
            std::lock_guard<std::mutex> lock(guard);
            if (!working) {
                return -1;
            }
            if (queue.empty()) {
                return -2;
            }

            _value = queue.front();
            queue.pop();
            return 0;
        }

        bool waitAndPop(T &_value) {
            std::unique_lock<std::mutex> lock(guard);
            while (true) {
                if (!working) {
                    return false;
                } else if (queue.empty()) {
                    signal.wait(lock);
                } else {
                    break;
                }
            }

            _value = queue.front();
            queue.pop();
            return true;
        }

        int tryWaitAndPop(T &_value, int _milli) {
            std::unique_lock<std::mutex> lock(guard);
            while (true) {
                if (!working) {
                    return -1;
                } else if (queue.empty()) {
                    std::cv_status x = signal.wait_for(lock, std::chrono::milliseconds(_milli));
                    if (x == std::cv_status::timeout) {
                        return -2;
                    }
                } else {
                    break;
                }
            }

            _value = queue.front();
            queue.pop();
            return 0;
        }

        void setWorking(bool newwork) {
            working = newwork;
            signal.notify_one();
            signal.notify_all();
            signal.notify_all();
            guard.unlock();
        }

    private:
        std::queue<T> queue;
        mutable std::mutex guard;
        std::condition_variable signal;
        bool working = false;
};

class ModemPacketInterface {
    public:
        bool start(std::string ser) {
            fd = open(ser.c_str(), O_RDWR);
            if (fd < 0) {
                printf("ModemPI: Port open failed: %s\n", strerror(errno));
                return false;
            }
            if (tcgetattr(fd, &(tty)) != 0) {
                printf("ModemPI: tcgetattr() failed! Error: %s\n", strerror(errno));
                close(fd);
                return false;
            }
            tty.c_cflag &= ~CRTSCTS;   // Disable RTS/CTS control flow
            tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
            tty.c_lflag &= ~ICANON;
            tty.c_lflag &= ~ECHO; // Disable echo
            tty.c_lflag &= ~ECHOE; // Disable erasure
            tty.c_lflag &= ~ECHONL; // Disable new-line echo
            tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
            tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
            tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytess
            tty.c_cc[VTIME] = 3;
            tty.c_cc[VMIN] = 0;
            if (cfsetspeed(&(tty), B1000000)) {
                printf("ModemPI: cfsetispeed() failed! Error: %s\n", strerror(errno));
                close(fd);
                return false;
            }
            if (tcsetattr(fd, TCSANOW, &(tty)) != 0) {
                printf("ModemPI: tcsetattr() failed! Error: %s\n", strerror(errno));
                close(fd);
                return false;
            }
            usleep(500);
            ioctl(fd, TCFLSH, 2); // flush R and T buffs
            rx_queue.setWorking(true);
            tx_queue.setWorking(true);
            tx_thr = new std::thread(tx_thread_func, this);
            rx_thr = new std::thread(rx_thread_func, this);
            working = true;
            return true;
        }

        void stop() {
            tty_mtx.unlock();
            working = false;

            if (fd != -1) {
                uint8_t newmode = pc_packet_interface::modes::PC_PI_MODE_UNINITED;
                pc_packet_interface::pc_packet startp;
                startp.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_CHANGE_MODE;
                startp.len = 1;
                startp.data[0] = newmode;
                send_packet(startp);
                read_packet_ack();
                rx_queue.setWorking(false);
                tx_queue.setWorking(false);
                int oldfd = fd;
                fd = -1;
                close(oldfd);
                rx_thr->join();
                tx_thr->join();
                delete rx_thr;
                delete tx_thr;
            }
        }

        static void rx_thread_func(void *ctx) {
            ModemPacketInterface *_this = (ModemPacketInterface*) ctx;
            pthread_setname_np(pthread_self(), "modemPI_rx");
            int state = 0; //0-receiving startByte; 1-receiving type; 2-receiving len; 3-receiving data; 4-receiving checksum
            int outdata_pos = 0;
            pc_packet_interface::pc_packet readp;
            uint8_t checksum_buff[sizeof(pc_packet_interface::pc_packet)];
            checksum_buff[0] = pc_packet_interface::startByte;
            while (1) {
                while (_this->packet_read_buff_data > 0) {
//              if((_this->packet_read_buff[_this->packet_read_buff_pos] < 0x20 || _this->packet_read_buff[_this->packet_read_buff_pos] > 0x7F) && _this->packet_read_buff[_this->packet_read_buff_pos] != '\n' && _this->packet_read_buff[_this->packet_read_buff_pos] != '\r') {
//                  printf("[%x]", _this->packet_read_buff[_this->packet_read_buff_pos]);
//              } else {
//                  printf("%c", _this->packet_read_buff[_this->packet_read_buff_pos]);
//              }
//              fflush(stdout);
                    if (state == 0) {
                        if (_this->packet_read_buff[_this->packet_read_buff_pos] == pc_packet_interface::startByte) {
                            state = 1;
//                         printf("s1\n");
                        } else {
                            if(_this->packet_read_buff[_this->packet_read_buff_pos] != 0x00) {
                                if ((_this->packet_read_buff[_this->packet_read_buff_pos] < 0x20 || _this->packet_read_buff[_this->packet_read_buff_pos] > 0x7F) && _this->packet_read_buff[_this->packet_read_buff_pos] != '\n' && _this->packet_read_buff[_this->packet_read_buff_pos] != '\r') {
                                    printf("[%x]", _this->packet_read_buff[_this->packet_read_buff_pos]);
                                } else {
                                    printf("%c", _this->packet_read_buff[_this->packet_read_buff_pos]);
                                }
                                fflush(stdout);
                            }
                        }
                    } else if (state == 1) {
                        readp.type = _this->packet_read_buff[_this->packet_read_buff_pos];
                        checksum_buff[1] = readp.type;
                        state = 2;
//                    printf("s2 %d\n", readp.type);
                    } else if (state == 2) {
                        readp.len = _this->packet_read_buff[_this->packet_read_buff_pos];
                        checksum_buff[2] = readp.len;
                        if (readp.len == 0) {
                            state = 4;
//                         printf("s4\n");
                        } else {
                            state = 3;
//                         printf("s3 %d\n", readp.len);
                        }
                    } else if (state == 3) {
                        readp.data[outdata_pos] = _this->packet_read_buff[_this->packet_read_buff_pos];
                        checksum_buff[3+outdata_pos] = readp.data[outdata_pos];
                        outdata_pos++;
                        if (outdata_pos >= readp.len) {
                            state = 4;
//                             printf("s4\n");
                        }
                    } else if (state == 4) {
                        readp.checksum = _this->packet_read_buff[_this->packet_read_buff_pos];
                        uint8_t calc_checksum = calc_crc8(checksum_buff, 3+outdata_pos);
//                    printf("Shpack read: t %d l %d d0 %d c %d\n", readp.type, readp.len, (readp.len == 0 ? 0 : readp.data[0]), readp.checksum);
                        _this->packet_read_buff_pos++;
                        _this->packet_read_buff_data--;
                        state = 0;
                        outdata_pos = 0;
                        if(calc_checksum == readp.checksum) {
                            if (!_this->rx_queue.push(readp)) {
                                _this->rx_queue.setWorking(false);
                                return;
                            }
                        } else {
                            printf("HOST WRONG CRC(%x %x %d %d)!\n", calc_checksum, readp.checksum, readp.len, 3+outdata_pos);
                        }
                        //Read success
                    }
                    _this->packet_read_buff_pos++;
                    _this->packet_read_buff_data--;
                }
                if (_this->fd < 0) {
                    _this->rx_queue.setWorking(false);
                    return;
                }
                int r = read(_this->fd, _this->packet_read_buff, 4);
                // printf("rdata: %d\n", r);
                if (r < 0) {
                    if (_this->fd >= 0) {
                        printf("ModemPI: serial read failed! Error: %s\n", strerror(r));
                    }
                    _this->rx_queue.setWorking(false);
                    return;
                }
                _this->packet_read_buff_data = r;
                _this->packet_read_buff_pos = 0;
            }
        }

        static void tx_thread_func(void *ctx) {
            ModemPacketInterface *_this = (ModemPacketInterface*) ctx;
            pthread_setname_np(pthread_self(), "modemPI_tx");
            pc_packet_interface::pc_packet sendp;
            while (true) {
                if (!_this->tx_queue.waitAndPop(sendp)) {
                    _this->tx_queue.setWorking(false);
                    return;
                }
                int bufs = 5 + sendp.len;
                uint8_t packet_send_buff[bufs];
                packet_send_buff[0] = 0xaa;
                packet_send_buff[1] = pc_packet_interface::startByte;
                packet_send_buff[2] = sendp.type;
                packet_send_buff[3] = sendp.len;
                if (sendp.len > 0) {
                    for (int i = 0; i < sendp.len; i++) {
                        packet_send_buff[4 + i] = sendp.data[i];
                    }
                }
                packet_send_buff[4 + sendp.len] = calc_crc8(&packet_send_buff[1], 3+sendp.len);
                if (_this->fd < 0) {
                    _this->tx_queue.setWorking(false);
                    return;
                }
//            printf("Shpack written: t %d l %d d0 %d c %d\n", sendp.type, sendp.len, (sendp.len == 0 ? 0 : sendp.data[0]), sendp.checksum);
//            for(int i = 0; i < bufs; i++) {
//              if((packet_send_buff[i] < 0x20 || packet_send_buff[i] > 0x7F)) {
//                  printf("[%x]", packet_send_buff[i]);
//              } else {
//                  printf("%c", packet_send_buff[i]);
//              }
//            }
//            printf("\n");
                write(_this->fd, &packet_send_buff[0], 1);
                usleep(10000UL);
                write(_this->fd, &packet_send_buff[1], 1);
                usleep(10000UL);

                for(int i = 2; i < bufs; i+=16) {
                    write(_this->fd, &packet_send_buff[i], std::min(16, bufs-i));
                    fsync(_this->fd);
                    usleep(5000);
                }
//                write(_this->fd, packet_send_buff, bufs);
            }
        }

        void reset_mcu() {
            ioctl(fd, TIOCMSET, &RTS_flag);
            usleep(500UL * 1000UL);
            ioctl(fd, TIOCMSET, &none_flag);
        }

        //Blocks until packet is read or timeout
        int read_packet(pc_packet_interface::pc_packet &p) {
//        printf("Read req\n");
            int ret = rx_queue.tryWaitAndPop(p, 300);
            return ret;
        }

        bool send_packet(pc_packet_interface::pc_packet &p) {
//        printf("Write req\n");
            if (!tx_queue.push(p)) {
                return -1;
            }
//        printf("Write req fin\n");
            return 0;
        }

        uint8_t read_packet_ack() {
            // printf("Waiting for ack...\n");
            int ctr = 0;
            while (1) {
                pc_packet_interface::pc_packet p;
                if (read_packet(p) != 0) {
                    printf("ModemPI: Ack packet read failed!\n");
                    return 0;
                }
                if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_ACK) {
                    return p.data[0];
                }
                ctr++;
                if (ctr >= 50) {
                    return 0; //too many packets; ack probably missed
                }
            }
        }

        void change_mode(pc_packet_interface::modes newmode, bool reset) {
            std::lock_guard<std::mutex> lock(tty_mtx);
            usleep(500);
            ioctl(fd, TCFLSH, 2); // flush R and T buffs
            if (reset) {
                reset_mcu();
                while (1) {
                    pc_packet_interface::pc_packet p;
                    int ret = read_packet(p);
                    if (ret == -1 || !working) {
                        printf("ModemPI: Init packet read failed!\n");
                        return;
                    } else if(ret == -2) {
                        continue;
                    }
                    if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_START) {
                        break;
                    }
                }
            }
            pc_packet_interface::pc_packet modep;
            modep.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_CHANGE_MODE;
            modep.len = 1;
            modep.data[0] = (uint8_t) newmode;
            send_packet(modep);
            if (!read_packet_ack()) {
                printf("ModemPI: Mode set NACK!\n");
            }
        }

        void set_fr(float fr) {
            std::lock_guard<std::mutex> lock(tty_mtx);
            // printf("Fr req\n");
            // printf("Fr mtx lock\n");
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_SET_FR;
            p.len = sizeof(float);
            for (int i = 0; i < sizeof(float); i++) {
                p.data[i] = ((uint8_t*) (&fr))[i];
            }
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Fr set NACK!\n");
            }
            // printf("Fr mtx unlock\n");
        }

        void set_inputs(uint8_t hi) {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_SET_IN_SRC;
            p.len = 1;
            p.data[0] = hi;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Ins set NACK!\n");
            }
        }

        void set_speed(float speed, float frdiff, int frcnt) {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_SET_SPD;
            float params[3];
            params[0] = speed;
            params[1] = frdiff;
            params[2] = frcnt;
            p.len = sizeof(float) * 3;
            for (int i = 0; i < sizeof(float) * 3; i++) {
                p.data[i] = ((uint8_t*) (params))[i];
            }
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Speed set NACK!\n");
            }
        }

        void start_rx() {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_START;
            p.len = 0;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Rx start NACK!\n");
            }
        }

        void stop_rx() {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_STOP;
            p.len = 0;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Rx stop NACK!\n");
            }
        }

        void start_tx() {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_START;
            p.len = 0;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Tx start NACK!\n");
            }
        }

        void start_tx_wait() {
//        printf("Tx start req\n");
            std::lock_guard<std::mutex> lock(tty_mtx);
//        printf("Tx start lock\n");
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_START;
            p.len = 0;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Tx start NACK!\n");
            }
            while (1) {
                int ret = read_packet(p);
                if (ret != 0 || !working) {
                    if (ret == -1 || !working) {
                        return;
                    } else {
                        continue;
                    }
                }
                if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_TRANSMIT_COMPL) {
                    return;
                }
            }
        }

        void stop_tx() {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_STOP;
            p.len = 0;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Tx stop NACK!\n");
            }
        }

        void start_tx_carrier() {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_CARRIER;
            p.len = 0;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Txc start NACK!\n");
            }
        }

        void put_sdr_tx_samples(int16_t *iq_data, int cnt) {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_DATA;
            p.len = cnt * 4;
            for (int i = 0; i < cnt * 4; i++) {
                p.data[i] = ((uint8_t*) iq_data)[i];
            }
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Tx data NACK!\n");
            }
        }

        void put_tx_data(char *data, uint8_t cnt) {
//	    printf("Tx data req\n");
            std::lock_guard<std::mutex> lock(tty_mtx);
//	    printf("Tx data lock\n");
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_DATA;
            p.len = cnt;
            for (int i = 0; i < cnt; i++) {
                p.data[i] = ((uint8_t*) data)[i];
            }
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Tx data NACK!\n");
            }
        }

        int receive_sdr_rx_samples(int16_t *iq_data) {
            // printf("Spls req\n");
            std::lock_guard<std::mutex> lock(tty_mtx);
            // printf("Spls mtx lock\n");
            while (1) {
                pc_packet_interface::pc_packet p;
                if (read_packet(p) != 0 || !working) {
                    return -1;
                }
                if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_SDR_RX_DATA) {
                    for (int i = 0; i < p.len / 4; i++) {
                        iq_data[i * 2] = *((int16_t*) &(p.data)[i * 4]);
                        iq_data[i * 2 + 1] = *((int16_t*) &(p.data)[i * 4 + 2]);
                    }
                    // printf("Spls mtx unlock\n");
                    return p.len / 4;
                }
            }
        }

        int receive_rx_data(char *data, int maxsz) {
//    	 printf("Spls req\n");
            std::lock_guard<std::mutex> lock(tty_mtx);
//		 printf("Spls mtx lock\n");
            while (1) {
                pc_packet_interface::pc_packet p;
                int ret = read_packet(p);
                if (ret != 0 || !working) {
                    return ret;
                }
                if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA) {
                    uint8_t ptypeerrs = p.data[0];
                    uint8_t ptype = (ptypeerrs & 0b11000000) >> 6;
                    uint8_t errs = ptypeerrs & 0b111111;
                    if(ptype == 0) {
                        data[0] = errs;
                        return -10;
                    } else if(ptype == 1) {
                        data[0] = errs;
                        return -11;
                    } else if(ptype == 3) {
                        return -13;
                    } else if(ptype == 2) {
                        uint8_t dlen = p.len - 1;
                        for (int i = 0; i < dlen; i++) {
                            if (i >= maxsz - 1) {
                                break;
                            }
                            data[i] = p.data[i+1];
                        }
                        if(errs == 0) {
                            return dlen;
                        } else if(errs == 2) {
                            return 2000 + dlen;
                        } else {
                            return 1000 + dlen;
                        }
                    }
                }
            }
        }

        void wait_for_tx() {
            std::lock_guard<std::mutex> lock(tty_mtx);
            // printf("Spls mtx lock\n");
            while (1) {
                pc_packet_interface::pc_packet p;
                int ret = read_packet(p);
                if (ret != 0 || !working) {
                    return;
                }
                if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_TRANSMIT_COMPL) {
                    return;
                }
            }
        }

        int read_param(std::string name, void *data, int len) {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_READ;
            p.len = 1 + name.length();
            p.data[0] = name.length();
            for (int i = 0; i < name.length(); i++) {
                p.data[1 + i] = name[i];
            }
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Param read NACK!\n");
                return -1;
            }
            while (1) {
                pc_packet_interface::pc_packet rp;
                int ret = read_packet(rp);
                if (ret != 0 || !working) {
                    return -1;
                }
                if (rp.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_PARAM_DATA) {
                    for (int i = 0; i < rp.len; i++) {
                        if (i >= len - 1) {
                            break;
                        }
                        ((uint8_t*) data)[i] = rp.data[i];
                    }
                    return rp.len;
                }
            }
        }

        void write_param(std::string name, void *data, int len) {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_WRITE;
            p.len = 1 + name.length() + 1 + len;
            p.data[0] = name.length();
            for (int i = 0; i < name.length(); i++) {
                p.data[1 + i] = name[i];
            }
            p.data[name.length() + 1] = len;
            for (int i = 0; i < len; i++) {
                p.data[name.length() + 2 + i] = ((uint8_t*) data)[i];
            }
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Param write NACK!\n");
            }
        }

        void store_params() {
            std::lock_guard<std::mutex> lock(tty_mtx);
            pc_packet_interface::pc_packet p;
            p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_STORE;
            p.len = 0;
            send_packet(p);
            if (!read_packet_ack()) {
                printf("ModemPI: Param store NACK!\n");
            }
        }

        struct termios tty;
        int fd = -1;
        bool working = false;
        int none_flag = 0;
        int RTS_flag = TIOCM_RTS;
        int DTR_flag = TIOCM_DTR;
        int RTSDTR_flag = TIOCM_RTS | TIOCM_DTR;
        uint8_t packet_read_buff[MAX_PC_P_SIZE];
        int packet_read_buff_data = 0;
        int packet_read_buff_pos = 0;
        std::mutex tty_mtx;
        LockingQueue<pc_packet_interface::pc_packet> rx_queue;
        LockingQueue<pc_packet_interface::pc_packet> tx_queue;
        std::thread *tx_thr;
        std::thread *rx_thr;
};

void printHelp() {
    std::cout << "Help: " << std::endl;
    std::cout << "cbmctl - open-source cli program to communicate with and control CB modem" << std::endl;
    std::cout << "Keys: " << std::endl;
    std::cout << "--help                                    - this help" << std::endl;
    std::cout << "--tty <device>                            - select tty device" << std::endl;
    std::cout << "--no-reset                                - don't reset MCU after opening port" << std::endl;
    std::cout << "--mode <bfsk/mfsk/msk>                    - select modulation" << std::endl;
    std::cout << "--speed <bitrate>                         - select bitrate" << std::endl;
    std::cout << "--centerfr <freq>                 		- select center frequency(in hz)" << std::endl;
    std::cout << "--frdiff <frdiff>                       	- for bfsk/mfsk select frequency difference" << std::endl;
    std::cout << "--frcnt <number>                       	- for mfsk select frequency count" << std::endl;
    std::cout << "--receive <number>                    	- start receiving messages, stop after receiving <number>(optional arg)" << std::endl;
    std::cout << "--transmit <text>                     	- transmit text message" << std::endl;
    std::cout << "Interactive mode:" << std::endl;
    std::cout << "	Write text and newline to send it" << std::endl;
    std::cout << "	While not sending, receiving" << std::endl;
    std::cout << "--transmit-carrier                     	- transmit carrier" << std::endl;
    std::cout << "--read-param-int <name>                   - read int32 parameter and print" << std::endl;
    std::cout << "--read-param-float <name>                 - read float parameter and print" << std::endl;
    std::cout << "--write-param-int <name> <value>          - write int32 parameter" << std::endl;
    std::cout << "--write-param-float <name> <value>        - write float parameter" << std::endl;
    std::cout << "(tty argument is requried)" << std::endl;
}

int parseArg(int argc, int *position, char *argv[], std::map<std::string,
        std::string> *params, bool recursive) {
    std::string arg1 = std::string(argv[*position]);
    //i would be using switch() here... but it's not available for strings, so...
    if (arg1 == "--help") {
        return 1;
    } else if (arg1 == "--no-reset") {
        params->insert(std::pair<std::string, std::string>("noReset", "true"));
        return 0;
    } else if (arg1 == "--transmit-carrier") {
        params->insert(std::pair<std::string, std::string>("carrierTx", "true"));
        return 0;
    } else if (arg1 == "--tty") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("tty", arg2));
        return 0;
    } else if (arg1 == "--mode") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("mode", arg2));
        return 0;
    } else if (arg1 == "--speed") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("speed", arg2));
        return 0;
    } else if (arg1 == "--centerfr") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("centerFreq", arg2));
        return 0;
    } else if (arg1 == "--frdiff") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("freqDiff", arg2));
        return 0;
    } else if (arg1 == "--frcnt") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("freqCount", arg2));
        return 0;
    } else if (arg1 == "--receive") {
        std::string arg2;
        arg2 = "-1";
        int nextpos = *position + 1;
        if (nextpos < argc and !recursive) {
            int parseRes = parseArg(argc, &nextpos, argv, params, true);
            if (parseRes == 2) {
                arg2 = std::string(argv[nextpos]);
            }
            *position = nextpos;
        }
        params->insert(std::pair<std::string, std::string>("receive", arg2));
        return 0;
    } else if (arg1 == "--transmit") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("transmit", arg2));
        return 0;
    } else if (arg1 == "--read-param-int") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("readParamInt", arg2));
        return 0;
    } else if (arg1 == "--read-param-float") {
        int nextpos = *position + 1;
        if (nextpos > argc or recursive) {
            return 1;
        }
        int parseRes = parseArg(argc, &nextpos, argv, params, true);
        if (parseRes != 2) {
            return 1;
        }
        *position = nextpos;
        std::string arg2 = std::string(argv[nextpos]);
        params->insert(std::pair<std::string, std::string>("readParamFloat", arg2));
        return 0;
    } else if (arg1 == "--write-param-int") {
        std::string arg2;
        std::string arg3;
        int nextpos = *position + 1;
        if (nextpos < argc and !recursive) {
            int parseRes = parseArg(argc, &nextpos, argv, params, true);
            if (parseRes != 2) {
                return 1;
            }
            arg2 = std::string(argv[nextpos]);
            *position = nextpos;
            nextpos++;
            if (nextpos < argc) {
                parseRes = parseArg(argc, &nextpos, argv, params, true);
                if (parseRes != 2) {
                    return 1;
                }
                arg3 = std::string(argv[nextpos]);
                *position = nextpos;
            } else {
                return 1;
            }
        } else {
            return 1;
        }
        params->insert(std::pair<std::string, std::string>("writeParamInt", arg2));
        params->insert(std::pair<std::string, std::string>("writeParamIntData", arg3));
        return 0;
    } else if (arg1 == "--write-param-float") {
        std::string arg2;
        std::string arg3;
        int nextpos = *position + 1;
        if (nextpos < argc and !recursive) {
            int parseRes = parseArg(argc, &nextpos, argv, params, true);
            if (parseRes != 2) {
                return 1;
            }
            arg2 = std::string(argv[nextpos]);
            *position = nextpos;
            nextpos++;
            if (nextpos < argc) {
                parseRes = parseArg(argc, &nextpos, argv, params, true);
                if (parseRes != 2) {
                    return 1;
                }
                arg3 = std::string(argv[nextpos]);
                *position = nextpos;
            } else {
                return 1;
            }
        } else {
            return 1;
        }
        params->insert(std::pair<std::string, std::string>("writeParamFloat", arg2));
        params->insert(std::pair<std::string, std::string>("writeParamFloatData", arg3));
        return 0;
    } else {
        return 2;
    }
}

ModemPacketInterface modemPI;
std::atomic<bool> interactiveModeRun(true);
std::mutex interactModeMtx;
int interactModeSpd = 1;
std::thread *interactiveModeThread = NULL;

void signal_callback_handler(int signum) {
    std::cout << "Caught signal " << signum << std::endl;
    interactiveModeRun.store(false);
    modemPI.working = false;
//   printf("stopping pi\n");
//   modemPI.stop();
//   printf("stopping thrd\n");
//   if(interactiveModeThread != NULL) {
//       std::fclose(stdin);
//       std::cin.setstate(std::ios::badbit);
//       printf("joining...\n");
//	   interactiveModeThread->join();
//	   delete interactiveModeThread;
//   }
//   printf("exiting\n");
//   exit(signum);
}

void interactive_mode_read() {
    pthread_setname_np(pthread_self(), "interactive");
    std::string buffer;
    struct pollfd pfd;
    pfd.fd = fileno(stdin);
    pfd.events = POLLRDBAND | POLLRDNORM;
    while (interactiveModeRun.load()) {
        int ret = poll(&pfd, 1, 100);
        if (ret > 0 && (pfd.revents & pfd.events)) {
            std::getline(std::cin, buffer);
            int attempts;
            for(attempts = 3; attempts > 0 && interactiveModeRun.load(); attempts--) {
                std::lock_guard<std::mutex> lock(interactModeMtx);
                if (std::cin.bad() || std::cin.fail() || std::cin.eof()) {
                    break;
                }
                printf("TX(%d) | < %s ", buffer.length(), buffer.c_str());
                fflush(stdout);
                modemPI.put_tx_data((char*) buffer.c_str(), buffer.length());
                modemPI.start_tx_wait();
                printf("| SENT ");
                fflush(stdout);
                bool started = false;
                bool waiting = true;
                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                while((waiting || started) && interactiveModeRun.load()) {
                    char data[257];
                    int l = modemPI.receive_rx_data(data, 256);
                    if (l < 0) {
                        if (l == -1) {
                            std::cerr << "RX error!" << std::endl;
                            break;
                        }
                        if(l == -10) {
                            printf("| RX SW %d ", data[0]);
                            fflush(stdout);
                            started = true;
                        }
                        if(l == -11) {
                            printf("| LEN %d ", data[0]);
                            fflush(stdout);
                        }
                        if(l == -13) {
                            printf("| ACK \n");
                            attempts = 0;
                            break;
                        }
                    } else {
                        printf("| NOT ACK \n");
                        started = false;
                        waiting = false;
                    }
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                    uint32_t required_time = (1000000UL / (interactModeSpd)) * 128;
                    if(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() >= required_time && !started) {
                        printf("| NACK \n");
                        waiting = false;
                    }
                }
            }
        }
        if (std::cin.bad() || std::cin.fail() || std::cin.eof()) {
            break;
        }
    }
    interactiveModeRun.store(false);
}

int do_receive() {
    char data[257];
    int l = modemPI.receive_rx_data(data, 256);
    if (l < 0) {
        if (l == -1) {
            std::cerr << "RX error!" << std::endl;
            return -1;
        }
        if(l == -10) {
            printf("RX SW %d ", data[0]);
            fflush(stdout);
        }
        if(l == -11) {
            printf("| LEN %d ", data[0]);
            fflush(stdout);
        }
        if(l == -13) {
            printf("| ACK\n");
        }
        return 0;
    }
    if(l > 2000) {
        printf("| DUP! ");
        l -= 2000;
    }
    if(l > 1000) {
        printf("| BAD CRC! ");
        l -= 1000;
    }
    data[l] = '\0';
    printf("| > %s\n", data);
    return 0;
}

int main(int argc, char **argv) {
    int retval = 0;
    std::map<std::string, std::string> params;
    if (argc < 2) {
        printHelp();
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        int res = parseArg(argc, &i, argv, &params, false);
        if (res == 1 or res == 2) {
            std::cout << "Wrong args!" << std::endl;
            printHelp();
            return 1;
        }
    }
    if (params.find("tty") == params.end()) {
        std::cout << "No TTY selected!" << std::endl;
        printHelp();
        return 1;
    }
    std::string tty = params["tty"];
    bool noReset = (params.find("noReset") == params.end()) ? false : true;
    bool carrierTx = (params.find("carrierTx") == params.end()) ? false : true;
    std::string mode =
            (params.find("mode") == params.end()) ? "none" : params["mode"];
    char *n;
    int speed =
            (params.find("speed") == params.end()) ? -1 : std::strtol(params["speed"].c_str(), &n, 10);
    int centerFreq =
            (params.find("centerFreq") == params.end()) ? -1 : std::strtol(params["centerFreq"].c_str(), &n, 10);
    int freqDiff =
            (params.find("freqDiff") == params.end()) ? -1 : std::strtol(params["freqDiff"].c_str(), &n, 10);
    int frcnt =
            (params.find("freqCount") == params.end()) ? -1 : std::strtol(params["freqCount"].c_str(), &n, 10);
    int receive =
            (params.find("receive") == params.end()) ? -2 : std::strtol(params["receive"].c_str(), &n, 10);
    std::string transmit =
            (params.find("transmit") == params.end()) ? "" : params["transmit"];
    float fr = centerFreq;

    if (!modemPI.start(tty)) {
        std::cerr << "Start failed!" << std::endl;
        retval = 1;
        goto _end;
    }
    if ((params.find("readParamInt") != params.end())) {
        std::string paramName = params["readParamInt"];
        char data[256];
        int len = modemPI.read_param(paramName, data, 255);
        if (len != 4) {
            printf("Wrong data!\n");
            retval = 1;
            goto _end;
        }
        int32_t ret = *((int32_t*) data);
        printf("Value: %d\n", ret);
        goto _end;
    } else if ((params.find("readParamFloat") != params.end())) {
        std::string paramName = params["readParamFloat"];
        char data[256];
        int len = modemPI.read_param(paramName, data, 255);
        if (len != 4) {
            printf("Wrong data!\n");
            retval = 1;
            goto _end;
        }
        float ret = *((float*) data);
        printf("Value: %f\n", ret);
        goto _end;
    } else if ((params.find("writeParamInt") != params.end())) {
        std::string paramName = params["writeParamInt"];
        int32_t paramVal = std::strtol(params["writeParamIntData"].c_str(), &n, 10);
        modemPI.write_param(paramName, (uint8_t*) &paramVal, 4);
        modemPI.store_params();
        printf("Value written: %d\n", paramVal);
        goto _end;
    } else if ((params.find("writeParamFloat") != params.end())) {
        std::string paramName = params["writeParamFloat"];
        float paramVal = std::strtof(params["writeParamFloatData"].c_str(), &n);
        modemPI.write_param(paramName, (uint8_t*) &paramVal, 4);
        modemPI.store_params();
        printf("Value written: %f\n", paramVal);
        goto _end;
    }

    pc_packet_interface::modes newmode;
    if (carrierTx || receive == -3) {
        newmode = pc_packet_interface::modes::PC_PI_MODE_SDR;
    } else if (mode == "bfsk") {
        newmode = pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK;
        if (speed == -1 || freqDiff == -1) {
            std::cerr << "Invalid speed/frdiff!" << std::endl;
            retval = 1;
            goto _end;
        }
    } else if (mode == "mfsk") {
        newmode = pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK;
        if (speed == -1 || freqDiff == -1 || frcnt == -1) {
            std::cerr << "Invalid speed/frdiff/frcnts!" << std::endl;
            retval = 1;
            goto _end;
        }
    } else if (mode == "msk") {
        newmode = pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK;
        if (speed == -1) {
            std::cerr << "Invalid speed!" << std::endl;
            retval = 1;
            goto _end;
        }
    } else {
        std::cerr << "Invalid mode!" << std::endl;
        retval = 1;
        goto _end;
    }
    if(fr == -1) {
        std::cerr << "Invalid freq!" << std::endl;
        retval = 1;
        goto _end;
    }
    modemPI.change_mode(newmode, !noReset);
    modemPI.set_speed(speed, freqDiff, frcnt);
    interactModeSpd = speed;
    modemPI.set_fr(fr);
    signal(SIGINT, signal_callback_handler);

    if (carrierTx) {
        modemPI.start_tx_carrier();
        while(interactiveModeRun.load()) {
            usleep(500);
        }
    } else if (receive != -2) {
        modemPI.start_rx();
        if (receive == -1) {
            while (interactiveModeRun.load()) {
                int ret = do_receive();
                if(ret == -1) {
                    retval = 1;
                    goto _end;
                }
            }
        } else if (receive == -3) {
            modemPI.start_rx();
            std::ofstream wf("data.bin", std::ios::out | std::ios::binary);
            while (interactiveModeRun.load()) {
                int16_t in_buff[64];
                int r = modemPI.receive_sdr_rx_samples(in_buff);
                if (r < 1) {
                    printf("ModemTestSourceModule: Samples read failed!\n");
                    retval = 1;
                    goto _end;
                }
                for (int i = 0; i < r; i++) {
                    float real = in_buff[i * 2] * (1.0f / 32767.0f) * 10.0f;
                    float imag = in_buff[i * 2 + 1] * (1.0f / 32767.0f) * 10.0f;
                    wf.write((char*) &real, sizeof(float));
                    wf.write((char*) &imag, sizeof(float));
                    wf.flush();
                }
            }
        } else {
            int x = 0;
            while (x < receive && interactiveModeRun.load()) {
                int ret = do_receive();
                if(ret == -1) {
                    retval = 1;
                    goto _end;
                }
                x++;
            }
        }
        modemPI.stop_rx();
    } else if (transmit.length() != 0) {
        modemPI.put_tx_data((char*) transmit.c_str(), transmit.length());
        modemPI.start_tx_wait();
    } else {
        //Interactive mode
        interactiveModeRun.store(true);
        interactiveModeThread = new std::thread(interactive_mode_read);
        modemPI.start_rx();
        while (interactiveModeRun.load()) {
            {
                std::lock_guard<std::mutex> lock(interactModeMtx);
                int ret = do_receive();
                if(ret == -1) {
                    retval = 1;
                    goto _end;
                }
            }
            usleep(500);

        }
        modemPI.stop_rx();
        std::fclose(stdin);
        std::cin.setstate(std::ios::badbit);
        interactiveModeThread->join();
        delete interactiveModeThread;
    }
_end:
    modemPI.stop();
    return retval;
}
