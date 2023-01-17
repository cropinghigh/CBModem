#include "i2sanalog.h"

void test_print_regs() {
    printf("i2s regs------------------------------\r\n");
    printf("I2S_CONF_REG:                 %#lx\r\n", REG_READ(I2S_CONF_REG(0)));
    printf("I2S_CONF1_REG:                %#lx\r\n", REG_READ(I2S_CONF1_REG(0)));
    printf("I2S_CONF2_REG:                %#lx\r\n", REG_READ(I2S_CONF2_REG(0)));
    printf("I2S_TIMING_REG:               %#lx\r\n", REG_READ(I2S_TIMING_REG(0)));
    printf("I2S_FIFO_CONF_REG:            %#lx\r\n", REG_READ(I2S_FIFO_CONF_REG(0)));
    printf("I2S_CONF_SIGLE_DATA_REG :     %#lx\r\n", REG_READ(I2S_CONF_SIGLE_DATA_REG (0)));
    printf("I2S_CONF_CHAN_REG:            %#lx\r\n", REG_READ(I2S_CONF_CHAN_REG(0)));
    printf("I2S_LC_HUNG_CONF_REG:         %#lx\r\n", REG_READ(I2S_LC_HUNG_CONF_REG(0)));
    printf("I2S_CLKM_CONF_REG:            %#lx\r\n", REG_READ(I2S_CLKM_CONF_REG(0)));
    printf("I2S_SAMPLE_RATE_CONF_REG:     %#lx\r\n", REG_READ(I2S_SAMPLE_RATE_CONF_REG(0)));
    printf("I2S_PD_CONF_REG:              %#lx\r\n", REG_READ(I2S_PD_CONF_REG(0)));
    printf("I2S_STATE_REG:                %#lx\r\n", REG_READ(I2S_STATE_REG(0)));
    printf("I2S_LC_CONF_REG:              %#lx\r\n", REG_READ(I2S_LC_CONF_REG(0)));
    printf("I2S_RXEOF_NUM_REG:            %#lx\r\n", REG_READ(I2S_RXEOF_NUM_REG(0)));
    printf("I2S_OUT_LINK_REG:             %#lx\r\n", REG_READ(I2S_OUT_LINK_REG(0)));
    printf("I2S_IN_LINK_REG:              %#lx\r\n", REG_READ(I2S_IN_LINK_REG(0)));
    printf("I2S_OUT_EOF_DES_ADDR_REG:     %#lx\r\n", REG_READ(I2S_OUT_EOF_DES_ADDR_REG(0)));
    printf("I2S_IN_EOF_DES_ADDR_REG:      %#lx\r\n", REG_READ(I2S_IN_EOF_DES_ADDR_REG(0)));
    printf("I2S_OUT_EOF_BFR_DES_ADDR_REG: %#lx\r\n", REG_READ(I2S_OUT_EOF_BFR_DES_ADDR_REG(0)));
    printf("I2S_INLINK_DSCR_REG:          %#lx\r\n", REG_READ(I2S_INLINK_DSCR_REG(0)));
    printf("I2S_INLINK_DSCR_BF0_REG:      %#lx\r\n", REG_READ(I2S_INLINK_DSCR_BF0_REG(0)));
    printf("I2S_INLINK_DSCR_BF1_REG:      %#lx\r\n", REG_READ(I2S_INLINK_DSCR_BF1_REG(0)));
    printf("I2S_OUTLINK_DSCR_REG:         %#lx\r\n", REG_READ(I2S_OUTLINK_DSCR_REG(0)));
    printf("I2S_OUTLINK_DSCR_BF0_REG:     %#lx\r\n", REG_READ(I2S_OUTLINK_DSCR_BF0_REG(0)));
    printf("I2S_OUTLINK_DSCR_BF1_REG:     %#lx\r\n", REG_READ(I2S_OUTLINK_DSCR_BF1_REG(0)));
    printf("I2S_LC_STATE0_REG:            %#lx\r\n", REG_READ(I2S_LC_STATE0_REG(0)));
    printf("I2S_LC_STATE1_REG:            %#lx\r\n", REG_READ(I2S_LC_STATE1_REG(0)));
    printf("I2S_PDM_CONF_REG:             %#lx\r\n", REG_READ(I2S_PDM_CONF_REG(0)));
    printf("I2S_PDM_FREQ_CONF_REG:        %#lx\r\n", REG_READ(I2S_PDM_FREQ_CONF_REG(0)));
    printf("I2S_INT_RAW_REG:              %#lx\r\n", REG_READ(I2S_INT_RAW_REG(0)));
    printf("I2S_INT_ST_REG:               %#lx\r\n", REG_READ(I2S_INT_ST_REG(0)));
    printf("I2S_INT_ENA_REG:              %#lx\r\n", REG_READ(I2S_INT_ENA_REG(0)));
    printf("--------------------------------------\r\n");
}

