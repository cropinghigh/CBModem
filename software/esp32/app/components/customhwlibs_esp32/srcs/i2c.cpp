#include "i2c.h"

chl_i2c::chl_i2c(int num, int sckspdkhz, int sdagpio, int sclgpio) {
    assert(num == 0 || num == 1);
    _number = num;
    _sdagpio = sdagpio;
    _sclgpio = sclgpio;
    _sckspdkhz = sckspdkhz;
    _ram_data = (volatile uint32_t*)((I2C_FIFO_START_ADDR_REG(_number) - 0x3FF40000) + 0x60000000);
    _last_transm_state = 4;
    _last_transm_end = 0;
    _i2c_transm_bsmph = xSemaphoreCreateBinary();
    if(_i2c_transm_bsmph == NULL) {
        printf("I2C%d ERROR: NOT ENOUGH MEMORY FOR TRANSMISSION BINARY SEMAPHORE\r\n", _number);
        return;
    }
    xSemaphoreGive(_i2c_transm_bsmph);
    periph_module_enable((_number == 0) ? PERIPH_I2C0_MODULE : PERIPH_I2C1_MODULE);
    _reset_module();

    if(esp_intr_alloc(ETS_I2C_EXT0_INTR_SOURCE, ESP_INTR_FLAG_IRAM, _i2c_intr_hdlr, this, &_i2c_intr_hdl) != ESP_OK) {
        printf("I2C%d ERROR: ESP INTR ALLOC FAIL\r\n", _number);
        return;
    }
    REG_WRITE(I2C_INT_ENA_REG(_number), 0);
    REG_WRITE(I2C_INT_CLR_REG(_number), 0b1111111101000);
    //test_print_regs(_ram_data, _number);
}

chl_i2c::~chl_i2c() {
    vSemaphoreDelete(_i2c_transm_bsmph);
    periph_module_disable((_number == 0) ? PERIPH_I2C0_MODULE : PERIPH_I2C1_MODULE);
}

int chl_i2c::i2c_write_regs(uint8_t devaddr, uint8_t startregnum, uint8_t* buffer, unsigned int len, bool block, bool check_ack) {
    if(len > 30) {
        len = 30; //i2c memory is limited to 32 bytes
    }
    if(xSemaphoreTake(_i2c_transm_bsmph, block ? portMAX_DELAY : 0) != pdTRUE) {
        return ESP_FAIL;
    }
    _set_commands_tx(devaddr, startregnum, len, check_ack);
    for(int i = 0; i < len; i++) {
        _ram_data[i+2] = buffer[i];
    }
    REG_WRITE(I2C_INT_ENA_REG(_number), I2C_INTERRUPTS_ENA);
    REG_SET_BIT(I2C_CTR_REG(_number), I2C_TRANS_START);
    return ESP_OK;
}

int chl_i2c::i2c_read_regs(uint8_t devaddr, uint8_t startregnum, uint8_t* buffer, unsigned int len, bool check_ack) {
    if(len > 32) {
        len = 32; //i2c memory is limited to 32 bytes
    }
    if(xSemaphoreTake(_i2c_transm_bsmph, portMAX_DELAY) != pdTRUE) {
        return ESP_FAIL;
    }
    _set_commands_rx(devaddr, startregnum, len, check_ack);
    REG_WRITE(I2C_INT_ENA_REG(_number), I2C_INTERRUPTS_ENA);
    REG_SET_BIT(I2C_CTR_REG(_number), I2C_TRANS_START);
    if(xSemaphoreTake(_i2c_transm_bsmph, portMAX_DELAY) != pdTRUE) {
        return ESP_FAIL;
    }
    int ret = _last_transm_state;
    for(int i = 0; i < len; i++) {
        buffer[i] = _ram_data[i];
    }
    xSemaphoreGive(_i2c_transm_bsmph);
    return ret;
}

int chl_i2c::i2c_wait_for_transaction() {
    if(xSemaphoreTake(_i2c_transm_bsmph, portMAX_DELAY) != pdTRUE) {
        return -1;
    }
    int lts = _last_transm_state;
    xSemaphoreGive(_i2c_transm_bsmph);
    return lts;
}

