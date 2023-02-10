#pragma once
#include <math.h>
#include "cdsp_common.h"
#include "chl_i2sanalog.h"

#define CDSP_SOURCE_TIMEOUT 100 / portTICK_PERIOD_MS

//Static 11dB atten(max voltage range)
#define ADC_ATTEN 11

//Only ADC1 available, output type=cdsp_complex_t
class cdsp_src_adc : public cdsp_block<void, cdsp_complex_t> {
public:
    cdsp_src_adc(chl_i2sanalog* adc_dev, int adc_i_ch, int adc_q_ch, int adc_bitwidth);
    void setAdcChannels(int adc_i_ch, int adc_q_ch);
    static int IRAM_ATTR requestData(void* ctx, cdsp_complex_t* data, int samples_cnt);

private:
    int _adc_bitwidth;
    float _convert_divisor;
    int _adc_i_ch, _adc_q_ch;
    chl_i2sanalog* _adc_dev;
    chl_i2sanalog_type1 _in_buf[CDSP_DEF_BUFF_SIZE];

    void _do_start() override;
    void _do_stop() override;
};