chl_i2sanalog::chl_i2sanalog() {
    _chl_i2sanalog_dma_rx_buff = (chl_i2sanalog_type1*)heap_caps_malloc(sizeof(chl_i2sanalog_type1)*DMA_I2SANALOG_RX_LINK_BUFF_CNT, MALLOC_CAP_DMA);
    if(_chl_i2sanalog_dma_rx_buff == NULL) {
        printf("I2S0 DMA ERROR: NOT ENOUGH MEMORY FOR RX BUFFER\r\n");
        return;
    }
    _chl_i2sanalog_dma_inlinks = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t)*2, MALLOC_CAP_DMA);
    if(_chl_i2sanalog_dma_inlinks == NULL) {
        printf("I2S0 DMA ERROR: NOT ENOUGH MEMORY FOR RX INPUT LINKS\r\n");
        return;
    }
    _chl_i2sanalog_dma_outlinks = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t)*2, MALLOC_CAP_DMA);
    if(_chl_i2sanalog_dma_outlinks == NULL) {
        printf("I2S0 DMA ERROR: NOT ENOUGH MEMORY FOR TX OUTPUT LINKS\r\n");
        return;
    }
    _rx_streambuffer_mtx = xSemaphoreCreateMutex();
    if(_rx_streambuffer_mtx == NULL) {
        printf("I2S0 DMA ERROR: NOT ENOUGH MEMORY FOR RX STREAM BUFFER MUTEX\r\n");
        return;
    }
    _tx_links_mtx = xSemaphoreCreateMutex();
    if(_tx_links_mtx == NULL) {
        printf("I2S0 DMA ERROR: NOT ENOUGH MEMORY FOR TX LINKS MUTEX\r\n");
        return;
    }
    _xRxStreamBuffer = xStreamBufferCreate(sizeof(chl_i2sanalog_type1)*DMA_I2SANALOG_RX_STREAM_CNT, sizeof(chl_i2sanalog_type1));
    if(_xRxStreamBuffer == NULL) {
        printf("I2S0 DMA ERROR: NOT ENOUGH MEMORY FOR RX STREAM BUFFER\r\n");
        return;
    }
    periph_module_enable(PERIPH_I2S0_MODULE);
    periph_module_enable(PERIPH_SARADC_MODULE);
    periph_rtc_apll_acquire();
    _reset_module();
    if(esp_intr_alloc(ETS_I2S0_INTR_SOURCE, ESP_INTR_FLAG_IRAM, _i2sanalog_intr_hdlr, this, &_i2sanalog_intr_hdl) != ESP_OK) {
        printf("I2S0 ERROR: ESP INTR ALLOC FAIL\r\n");
        return;
    }
    _tx_links_task_hdl = NULL;
    REG_WRITE(I2S_INT_CLR_REG(0), 0xffffffff);
    clearBuffers();
    REG_WRITE(I2S_INT_ENA_REG(0), I2SANALOG_INTERRUPTS_ENA);
}

chl_i2sanalog::~chl_i2sanalog() {
    esp_intr_free(_i2sanalog_intr_hdl);
    periph_module_disable(PERIPH_I2S0_MODULE);
    periph_module_disable(PERIPH_SARADC_MODULE);
    heap_caps_free(_chl_i2sanalog_dma_rx_buff);
    heap_caps_free(_chl_i2sanalog_dma_inlinks);
    heap_caps_free(_chl_i2sanalog_dma_outlinks);
    vSemaphoreDelete(_tx_links_mtx);
    vSemaphoreDelete(_rx_streambuffer_mtx);
    vStreamBufferDelete(_xRxStreamBuffer);
    periph_rtc_apll_release();
}