void chl_i2c::_set_commands_tx(uint8_t devaddr, uint8_t startregnum, uint8_t len, bool check_ack) {
    uint16_t comd0 = (0 << 11) | (0 << 10) | (0 << 9) | (0 << 8); //start
    uint16_t comd1 = (1 << 11) | (0 << 10) | (0 << 9) | (check_ack << 8) | (1); //write 1(addr) bytes; check ack;
    uint16_t comd2 = (1 << 11) | (0 << 10) | (0 << 9) | (check_ack << 8) | (1); //write 1(regaddr) bytes; check ack;
    uint16_t comd3 = (1 << 11) | (0 << 10) | (0 << 9) | (check_ack << 8) | (len); //write len bytes; check ack;
    uint16_t comd4 = (3 << 11) | (0 << 10) | (0 << 9) | (0 << 8); //stop
    REG_WRITE(I2C_COMD0_REG(_number), comd0);
    REG_WRITE(I2C_COMD1_REG(_number), comd1);
    REG_WRITE(I2C_COMD2_REG(_number), comd2);
    REG_WRITE(I2C_COMD3_REG(_number), comd3);
    REG_WRITE(I2C_COMD4_REG(_number), comd4);
    REG_SET_BIT(I2C_FIFO_CONF_REG(_number), I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);
    REG_CLR_BIT(I2C_FIFO_CONF_REG(_number), I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);
    _ram_data[0] = ((devaddr&0x7F) << 1) | 0; //send device address and indicate write operation
    _ram_data[1] = startregnum;
    REG_SET_FIELD(I2C_FIFO_CONF_REG(_number), I2C_NONFIFO_TX_THRES, len+3);
}

void chl_i2c::_set_commands_rx(uint8_t devaddr, uint8_t startregnum, uint8_t len, bool check_ack) {
    uint16_t comd0 = (0 << 11) | (0 << 10) | (0 << 9) | (0 << 8); //start
    uint16_t comd1 = (1 << 11) | (0 << 10) | (0 << 9) | (check_ack << 8) | (1); //write 1(addr) bytes(write); check ack;
    uint16_t comd2 = (1 << 11) | (0 << 10) | (0 << 9) | (check_ack << 8) | (1); //write 1(regaddr) bytes; check ack;
    uint16_t comd3 = (0 << 11) | (0 << 10) | (0 << 9) | (0 << 8); //restart
    uint16_t comd4 = (1 << 11) | (0 << 10) | (0 << 9) | (check_ack << 8) | (1); //write 1(addr) bytes(read); check ack;
    uint16_t comd5 = (2 << 11) | (0 << 10) | (0 << 9) | (0 << 8) | (len-1); //read len bytes; send ack;
    uint16_t comd6 = (2 << 11) | (1 << 10) | (0 << 9) | (0 << 8) | (1); //read 1 bytes; send nack;
    uint16_t comd7 = (3 << 11) | (0 << 10) | (0 << 9) | (0 << 8); //stop
    REG_WRITE(I2C_COMD0_REG(_number), comd0);
    REG_WRITE(I2C_COMD1_REG(_number), comd1);
    REG_WRITE(I2C_COMD2_REG(_number), comd2);
    REG_WRITE(I2C_COMD3_REG(_number), comd3);
    REG_WRITE(I2C_COMD4_REG(_number), comd4);
    REG_WRITE(I2C_COMD5_REG(_number), comd5);
    REG_WRITE(I2C_COMD6_REG(_number), comd6);
    REG_WRITE(I2C_COMD7_REG(_number), comd7);
    REG_SET_BIT(I2C_FIFO_CONF_REG(_number), I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);
    REG_CLR_BIT(I2C_FIFO_CONF_REG(_number), I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);
    _ram_data[0] = ((devaddr&0x7F) << 1) | 0; //send device address and indicate write operation
    _ram_data[1] = startregnum;
    _ram_data[2] = ((devaddr&0x7F) << 1) | 1; //send device address and indicate read operation
    REG_SET_FIELD(I2C_FIFO_CONF_REG(_number), I2C_NONFIFO_TX_THRES, 4);
    REG_SET_FIELD(I2C_FIFO_CONF_REG(_number), I2C_NONFIFO_RX_THRES, len+1);
}

