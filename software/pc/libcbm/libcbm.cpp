#include "libcbm.h"

namespace cbmodem {

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

    template<typename T>
    ModemPILockingQueue<T>::~ModemPILockingQueue() {
        setWorking(false);
    }

    template<typename T>
    bool ModemPILockingQueue<T>::push(T const &_data) {
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

    template<typename T>
    bool ModemPILockingQueue<T>::empty() {
        std::lock_guard<std::mutex> lock(guard);
        return queue.empty() || !working;
    }

    template<typename T>
    size_t ModemPILockingQueue<T>::size() {
        std::lock_guard<std::mutex> lock(guard);
        return queue.size();
    }

    template<typename T>
    int ModemPILockingQueue<T>::tryPop(T &_value) {
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

    template<typename T>
    bool ModemPILockingQueue<T>::waitAndPop(T &_value) {
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

    template<typename T>
    int ModemPILockingQueue<T>::tryWaitAndPop(T &_value, int _milli) {
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

    template<typename T>
    void ModemPILockingQueue<T>::setWorking(bool newwork) {
        working = newwork;
        signal.notify_one();
        signal.notify_all();
        signal.notify_all();
        guard.unlock();
    }

    template class ModemPILockingQueue<char>;
    template class ModemPILockingQueue<pc_packet_interface::pc_packet>;

    ModemPacketInterface::~ModemPacketInterface() {
        stop();
    }


    void ModemPacketInterface::init(std::string serial_dev) {
        stop();
        _serial_dev = serial_dev;
    }

    bool ModemPacketInterface::start() {
        stop();
        _fd = open(_serial_dev.c_str(), O_RDWR);
        if (_fd < 0) {
            fprintf(stderr, "ModemPI: Port open failed: %s\n", strerror(errno));
            return false;
        }
        if (tcgetattr(_fd, &(_tty)) != 0) {
            fprintf(stderr, "ModemPI: tcgetattr() failed! Error: %s\n", strerror(errno));
            close(_fd);
            return false;
        }
        _tty.c_cflag &= ~CRTSCTS;   // Disable RTS/CTS control flow
        _tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
        _tty.c_lflag &= ~ICANON;
        _tty.c_lflag &= ~ECHO; // Disable echo
        _tty.c_lflag &= ~ECHOE; // Disable erasure
        _tty.c_lflag &= ~ECHONL; // Disable new-line echo
        _tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
        _tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
        _tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytess
        _tty.c_cc[VTIME] = READ_TIMEOUT_MS/100;
        _tty.c_cc[VMIN] = 0;
        if (cfsetspeed(&(_tty), B115200)) {
            fprintf(stderr, "ModemPI: cfsetispeed() failed! Error: %s\n", strerror(errno));
            close(_fd);
            return false;
        }
        if (tcsetattr(_fd, TCSANOW, &(_tty)) != 0) {
            fprintf(stderr, "ModemPI: tcsetattr() failed! Error: %s\n", strerror(errno));
            close(_fd);
            return false;
        }
        usleep(500);
        ioctl(_fd, TCFLSH, 2); // flush R and T buffs
        _tty_mtx.unlock();
        _rx_queue.setWorking(true);
        _rx2_queue.setWorking(true);
        _tx_queue.setWorking(true);
        _tx_thr = new std::thread(_tx_thread_func, this);
        _rx_thr = new std::thread(_rx_thread_func, this);
        _working = true;
        return true;
    }

    void ModemPacketInterface::stop() {
        _tty_mtx.unlock();
        _working = false;

        if (_fd >= 0) {
            ioctl(_fd, TCFLSH, 2); // flush R and T buffs
            int oldfd = _fd;
            if(_rx_thr != 0 && _tx_thr != 0 && _rx_thr->joinable() && _tx_thr->joinable()) {
               uint8_t newmode = pc_packet_interface::modes::PC_PI_MODE_UNINITED;
               pc_packet_interface::pc_packet startp;
               startp.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_CHANGE_MODE;
               startp.len = 1;
               startp.data[0] = newmode;
               _send_packet(startp);
               _read_packet_ack();
            }
            ioctl(_fd, TCFLSH, 2); // flush R and T buffs
            _fd = -1;
            _rx_queue.setWorking(false);
            _rx2_queue.setWorking(false);
            _tx_queue.setWorking(false);
            if(_rx_thr != 0 && _rx_thr->joinable()) {
                _rx_thr->join();
            }
            delete _rx_thr;
            if(_tx_thr != 0 &&_tx_thr->joinable()) {
                _tx_thr->join();
            }
            delete _tx_thr;
            _rx_thr = 0;
            _tx_thr = 0;
            ioctl(oldfd, TCFLSH, 2); // flush R and T buffs
            close(oldfd);
        }
    }

    void ModemPacketInterface::reset_mcu() {
        ioctl(_fd, TIOCMSET, &_RTS_flag);
        usleep(500UL * 1000UL);
        ioctl(_fd, TIOCMSET, &_none_flag);
    }

    void ModemPacketInterface::change_mode(pc_packet_interface::modes newmode, bool reset) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        usleep(500);
        ioctl(_fd, TCFLSH, 2); // flush R and T buffs
        if (reset) {
            reset_mcu();
            int ctr = 0;
            while (1) {
                pc_packet_interface::pc_packet p;
                int ret = _read_packet(p);
                if (ret == -1 || ctr >= 20 || !_working) {
                    fprintf(stderr, "ModemPI: Init packet read failed!\n");
                    return;
                } else if(ret == -2) {
                    ctr++;
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
        _send_packet(modep);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Mode set NACK!\n");
        }
    }

    void ModemPacketInterface::set_fr(float fr) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_SET_FR;
        p.len = sizeof(float);
        for (int i = 0; i < sizeof(float); i++) {
            p.data[i] = ((uint8_t*) (&fr))[i];
        }
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Fr set NACK!\n");
        }
    }

    void ModemPacketInterface::set_inputs(uint8_t hi) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_SET_IN_SRC;
        p.len = 1;
        p.data[0] = hi;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Ins set NACK!\n");
        }
    }

    void ModemPacketInterface::set_speed(float speed, float frdiff, int frcnt) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
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
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Speed set NACK!\n");
        }
    }

    void ModemPacketInterface::start_rx() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_START;
        p.len = 0;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Rx start NACK!\n");
        }
    }

    void ModemPacketInterface::stop_rx() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_RX_STOP;
        p.len = 0;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Rx stop NACK!\n");
        }
    }

    void ModemPacketInterface::start_tx() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_START;
        p.len = 0;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Tx start NACK!\n");
        }
    }

    void ModemPacketInterface::start_tx_wait() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_START;
        p.len = 0;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Tx start NACK!\n");
            return;
        }
        int ctr = 0;
        while (ctr <= 100) {
            int ret = _read_packet(p);
            if (ret != 0 || !_working) {
                if (ret == -1 || !_working) {
                    return;
                } else {
                    ctr++;
                    continue;
                }
            }
            if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_TRANSMIT_COMPL) {
                return;
            }
        }
        return;
    }

    void ModemPacketInterface::stop_tx() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_STOP;
        p.len = 0;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Tx stop NACK!\n");
        }
    }

    void ModemPacketInterface::start_tx_carrier() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_CARRIER;
        p.len = 0;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Txc start NACK!\n");
        }
    }

    void ModemPacketInterface::put_sdr_tx_samples(int16_t *iq_data, int cnt) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_DATA;
        p.len = cnt * 4;
        for (int i = 0; i < cnt * 4; i++) {
            p.data[i] = ((uint8_t*) iq_data)[i];
        }
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Tx data NACK!\n");
        }
    }

    void ModemPacketInterface::put_tx_data(char *data, uint8_t cnt) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_TX_DATA;
        p.len = cnt;
        for (int i = 0; i < cnt; i++) {
            p.data[i] = ((uint8_t*) data)[i];
        }
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Tx data NACK!\n");
        }
    }

    int ModemPacketInterface::receive_sdr_rx_samples(int16_t *iq_data) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        while (1) {
            pc_packet_interface::pc_packet p;
            if (_read_packet(p) != 0 || !_working) {
                return -1;
            }
            if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_SDR_RX_DATA) {
                for (int i = 0; i < p.len / 4; i++) {
                    iq_data[i * 2] = *((int16_t*) &(p.data)[i * 4]);
                    iq_data[i * 2 + 1] = *((int16_t*) &(p.data)[i * 4 + 2]);
                }
                return p.len / 4;
            }
        }
    }

    int ModemPacketInterface::receive_rx_data(char *data, int maxsz) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        while (1) {
            pc_packet_interface::pc_packet p;
            int ret = _read_packet(p);
            if (ret != 0 || !_working) {
                return ret;
            }
            if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_RX_DATA) {
                uint8_t ptypeerrs = p.data[0];
                uint8_t ptype = (ptypeerrs & 0b11000000) >> 6;
                uint8_t errs = ptypeerrs & 0b111111;
                if(ptype == 0) {
                    data[0] = errs;
                    return RX_DATA_RET_SW;
                } else if(ptype == 1) {
                    data[0] = errs;
                    return RX_DATA_RET_LEN;
                } else if(ptype == 3) {
                    return RX_DATA_RET_ACK;
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
                        return RX_DATA_RETSHIFT_DUP + dlen;
                    } else {
                        return RX_DATA_RETSHIFT_BADCRC + dlen;
                    }
                }
            }
        }
    }

    void ModemPacketInterface::wait_for_tx() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        while (1) {
            pc_packet_interface::pc_packet p;
            int ret = _read_packet(p);
            if (ret != 0 || !_working) {
                return;
            }
            if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_N_TRANSMIT_COMPL) {
                return;
            }
        }
    }

    int ModemPacketInterface::read_param(std::string name, void *data, int len) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_READ;
        p.len = 1 + name.length();
        p.data[0] = name.length();
        for (int i = 0; i < name.length(); i++) {
            p.data[1 + i] = name[i];
        }
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Param read NACK!\n");
            return -1;
        }
        while (1) {
            pc_packet_interface::pc_packet rp;
            int ret = _read_packet(rp);
            if (ret != 0 || !_working) {
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

    void ModemPacketInterface::write_param(std::string name, void *data, int len) {
        std::lock_guard<std::mutex> lock(_tty_mtx);
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
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Param write NACK!\n");
        }
    }

    void ModemPacketInterface::store_params() {
        std::lock_guard<std::mutex> lock(_tty_mtx);
        pc_packet_interface::pc_packet p;
        p.type = pc_packet_interface::packetType_frompc::PC_PI_PTP_PARAM_STORE;
        p.len = 0;
        _send_packet(p);
        if (!_read_packet_ack()) {
            fprintf(stderr, "ModemPI: Param store NACK!\n");
        }
    }

    int ModemPacketInterface::read_rx2_char(char &c) {
        int ret = _rx2_queue.tryWaitAndPop(c, READ_TIMEOUT_MS);
        return ret;
    }

    void ModemPacketInterface::_rx_thread_func(void *ctx) {
        ModemPacketInterface *_this = (ModemPacketInterface*) ctx;
        pthread_setname_np(pthread_self(), "modemPI_rx");
        int state = 0; //0-receiving startByte; 1-receiving type; 2-receiving len; 3-receiving data; 4-receiving checksum
        int outdata_pos = 0;
        pc_packet_interface::pc_packet readp;
        uint8_t checksum_buff[sizeof(pc_packet_interface::pc_packet)];
        checksum_buff[0] = pc_packet_interface::startByte;
        while (_this->_fd >= 0) {
            while (_this->_packet_read_buff_data > 0) {
                if (state == 0) {
                    if (_this->_packet_read_buff[_this->_packet_read_buff_pos] == pc_packet_interface::startByte) {
                        state = 1;
                    } else {
                        if(_this->_packet_read_buff[_this->_packet_read_buff_pos] != 0x00) {
                            if ((_this->_packet_read_buff[_this->_packet_read_buff_pos] < 0x20 || _this->_packet_read_buff[_this->_packet_read_buff_pos] > 0x7F) && _this->_packet_read_buff[_this->_packet_read_buff_pos] != '\n' && _this->_packet_read_buff[_this->_packet_read_buff_pos] != '\r') {
                                if(_this->_rx2_queue.size() <= RX2_BYTES_LIMIT) {
                                    std::string s = std::format("[{:x}]", _this->_packet_read_buff[_this->_packet_read_buff_pos]);
                                    for(int i = 0; i < s.size(); i++) {
                                        _this->_rx2_queue.push(s[i]);
                                    }
                                }
                            } else {
                                if(_this->_rx2_queue.size() <= RX2_BYTES_LIMIT) {
                                    _this->_rx2_queue.push(_this->_packet_read_buff[_this->_packet_read_buff_pos]);
                                }
                            }
                        }
                    }
                } else if (state == 1) {
                    readp.type = _this->_packet_read_buff[_this->_packet_read_buff_pos];
                    checksum_buff[1] = readp.type;
                    state = 2;
                } else if (state == 2) {
                    readp.len = _this->_packet_read_buff[_this->_packet_read_buff_pos];
                    checksum_buff[2] = readp.len;
                    if (readp.len == 0) {
                        state = 4;
                    } else {
                        state = 3;
                    }
                } else if (state == 3) {
                    readp.data[outdata_pos] = _this->_packet_read_buff[_this->_packet_read_buff_pos];
                    checksum_buff[3+outdata_pos] = readp.data[outdata_pos];
                    outdata_pos++;
                    if (outdata_pos >= readp.len) {
                        state = 4;
                    }
                } else if (state == 4) {
                    readp.checksum = _this->_packet_read_buff[_this->_packet_read_buff_pos];
                    uint8_t calc_checksum = calc_crc8(checksum_buff, 3+outdata_pos);
                    _this->_packet_read_buff_pos++;
                    _this->_packet_read_buff_data--;
                    state = 0;
                    outdata_pos = 0;
                    if(calc_checksum == readp.checksum) {
                        if (!_this->_rx_queue.push(readp)) {
                            _this->_rx_queue.setWorking(false);
                            goto _end;
                        }
                    } else {
                        fprintf(stderr, "ModemPI: packet RX host CRC error(%x %x %d %d)!\n", calc_checksum, readp.checksum, readp.len, 3+outdata_pos);
                    }
                    //Read success
                }
                _this->_packet_read_buff_pos++;
                _this->_packet_read_buff_data--;
            }
            int r = read(_this->_fd, _this->_packet_read_buff, 4);
            if (r < 0) {
                if (_this->_fd >= 0) {
                    fprintf(stderr, "ModemPI: serial read failed! Error: %s\n", strerror(r));
                }
                break;
            }
            _this->_packet_read_buff_data = r;
            _this->_packet_read_buff_pos = 0;
        }
    _end:
        _this->_rx_queue.setWorking(false);
        _this->_rx2_queue.setWorking(false);
    }

    void ModemPacketInterface::_tx_thread_func(void *ctx) {
        ModemPacketInterface *_this = (ModemPacketInterface*) ctx;
        pthread_setname_np(pthread_self(), "modemPI_tx");
        pc_packet_interface::pc_packet sendp;
        while (1) {
            if (!_this->_tx_queue.waitAndPop(sendp)) {
                goto _end;
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
            if (_this->_fd < 0) {
                goto _end;
            }
            // fprintf(stderr, "SENDPKT T: %d, CRC: %x, LEN: %d : [", sendp.type, packet_send_buff[4 + sendp.len], sendp.len);
            // write(_this->_fd, &packet_send_buff[0], bufs);
            // fsync(_this->_fd);
            write(_this->_fd, &packet_send_buff[0], 1);
            fsync(_this->_fd);
            usleep(1000UL); //don't remember, why this is needed. probably can be removed. NO!!!!!!!!!
            // fprintf(stderr, "%x, ", packet_send_buff[0]);
            write(_this->_fd, &packet_send_buff[1], 1);
            fsync(_this->_fd);
            usleep(1000UL);
            // fprintf(stderr, "%x, ", packet_send_buff[1]);

            for(int i = 2; i < bufs; i+=16) {
                write(_this->_fd, &packet_send_buff[i], std::min(16, bufs-i));
                fsync(_this->_fd);
                usleep(5000UL);
            }
            // for(int i = 2; i < bufs; i++) {
            //     write(_this->_fd, &packet_send_buff[i], 1);
            //     fsync(_this->_fd);
            //     // usleep(500UL);
            //     // fprintf(stderr, "%x, ", packet_send_buff[i]);
            // }
            // fprintf(stderr, "]\n");
        }
    _end:
        _this->_tx_queue.setWorking(false);
    }

    int ModemPacketInterface::_read_packet(pc_packet_interface::pc_packet &p) {
        int ret = _rx_queue.tryWaitAndPop(p, READ_TIMEOUT_MS);
        return ret;
    }

    bool ModemPacketInterface::_send_packet(pc_packet_interface::pc_packet &p) {
        if (!_tx_queue.push(p)) {
            return -1;
        }
        return 0;
    }

    uint8_t ModemPacketInterface::_read_packet_ack() {
        int ctr = 0;
        while (ctr < 50) {
            pc_packet_interface::pc_packet p;
            if (_read_packet(p) != 0) {
                fprintf(stderr, "ModemPI: Ack packet read failed!\n");
                return 0;
            }
            if (p.type == pc_packet_interface::packetType_fromdev::PC_PI_PTD_ACK) {
                return p.data[0];
            }
            ctr++;
        }
        return 0; //too many packets; ack probably missed
    }
}