void chl_i2sanalog::configureADC(int ichan, int qchan, int bitwidth, int attenuation) {
    REG_SET_FIELD(SYSCON_SARADC_CTRL_REG, SYSCON_SARADC_SAR1_PATT_LEN, 2-1);
    int bitwidth_field = (bitwidth == 9) ? 0b00 : ((bitwidth == 10) ? 0b01 : ((bitwidth == 11) ? 0b10 : 0b11));
    int atten_field = (attenuation == 0) ? 0b00 : ((attenuation == 3) ? 0b01 : ((attenuation == 6) ? 0b10 : 0b11));
    REG_WRITE(SYSCON_SARADC_SAR1_PATT_TAB1_REG, (ichan << 28) | (bitwidth_field << 26) | (atten_field << 24) | (qchan << 20) | (bitwidth_field << 18) | (atten_field << 16));
}

int chl_i2sanalog::setSampleRate(int sr) {
    sr *= 9;
    const int cpufreqdiv2 = 90000000; //APLL freq
    int final_div_bck = 63;
    int final_div_clkm = 255;
    int last_err = sr;
    for(int div_bck = 63;div_bck > 2; div_bck--) {
        //fr = cpufreqdiv2 / (final_div_clkm * final_div_bck)
        if((sr > (cpufreqdiv2/(255*div_bck))) && (sr < (cpufreqdiv2/(2*div_bck)))) {
            int divider = (sr*div_bck);
            int div_clkm = (cpufreqdiv2+divider/2)/divider;
            int fr = cpufreqdiv2 / (div_clkm * div_bck);
            int diff = abs(sr - fr);
            if(diff < last_err) {
                last_err = diff;
                final_div_bck = div_bck;
                final_div_clkm = div_clkm;
                if(last_err == 0) {
                    break;
                }
            }
        }
    }
    REG_SET_FIELD(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_NUM, final_div_clkm);
    REG_SET_FIELD(I2S_SAMPLE_RATE_CONF_REG(0), I2S_RX_BCK_DIV_NUM, final_div_bck);
    REG_SET_FIELD(I2S_SAMPLE_RATE_CONF_REG(0), I2S_TX_BCK_DIV_NUM, final_div_bck);
    return (cpufreqdiv2 / (final_div_clkm * final_div_bck))/9;
}

void chl_i2sanalog::startRx() {
    stopRx();
    _chl_i2sanalog_curr_inlink = 0;
    REG_WRITE(I2S_IN_LINK_REG(0), (((uint32_t)&_chl_i2sanalog_dma_inlinks[0]) << I2S_INLINK_ADDR_S) & I2S_INLINK_ADDR_M);
    REG_SET_BIT(I2S_IN_LINK_REG(0), I2S_INLINK_START);
    REG_SET_BIT(SYSCON_SARADC_CTRL_REG, SYSCON_SARADC_DATA_TO_I2S);
    REG_SET_BIT(I2S_IN_LINK_REG(0), I2S_INLINK_START);
    REG_SET_BIT(I2S_CONF_REG(0), I2S_RX_START);
}

void chl_i2sanalog::stopRx() {
    REG_CLR_BIT(I2S_CONF_REG(0), I2S_RX_START);
    REG_SET_BIT(I2S_IN_LINK_REG(0), I2S_INLINK_STOP);
    REG_CLR_BIT(SYSCON_SARADC_CTRL_REG, SYSCON_SARADC_DATA_TO_I2S);
}