void chl_i2c::_reset_module() {
    DPORT_REG_SET_BIT(DPORT_PERIP_RST_EN_REG, (_number == 0) ? DPORT_I2C_EXT0_RST : DPORT_I2C_EXT1_RST);
    _config_gpio();
    DPORT_REG_CLR_BIT(DPORT_PERIP_RST_EN_REG, (_number == 0) ? DPORT_I2C_EXT0_RST : DPORT_I2C_EXT1_RST);
    REG_WRITE(I2C_INT_ENA_REG(_number), 0);
    REG_WRITE(I2C_CTR_REG(_number), I2C_MS_MODE | I2C_SCL_FORCE_OUT | I2C_SDA_FORCE_OUT);
    REG_WRITE(I2C_SLAVE_ADDR_REG(_number), 0);
    REG_WRITE(I2C_FIFO_CONF_REG(_number),  (1 << I2C_NONFIFO_TX_THRES_S) | (1 << I2C_NONFIFO_RX_THRES_S) | I2C_NONFIFO_EN);
    REG_SET_BIT(I2C_FIFO_CONF_REG(_number), I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);
    REG_CLR_BIT(I2C_FIFO_CONF_REG(_number), I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);

    uint32_t scl_period_apb = APB_CLK_FREQ / (_sckspdkhz*1000);
    uint32_t scl_edge_to_edge = scl_period_apb/2;
    uint32_t scl_high_period = scl_edge_to_edge-8; //Max freq~=3MHz(practically with 1k resistors)
    uint32_t scl_low_period = scl_edge_to_edge-1;
    REG_WRITE(I2C_SCL_LOW_PERIOD_REG(_number), (scl_low_period << I2C_SCL_LOW_PERIOD_S) & I2C_SCL_LOW_PERIOD_M);
    REG_WRITE(I2C_SCL_HIGH_PERIOD_REG(_number), (scl_high_period << I2C_SCL_HIGH_PERIOD_S) & I2C_SCL_HIGH_PERIOD_M);
    REG_WRITE(I2C_SCL_START_HOLD_REG(_number), (scl_edge_to_edge << I2C_SCL_START_HOLD_TIME_S) & I2C_SCL_START_HOLD_TIME_M);
    REG_WRITE(I2C_SDA_HOLD_REG(_number), (scl_edge_to_edge/2 << I2C_SDA_HOLD_TIME_S) & I2C_SDA_HOLD_TIME_M); //REQUIRED, otherwise sda is low during all the transmission
    REG_WRITE(I2C_SDA_SAMPLE_REG(_number), (scl_edge_to_edge/2 << I2C_SDA_SAMPLE_TIME_S) & I2C_SDA_SAMPLE_TIME_M);
    REG_WRITE(I2C_SCL_RSTART_SETUP_REG(_number), (scl_edge_to_edge*2 << I2C_SCL_RSTART_SETUP_TIME_S) & I2C_SCL_RSTART_SETUP_TIME_M);
    REG_WRITE(I2C_SCL_STOP_HOLD_REG(_number), (scl_edge_to_edge << I2C_SCL_STOP_HOLD_TIME_S) & I2C_SCL_STOP_HOLD_TIME_M);
    REG_WRITE(I2C_SCL_STOP_SETUP_REG(_number), (scl_edge_to_edge*2 << I2C_SCL_STOP_SETUP_TIME_S) & I2C_SCL_STOP_SETUP_TIME_M);
    REG_WRITE(I2C_SCL_FILTER_CFG_REG(_number), 0b1001);
    REG_WRITE(I2C_SDA_FILTER_CFG_REG(_number), 0b1001);
    REG_WRITE(I2C_TO_REG(_number), (scl_edge_to_edge*20 << I2C_TIME_OUT_REG_S) & I2C_TIME_OUT_REG_M);

    REG_WRITE(I2C_INT_CLR_REG(_number), 0b1111111101000);
}

