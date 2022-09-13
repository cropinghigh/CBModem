#pragma once

//Enable I2C module
void i2c_enable() {
    TWCR = (1 << TWEN) | (1<<TWEA);
}

//Disable I2C module
void i2c_disable() {
    TWCR = 0;
}

//Initialize I2C bus
bool i2c_setup() {
    TWSR |= (0b01 << 0); //prescaler = 4
    TWBR = 13; //100 kHz bus speed
    TWCR = 0;
    i2c_disable();
    i2c_enable();
    return true;
}

//Start operation by writing 1 to TWINT bit and wait for it to complete
inline void i2c_sync_do_operation() {
    TWCR |= (1<<TWINT);
    while(!(TWCR & (1 << TWINT)));
}

//Get current I2C state
inline uint8_t i2c_get_state() {
    return (TWSR & 0xF8);
}

//Send I2C start bit
bool i2c_send_start() {
    TWCR |= (1<<TWSTA); //send start bit
    i2c_sync_do_operation();
    return i2c_get_state() == START || i2c_get_state() == REP_START;
}

//Write device address to the I2C bus, read=enable reading mode(0=writing)
bool i2c_send_addr(bool read, uint8_t addr) {
    TWDR = (addr << 1) | read;
    i2c_sync_do_operation();
    return i2c_get_state() == MT_SLA_ACK || i2c_get_state() == MR_SLA_ACK;
}

//Write a byte to the I2C bus
bool i2c_send_byte(uint8_t data) {
    TWDR = data;
    i2c_sync_do_operation();
    return i2c_get_state() == MT_DATA_ACK;
}

//Read a byte from the I2C bus
bool i2c_read_byte(uint8_t& data) {
    i2c_sync_do_operation();
    data = TWDR;
    return i2c_get_state() == MR_DATA_ACK;
}

//Send I2C stop bit
void i2c_send_stop() {
    TWCR = (1<<TWSTO);
    i2c_sync_do_operation();
}

//Read data from I2C bus from specified device(addr) from specified start register(reg_addr)
bool i2c_sync_read(uint8_t addr, uint8_t reg_addr, int count, uint8_t* buffer) {
    if(!i2c_send_start()) { return false;}
    if(!i2c_send_addr(0, addr)) { return false;}
    if(!i2c_send_byte(reg_addr)) { return false;}
    if(!i2c_send_start()) { return false;}
    if(!i2c_send_addr(1, addr)) { return false;}
    for(int i = 0; i < count; i++) {
        if(!i2c_read_byte(buffer[i])) {return false;}
    }
    i2c_send_stop();
    return true;
}

//Write data to I2C bus to specified device(addr) to specified start register(reg_addr)
bool i2c_sync_write(uint8_t addr, uint8_t reg_addr, int count, uint8_t* buffer) {
    if(!i2c_send_start()) { return false;}
    if(!i2c_send_addr(0, addr)) { return false;}
    if(!i2c_send_byte(reg_addr)) { return false;}
    for(int i = 0; i < count; i++) {
        if(!i2c_send_byte(buffer[i]);
    }
    i2c_send_stop();
    return true;
}