void chl_i2sanalog::startTx() {
    stopTx();
    _chl_i2sanalog_curr_outlink = 0;
    REG_WRITE(I2S_OUT_LINK_REG(0), (((uint32_t)&_chl_i2sanalog_dma_outlinks[0]) << I2S_OUTLINK_ADDR_S) & I2S_OUTLINK_ADDR_M);
    REG_SET_BIT(I2S_OUT_LINK_REG(0), I2S_OUTLINK_START);
    REG_SET_BIT(SENS_SAR_DAC_CTRL1_REG, SENS_DAC_DIG_FORCE);
    REG_SET_BIT(I2S_CONF_REG(0), I2S_TX_START);
}

void chl_i2sanalog::stopTx() {
    REG_CLR_BIT(I2S_CONF_REG(0), I2S_TX_START);
    REG_SET_BIT(I2S_OUT_LINK_REG(0), I2S_OUTLINK_STOP);
    REG_CLR_BIT(SENS_SAR_DAC_CTRL1_REG, SENS_DAC_DIG_FORCE);
}

void chl_i2sanalog::clearBuffers() {
    _chl_i2sanalog_dma_inlinks[0].eof = 1;
    _chl_i2sanalog_dma_inlinks[0].owner = 1;
    _chl_i2sanalog_dma_inlinks[0].size = (sizeof(chl_i2sanalog_type1)*DMA_I2SANALOG_RX_LINK_BUFF_CNT/2);
    _chl_i2sanalog_dma_inlinks[0].length = 0;
    _chl_i2sanalog_dma_inlinks[0].buf = (uint8_t*)_chl_i2sanalog_dma_rx_buff;
    _chl_i2sanalog_dma_inlinks[0].empty = (uint32_t)&_chl_i2sanalog_dma_inlinks[1];
    _chl_i2sanalog_dma_inlinks[1].eof = 1;
    _chl_i2sanalog_dma_inlinks[1].owner = 1;
    _chl_i2sanalog_dma_inlinks[1].size = (sizeof(chl_i2sanalog_type1)*DMA_I2SANALOG_RX_LINK_BUFF_CNT/2);
    _chl_i2sanalog_dma_inlinks[1].length = 0;
    _chl_i2sanalog_dma_inlinks[1].buf = ((uint8_t*)_chl_i2sanalog_dma_rx_buff)+(sizeof(chl_i2sanalog_type1)*DMA_I2SANALOG_RX_LINK_BUFF_CNT/2);
    _chl_i2sanalog_dma_inlinks[1].empty = (uint32_t)&_chl_i2sanalog_dma_inlinks[0];
    _chl_i2sanalog_dma_outlinks[0].eof = 1;
    _chl_i2sanalog_dma_outlinks[0].owner = 1;
    _chl_i2sanalog_dma_outlinks[0].size = 0;
    _chl_i2sanalog_dma_outlinks[0].length = 0;
    _chl_i2sanalog_dma_outlinks[0].buf = 0;
    _chl_i2sanalog_dma_outlinks[0].empty = (uint32_t)&_chl_i2sanalog_dma_outlinks[1];
    _chl_i2sanalog_dma_outlinks[1].eof = 1;
    _chl_i2sanalog_dma_outlinks[1].owner = 1;
    _chl_i2sanalog_dma_outlinks[1].size = 0;
    _chl_i2sanalog_dma_outlinks[1].length = 0;
    _chl_i2sanalog_dma_outlinks[1].buf = 0;
    _chl_i2sanalog_dma_outlinks[1].empty = (uint32_t)&_chl_i2sanalog_dma_outlinks[0];
}

int chl_i2sanalog::read_samples(chl_i2sanalog_type1* buf, unsigned int count, bool block) {
    if(xSemaphoreTake(_rx_streambuffer_mtx, block ? portMAX_DELAY : 0) != pdTRUE) {
        return -1;
    }
    int rlen = xStreamBufferReceive(_xRxStreamBuffer, buf, count*sizeof(chl_i2sanalog_type1), block ? portMAX_DELAY : 0);
    xSemaphoreGive(_rx_streambuffer_mtx);
    return (rlen/sizeof(chl_i2sanalog_type1));
}