void chl_i2c::_config_gpio() {
    //Reset bus in bitbanging mode
    chl_gpio_connect_out(_sdagpio, SIG_GPIO_OUT_IDX, false);
    chl_gpio_connect_out(_sclgpio, SIG_GPIO_OUT_IDX, false);
    chl_gpio_iomux_select_func(_sdagpio, PIN_FUNC_GPIO);
    chl_gpio_iomux_select_func(_sclgpio, PIN_FUNC_GPIO);
    chl_gpio_set_direction(_sdagpio, true, true, true, false, false); //+input, +output, +od, -pullup, -pulldown
    chl_gpio_set_direction(_sclgpio, true, true, true, false, false); //+input, +output, +od, -pullup, -pulldown
    // If a SLAVE device was in a read operation when the bus was interrupted, the SLAVE device is controlling SDA.
    // The only bit during the 9 clock cycles of a READ byte the MASTER(ESP32) is guaranteed control over is during the ACK bit
    // period. If the slave is sending a stream of ZERO bytes, it will only release SDA during the ACK bit period.
    // So, this reset code needs to synchronize the bit stream with, Either, the ACK bit, Or a 1 bit to correctly generate
    // a STOP condition.
    chl_gpio_set_level(_sclgpio, 0);
    chl_gpio_set_level(_sdagpio, 1);
    esp_rom_delay_us(5);
    for(int i = 0; i < 9; i++) {
        if(chl_gpio_get_level(_sdagpio)) {
            break;
        }
        chl_gpio_set_level(_sclgpio, 1);
        esp_rom_delay_us(5);
        chl_gpio_set_level(_sclgpio, 0);
        esp_rom_delay_us(5);
    }
    chl_gpio_set_level(_sdagpio, 0); // setup for STOP
    chl_gpio_set_level(_sclgpio, 1);
    esp_rom_delay_us(5);
    chl_gpio_set_level(_sdagpio, 1); // STOP, SDA low -> high while SCL is HIGH
    esp_rom_delay_us(5);

    //Configure SDA pin
    //IOMUX and direction are configured earlier
    chl_gpio_set_level(_sdagpio, 1);
    chl_gpio_connect_out(_sdagpio, (_number == 0) ? I2CEXT0_SDA_OUT_IDX : I2CEXT1_SDA_OUT_IDX, false);
    chl_gpio_connect_in(_sdagpio, (_number == 0) ? I2CEXT0_SDA_IN_IDX : I2CEXT1_SDA_IN_IDX, false, false);

    //Configure SCL pin
    //IOMUX and direction are configured earlier
    chl_gpio_connect_out(_sclgpio, (_number == 0) ? I2CEXT0_SCL_OUT_IDX : I2CEXT1_SCL_OUT_IDX, false);
    chl_gpio_connect_in(_sclgpio, (_number == 0) ? I2CEXT0_SCL_IN_IDX : I2CEXT1_SCL_IN_IDX, false, false);
}

void chl_i2c::_i2c_intr_hdlr(void* arg) {
    chl_i2c* _this = (chl_i2c*) arg;
    portBASE_TYPE contsw_req = false;
    uint32_t curr_intr = REG_READ(I2C_INT_RAW_REG(_this->_number)) & 0b1111111101000;
    if(curr_intr == 0) {
        return;
    }
    bool disable_intr = false;
    if(curr_intr & I2C_TX_SEND_EMPTY_INT_RAW) {
        
    }
    if(curr_intr & I2C_RX_REC_FULL_INT_RAW) {
        
    }
    if(curr_intr & I2C_TRANS_COMPLETE_INT_RAW) {
        _this->_last_transm_state = 0;
        disable_intr = true;
    }
    if(curr_intr & I2C_ACK_ERR_INT_RAW) {
        _this->_last_transm_state = 1;
        disable_intr = true;
    }
    if(curr_intr & I2C_TIME_OUT_INT_RAW) {
        _this->_last_transm_state = 2;
        _this->_config_gpio(); //Clear bus to allow I2C module to work properly
        disable_intr = true;
    }
    if(curr_intr & I2C_ARBITRATION_LOST_INT_RAW) {
        _this->_last_transm_state = 3;
        disable_intr = true;
    }
    if(disable_intr) {
        //transmission is finished
        xSemaphoreGiveFromISR(_this->_i2c_transm_bsmph, &contsw_req);
        REG_WRITE(I2C_INT_ENA_REG(_this->_number), 0);
    }
    REG_WRITE(I2C_INT_CLR_REG(_this->_number), curr_intr);
    if(contsw_req) {
        portYIELD_FROM_ISR();
    }
}
