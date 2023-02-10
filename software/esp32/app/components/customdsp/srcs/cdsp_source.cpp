#include "cdsp_source.h"

cdsp_src_adc::cdsp_src_adc(chl_i2sanalog* adc_dev, int adc_i_ch, int adc_q_ch, int adc_bitwidth) {
    _adc_dev = adc_dev;
    _adc_bitwidth = adc_bitwidth;
    _convert_divisor = 2.0f / (float)(1 << _adc_bitwidth);
    _adc_dev->stopRx();
    _adc_dev->clearRxBuffers();
    _adc_i_ch = adc_i_ch;
    _adc_q_ch = adc_q_ch;
    _adc_dev->configureADC(adc_i_ch, adc_q_ch, _adc_bitwidth, ADC_ATTEN); 
}

void cdsp_src_adc::setAdcChannels(int adc_i_ch, int adc_q_ch) {
    _adc_i_ch = adc_i_ch;
    _adc_q_ch = adc_q_ch;
    _adc_dev->configureADC(adc_i_ch, adc_q_ch, _adc_bitwidth, ADC_ATTEN);
}

int cdsp_src_adc::requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_src_adc* _this = (cdsp_src_adc*) ctx;
    if(!_this->_running) {return -2;}
    int req_data = std::min(samples_cnt, CDSP_DEF_BUFF_SIZE);
    int got = 0;
    // printf("r1\n");
    got = _this->_adc_dev->read_samples(_this->_in_buf, req_data, CDSP_SOURCE_TIMEOUT);
    // printf("r2 %d\n", got);
    if(got <= 0) {return got;}
    for(int i = 0; i < got; i++) {
        if(_this->_in_buf[i].channelI == _this->_adc_i_ch) {
            data[i].i = ((float)_this->_in_buf[i].dataI * _this->_convert_divisor) - 1.0f;
        } else if(_this->_in_buf[i].channelQ == _this->_adc_i_ch) {
            data[i].i = ((float)_this->_in_buf[i].dataQ * _this->_convert_divisor) - 1.0f;
        }
        if(_this->_in_buf[i].channelQ == _this->_adc_q_ch) {
            data[i].q = ((float)_this->_in_buf[i].dataQ * _this->_convert_divisor) - 1.0f;
        } else if(_this->_in_buf[i].channelI == _this->_adc_q_ch) {
            data[i].q = ((float)_this->_in_buf[i].dataI * _this->_convert_divisor) - 1.0f;
        }
    }
    return got;
}

void cdsp_src_adc::_do_start() {
    _adc_dev->clearRxBuffers();
    _adc_dev->startRx();
}

void cdsp_src_adc::_do_stop() {
    _adc_dev->stopRx();
}