int chl_i2sanalog::set_tx_buffer(chl_i2sanalog_dact* buf, unsigned int count, bool block) {
    if(xSemaphoreTake(_tx_links_mtx, block ? portMAX_DELAY : 0) != pdTRUE) {
        return -1;
    }
    while(true) {
        if( _chl_i2sanalog_dma_outlinks[0].length == 0) {
            _chl_i2sanalog_dma_outlinks[0].eof = 0;
            _chl_i2sanalog_dma_outlinks[0].owner = 1;
            _chl_i2sanalog_dma_outlinks[0].size = count*sizeof(chl_i2sanalog_dact);
            _chl_i2sanalog_dma_outlinks[0].length = count*sizeof(chl_i2sanalog_dact);
            _chl_i2sanalog_dma_outlinks[0].buf = (uint8_t*)buf;
            if(_chl_i2sanalog_curr_outlink == -1) {
                _chl_i2sanalog_curr_outlink = 0;
                REG_SET_BIT(I2S_CONF_REG(0), I2S_TX_START);
            }
            xSemaphoreGive(_tx_links_mtx);
            return count;
        } else if( _chl_i2sanalog_dma_outlinks[1].length == 0) {
            _chl_i2sanalog_dma_outlinks[1].eof = 0;
            _chl_i2sanalog_dma_outlinks[1].owner = 1;
            _chl_i2sanalog_dma_outlinks[1].size = count*sizeof(chl_i2sanalog_dact);
            _chl_i2sanalog_dma_outlinks[1].length = count*sizeof(chl_i2sanalog_dact);
            _chl_i2sanalog_dma_outlinks[1].buf = (uint8_t*)buf;
            if(_chl_i2sanalog_curr_outlink == -1) {
                _chl_i2sanalog_curr_outlink = 1;
                REG_SET_BIT(I2S_CONF_REG(0), I2S_TX_START);
            }
            xSemaphoreGive(_tx_links_mtx);
            return count;
        } else if(block) {
            _tx_links_task_hdl = xTaskGetCurrentTaskHandle();
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            _tx_links_task_hdl = NULL;
        } else {
            xSemaphoreGive(_tx_links_mtx);
            return 0;
        }
    }
}

int chl_i2sanalog::getRxSamplesCount() {
    return (xStreamBufferBytesAvailable(_xRxStreamBuffer)/sizeof(chl_i2sanalog_type1));
}

