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

#include <libcbm.h>

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

int parseArg(int argc, int *position, char *argv[], std::map<std::string,std::string> *params, bool recursive) {
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

cbmodem::ModemPacketInterface modemPI;
std::atomic<bool> interactiveModeRun(true);
std::mutex interactModeMtx;
int interactModeSpd = 1;
std::thread *interactiveModeThread = NULL;
std::thread *RX2Thread = NULL;

void signal_callback_handler(int signum) {
    std::cout << "Caught signal " << signum << std::endl;
    interactiveModeRun.store(false);
    // modemPI.working = false;
    modemPI.stop();
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

void rx2_read() {
    pthread_setname_np(pthread_self(), "rx2");
    std::string buffer;
    while (interactiveModeRun.load()) {
        char c;
        int r = modemPI.read_rx2_char(c);
        if(r == 0) {
            fprintf(stderr, "%c", c);
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
    if(l > RX_DATA_RETSHIFT_DUP) {
        printf("| DUP! ");
        l -= RX_DATA_RETSHIFT_DUP;
    }
    if(l > RX_DATA_RETSHIFT_BADCRC) {
        printf("| BAD CRC! ");
        l -= RX_DATA_RETSHIFT_BADCRC;
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

    modemPI.init(tty);
    if (!modemPI.start()) {
        std::cerr << "Start failed!" << std::endl;
        retval = 1;
        goto _end;
    }
    // if(!noReset) {
    //     modemPI.reset_mcu();
    //     usleep(500000UL);
    // }
    RX2Thread = new std::thread(rx2_read);
    if ((params.find("readParamInt") != params.end())) {
        std::string paramName = params["readParamInt"];
        char data[256];
        int len = modemPI.read_param(paramName, data, 255);
        if (len != 4) {
            printf("Value not set/default!\n");
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
            printf("Value not set/default!\n");
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

    cbmodem::pc_packet_interface::modes newmode;
    if (carrierTx || receive == -3) {
        newmode = cbmodem::pc_packet_interface::modes::PC_PI_MODE_SDR;
    } else if (mode == "bfsk") {
        newmode = cbmodem::pc_packet_interface::modes::PC_PI_MODE_NORMAL_BFSK;
        if (speed == -1 || freqDiff == -1) {
            std::cerr << "Invalid speed/frdiff!" << std::endl;
            retval = 1;
            goto _end;
        }
    } else if (mode == "mfsk") {
        newmode = cbmodem::pc_packet_interface::modes::PC_PI_MODE_NORMAL_MFSK;
        if (speed == -1 || freqDiff == -1 || frcnt == -1) {
            std::cerr << "Invalid speed/frdiff/frcnts!" << std::endl;
            retval = 1;
            goto _end;
        }
    } else if (mode == "msk") {
        newmode = cbmodem::pc_packet_interface::modes::PC_PI_MODE_NORMAL_MSK;
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