void chl_i2sanalog::_reset_module() {
    REGI2C_WRITE_MASK(I2C_APLL, I2C_APLL_DSDM2, 5);
    REGI2C_WRITE_MASK(I2C_APLL, I2C_APLL_DSDM0, 0);
    REGI2C_WRITE_MASK(I2C_APLL, I2C_APLL_DSDM1, 0);
    REGI2C_WRITE(I2C_APLL, I2C_APLL_SDM_STOP, 0x09);
    REGI2C_WRITE(I2C_APLL, I2C_APLL_SDM_STOP, 0x49);
    REGI2C_WRITE_MASK(I2C_APLL, I2C_APLL_OR_OUTPUT_DIV, 5);
    REGI2C_WRITE(I2C_APLL, I2C_APLL_IR_CAL_DELAY, 0x0f);
    REGI2C_WRITE(I2C_APLL, I2C_APLL_IR_CAL_DELAY, 0x3f);
    REGI2C_WRITE(I2C_APLL, I2C_APLL_IR_CAL_DELAY, 0x1f);
    while(!REGI2C_READ_MASK(I2C_APLL, I2C_APLL_OR_CAL_END)) {
        esp_rom_delay_us(1);
    }
    //APLL = 90 MHz
    REG_WRITE(I2S_INT_ENA_REG(0), 0);
    REG_WRITE(I2S_CONF_REG(0), I2S_RX_FIFO_RESET | I2S_TX_FIFO_RESET | I2S_RX_RESET | I2S_TX_RESET);
    REG_WRITE(I2S_CONF_REG(0), I2S_TX_RIGHT_FIRST);
    REG_WRITE(I2S_CONF1_REG(0), I2S_RX_PCM_BYPASS | I2S_TX_PCM_BYPASS);
    REG_WRITE(I2S_CONF2_REG(0), I2S_LCD_EN);
    REG_WRITE(I2S_TIMING_REG(0), 0);
    REG_WRITE(I2S_FIFO_CONF_REG(0), I2S_RX_FIFO_MOD_FORCE_EN | I2S_TX_FIFO_MOD_FORCE_EN | (0x0 << I2S_TX_FIFO_MOD_S) | (0x1 << I2S_RX_FIFO_MOD_S) | (64 << I2S_TX_DATA_NUM_S) | (64 << I2S_RX_DATA_NUM_S));
    REG_WRITE(I2S_CONF_SIGLE_DATA_REG(0), 0);
    REG_WRITE(I2S_CONF_CHAN_REG(0), (0x0 << I2S_TX_CHAN_MOD_S) | (0x1 << I2S_RX_CHAN_MOD_S));
    REG_WRITE(I2S_LC_HUNG_CONF_REG(0), 0);
    REG_WRITE(I2S_CLKM_CONF_REG(0), (1 << I2S_CLKA_ENA_S) | (64 << I2S_CLKM_DIV_NUM_S) | (1 << I2S_CLKM_DIV_B_S) | (0 << I2S_CLKM_DIV_A_S)); //160MHz/250 = 2.5 MHz I2S clk
    REG_WRITE(I2S_SAMPLE_RATE_CONF_REG(0), (16 << I2S_RX_BITS_MOD_S) | (16 << I2S_TX_BITS_MOD_S) | (25 << I2S_RX_BCK_DIV_NUM_S) | (25 << I2S_TX_BCK_DIV_NUM_S)); //2.5 MHz / 25 = 100 kHz BCK
    REG_WRITE(I2S_PD_CONF_REG(0), I2S_PLC_MEM_FORCE_PU | I2S_FIFO_FORCE_PU);
    REG_WRITE(I2S_LC_CONF_REG(0), I2S_AHBM_RST | I2S_AHBM_FIFO_RST | I2S_OUT_RST | I2S_IN_RST);
    REG_WRITE(I2S_LC_CONF_REG(0), I2S_OUT_EOF_MODE | I2S_OUT_AUTO_WRBACK);
    REG_WRITE(I2S_RXEOF_NUM_REG(0), DMA_I2SANALOG_RX_LINK_BUFF_CNT);
    REG_WRITE(I2S_OUT_LINK_REG(0), (((uint32_t)&_chl_i2sanalog_dma_outlinks[0]) << I2S_OUTLINK_ADDR_S) & I2S_OUTLINK_ADDR_M);
    REG_WRITE(I2S_IN_LINK_REG(0), (((uint32_t)&_chl_i2sanalog_dma_inlinks[0]) << I2S_INLINK_ADDR_S) & I2S_INLINK_ADDR_M);
    REG_WRITE(I2S_PDM_CONF_REG(0), 0);
    REG_WRITE(I2S_PDM_FREQ_CONF_REG(0), 0);

    REG_WRITE(SYSCON_SARADC_FSM_REG, (8 << SYSCON_SARADC_RSTB_WAIT_S) | (2 << SYSCON_SARADC_START_WAIT_S) | (100 << SYSCON_SARADC_STANDBY_WAIT_S) | (2 << SYSCON_SARADC_SAMPLE_CYCLE_S));
    // REG_WRITE(SENS_SAR_START_FORCE_REG, (4 << SENS_SAR2_PWDET_CCT_S));
    // REG_WRITE(SENS_SAR_READ_CTRL_REG, SENS_SAR1_DIG_FORCE | (0b11 << SENS_SAR1_SAMPLE_BIT_S) | (2 << SENS_SAR1_SAMPLE_CYCLE_S) | (2 << SENS_SAR1_CLK_DIV_S));
    REG_SET_BIT(SENS_SAR_READ_CTRL_REG, SENS_SAR1_DIG_FORCE);
    REG_WRITE(SYSCON_SARADC_CTRL_REG, SYSCON_SARADC_SAR1_PATT_P_CLEAR | SYSCON_SARADC_SAR2_PATT_P_CLEAR);
    REG_WRITE(SYSCON_SARADC_CTRL_REG, (2 << SYSCON_SARADC_SAR_CLK_DIV_S) | SYSCON_SARADC_SAR_CLK_GATED | ((2-1) << SYSCON_SARADC_SAR1_PATT_LEN_S));
    REG_WRITE(SYSCON_SARADC_CTRL2_REG, SYSCON_SARADC_MEAS_NUM_LIMIT | (10 << SYSCON_SARADC_MAX_MEAS_NUM_S) | SYSCON_SARADC_SAR1_INV);

    REG_WRITE(SYSCON_SARADC_SAR1_PATT_TAB1_REG, (0 << 28) | (0b11 << 26) | (0b11 << 24) | (3 << 20) | (0b11 << 18) | (0b11 << 16)); //channel 0,12bit,11dB; channel 3,12bit,11dB

    chl_rtc_gpio_enable_dacs(true, true);
    REG_WRITE(SENS_SAR_DAC_CTRL1_REG, SENS_DAC_CLK_INV);
    REG_WRITE(SENS_SAR_DAC_CTRL2_REG, 0);

    REG_SET_BIT(I2S_FIFO_CONF_REG(0), I2S_DSCR_EN);
}

void chl_i2sanalog::_i2sanalog_intr_hdlr(void* arg) {
    chl_i2sanalog* _this = (chl_i2sanalog*)arg;
    portBASE_TYPE contsw_req = false;
    while(1) {
        uint32_t curr_intr = REG_READ(I2S_INT_RAW_REG(0));
        //ets_printf("a %lu\r\n", curr_intr);
        if((curr_intr & (I2SANALOG_INTERRUPTS_RAW)) == 0) {
            break;
        }
        REG_WRITE(I2S_INT_CLR_REG(0), curr_intr);
        if(curr_intr & I2S_IN_SUC_EOF_INT_RAW) {
            if(_this->_chl_i2sanalog_curr_inlink >= 0) {
                //lldesc_t* curr_descr = (lldesc_t*)REG_READ(I2S_INLINK_DSCR_REG(0));
                lldesc_t* curr_descr = &_this->_chl_i2sanalog_dma_inlinks[_this->_chl_i2sanalog_curr_inlink];
                _this->_chl_i2sanalog_curr_inlink = (_this->_chl_i2sanalog_curr_inlink + 1) % 2;
                curr_descr->eof = 1;
                curr_descr->owner = 1;
                if(curr_descr->length != 0) {
                    //ets_printf("b %lu eof %u own %u sz %u len %u\r\n", REG_READ(I2S_INLINK_DSCR_REG(0)), curr_descr->eof, curr_descr->owner, curr_descr->size, curr_descr->length);
                    xStreamBufferSendFromISR(_this->_xRxStreamBuffer, (const void*)curr_descr->buf, curr_descr->length, &contsw_req);
                }
                curr_descr->length = 0;
            }
        }
        if((curr_intr & I2S_OUT_DONE_INT_RAW)) {
            // ets_printf("X\n");
            //lldesc_t* curr_descr = (lldesc_t*)REG_READ(I2S_INLINK_DSCR_REG(0));
            if(_this->_chl_i2sanalog_curr_outlink >= 0) {
                lldesc_t* curr_descr = &_this->_chl_i2sanalog_dma_outlinks[_this->_chl_i2sanalog_curr_outlink];
                _this->_chl_i2sanalog_curr_outlink = (_this->_chl_i2sanalog_curr_outlink + 1) % 2;
                lldesc_t* next_descr = &_this->_chl_i2sanalog_dma_outlinks[_this->_chl_i2sanalog_curr_outlink];
                if(next_descr->length == 0) {
                    //Not enough data; stopping
                    ets_printf("N\n");
                    REG_CLR_BIT(I2S_CONF_REG(0), I2S_TX_START);
                    _this->_chl_i2sanalog_curr_outlink = -1;
                }
                curr_descr->eof = 0;
                curr_descr->owner = 1;
                curr_descr->length = 0;
                if(_this->_tx_links_task_hdl != NULL) {
                    vTaskNotifyGiveFromISR(_this->_tx_links_task_hdl, &contsw_req);
                }
            }
        }
    }
    if(contsw_req) {
        portYIELD_FROM_ISR();
    }
}
