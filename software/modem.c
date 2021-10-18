#define F_CPU 12000000UL

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//1-4 bytes
#define SYNCWORD_CONST 0b11011000111101000010100011010011

const char strAbout[] = "HF modem firmware. Author: https://github.com/cropinghigh";

const char strHelp[] = "Help: \n\rw - enter text to transmit\n\rre - enable receiving\n\rrd - disable receiving\n\ree - enable echo\n\red - disable echo\n\r";

const char strI2CError[] = "I2C error: ";

volatile char UARTRxRingBuff[256];
volatile uint8_t UARTRxRingBuffReadPtr = 0;
volatile uint8_t UARTRxRingBuffWritePtr = 0;
volatile char UARTTxRingBuff[256];
volatile uint8_t UARTTxRingBuffReadPtr = 0;
volatile uint8_t UARTTxRingBuffWritePtr = 0;
char UARTCommandBuffer[80];

volatile uint8_t ModemRXTXBuffer[831]; //1byte=1bit
volatile int ModemRXTXBufferLen = 0; //buffer size
volatile int ModemRXTXBufferPtr = 0; //buffer pointer
volatile char ModemRXTXDataByteBuffer[82]; //80 bytes + 2byte checksum
volatile bool TX_phase = false;
volatile bool TX_bithalf = false;
volatile bool RX_phase = false;
volatile bool RX_prev_value = false;
volatile bool RX_syncword_received = false;
volatile uint8_t RX_byte_number = 0;
volatile uint8_t RX_byte_receiving = 0;
volatile uint8_t RX_byte_bit_number = 0;
volatile uint8_t RX_packet_length = 0;
volatile uint32_t RX_syncword_buffer = 0;

uint8_t StatusByte = 0b01; //0 bit - is echo enabled, 1 bit - is receiving enabled

//Setup UART parameters
void setupUART() {
    UCSR0A = (1 << U2X0); //double the USART transmission speed
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0) | (1 << UDRIE0); //enable transmitter; enable receiver; enable rx complete interrupt
    UCSR0C = (3 << UCSZ00); //8-bit character size
    UBRR0 = 12; //115200 b/s while F_CPU==12M
    UARTRxRingBuffReadPtr = UARTRxRingBuffWritePtr = UARTTxRingBuffReadPtr = UARTTxRingBuffWritePtr = 0;
}

//Write char to UART, buffer overflows ignored
void writeToUart(char c) {
    UARTTxRingBuff[UARTTxRingBuffWritePtr++] = c;
    if(UARTTxRingBuffWritePtr == UARTTxRingBuffReadPtr) {
        UARTTxRingBuffWritePtr--;
    }
    UCSR0B |= (1 << UDRIE0); //allow UART UDR Empty interrupt
}

//Print uint to UART as decimal number
void printUIntToUart(unsigned int v) {
    char a[32];
    itoa(v, a, 10);
    for(int i = 0; i < 32; i++) {
        char c = a[i];
        if(c == '\0') {
            break;
        }
        writeToUart(c);
    }
}

//Write CR+LF to UART
void writeNLUart() {
    writeToUart('\r');
    writeToUart('\n');
}

//Read char from uart, blocking. Print back if echo enabled.
char readFromUart() {
    while(UARTRxRingBuffReadPtr == UARTRxRingBuffWritePtr) {}
    char c = UARTRxRingBuff[UARTRxRingBuffReadPtr++];
    if(StatusByte & 0b01) {
        writeToUart(c);
    }
    return c;
}

//Do init sequence
void waitForStart() {
    writeToUart('s');writeNLUart();
_sym1:
    char c = readFromUart(); //wait for s\r\n or s\n
    if(c != 's')
        goto _sym1;
    c = readFromUart();
    if(c == '\n')
        goto _end;
    if(c != '\r')
        goto _sym1;
    c = readFromUart();
    if(c != '\n')
        goto _sym1;
_end:
    writeToUart('o');writeNLUart();
    PORTD |= (1 << 5); //enable USB led
}

//Read from I2C bus
bool syncI2CRead(uint8_t addr, uint8_t reg_addr, int count, uint8_t* buffer) {
    int current = 0;
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); //send start bit
    while(!(TWCR & (1 << TWINT))); //wait for i2c to complete operation
    if((TWSR & 0xf8) != 0x08) {
            goto _error;
    }
    TWDR = (addr << 1);
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x18) {
        goto _error;
    }
    TWDR = reg_addr;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x28) {
        goto _error;
    }
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); //send second start bit
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x10) {
        goto _error;
    }
    TWDR = (addr << 1) | 0b1; //read mode
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x40) {
        goto _error;
    }
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x50) {
        goto _error;
    }
    buffer[current++] = TWDR;
    while(current < count) {
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
        while(!(TWCR & (1 << TWINT)));
        if((TWSR & 0xf8) != 0x50) {
            goto _error;
        }
        buffer[current++] = TWDR;
    }
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x58) {
        goto _error;
    }
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    _delay_us(40); //allow TWI to send stop bit
    return true;
_error:
    for(int i = 0; i < sizeof(strI2CError); i++) {
        writeToUart(strI2CError[i]);
    }
    printUIntToUart((TWSR & 0xf8)); writeNLUart();
    return false;
}

//Write to I2C bus
bool syncI2CWrite(uint8_t addr, uint8_t reg_addr, int count, uint8_t* buffer) {
    int current = 0;
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); //send start bit
    while(!(TWCR & (1 << TWINT))); //wait for i2c to complete operation
    if((TWSR & 0xf8) != 0x08) {
        goto _error;
    }
    TWDR = (addr << 1);
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x18) {
        goto _error;
    }
    TWDR = reg_addr;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x28) {
        goto _error;
    }
    TWDR = buffer[current++];
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xf8) != 0x28) {
        goto _error;
    }
    while(current < count) {
        TWDR = buffer[current++];
        TWCR = (1<<TWINT) | (1<<TWEN);
        while(!(TWCR & (1 << TWINT)));
        if((TWSR & 0xf8) != 0x28) {
            goto _error;
        }
    }
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    _delay_us(40); //allow TWI to send stop bit
    return true;
_error:
    for(int i = 0; i < sizeof(strI2CError); i++) {
        writeToUart(strI2CError[i]);
    }
    printUIntToUart((TWSR & 0xf8));writeNLUart();
    return false;
}

//Setup I2C bus and SI5351 config
void setupSI5351() {
    TWSR |= (0b01 << 0); //prescaler = 4
    TWBR = 13; //100 kHz bus speed
    uint8_t addr = 0b1100000; //si5351 addr
    uint8_t buffer[32];
    syncI2CRead(addr, 0, 1, buffer);
    while(((buffer[0] & (1 << 7)) >> 7)) {
        syncI2CRead(addr, 0, 1, buffer); //wait for device initialization complete
    }
    buffer[0] = 0xff;
    syncI2CWrite(addr, 3, 1, (uint8_t[]){0xff}); //disable all CLKs outputs
    buffer[0] = 0x80;
    for(uint8_t reg_addr = 16; reg_addr < 24; reg_addr++) {
        syncI2CWrite(addr, reg_addr, 1, (uint8_t[]){0x80}); //write 0x80 to regs 16-23(powerdown all)
    }
    //Write PLLs/multisynths data
    //CLK0=27.135.000M, CLK1=26.680.000M, CLKERROR = +13 kHz
    //PLLA multiplier = 28, int mode(F=700M); MSNA_P1=3072, MSNA_P2=0, MSNA_P3=0, FBA_INT=1
    //Multisynth0 divider = 25 + 809306/1000000(F~27.122000 + 13 kHz)
    //Multisynth1 divider = 26 + 249671/1000000(F~26.667000 + 13 kHz);
    syncI2CWrite(addr, 22, 1, (uint8_t[]){0b11000000}); //FBA_INT=1
    #define MSNA_a 28
    #define MS0_a 25
    #define MS0_b 809306.0
    #define MS0_c 1000000.0
    #define MS1_a 26
    #define MS1_b 249671.0
    #define MS1_c 1000000.0
    uint32_t MSNA_P1 = (128 * MSNA_a) + floor(128.0f * 0.0f) - 512;
    uint32_t MS0_P1 = (128 * MS0_a) + floor(128.0f * (MS0_b / MS0_c)) - 512;
    uint32_t MS0_P2 = (128 * MS0_b) - (MS0_c * floor(128.0f * (MS0_b / MS0_c)));
    uint32_t MS0_P3 = MS0_c;
    uint32_t MS1_P1 = (128 * MS1_a) + floor(128.0f * (MS1_b / MS1_c)) - 512;
    uint32_t MS1_P2 = (128 * MS1_b) - (MS0_c * floor(128.0f * (MS1_b / MS1_c)));
    uint32_t MS1_P3 = MS1_c;
    syncI2CWrite(addr, 26, 2, (uint8_t[]){0x00, 0x00}); //MSNA_P3[15:8] ; MSNA_P3[7:0]
    syncI2CWrite(addr, 28, 3, (uint8_t[]){
        ((MSNA_P1 & (3L << 16L)) >> 16L),
        ((MSNA_P1 & (0xffL << 8L)) >> 8L),
        ((MSNA_P1 & (0xffL << 0L)) >> 0L)
    }); //MSNA_P1[17:16] ; MSNA_P1[15:8] ; MSNA_P1[7:0]
    syncI2CWrite(addr, 31, 3, (uint8_t[]){0x00, 0x00, 0x00}); //MSNA_P3[19:16];MSNA_P2[19:16] ; MSNA_P2[15:8] ; MSNA_P2[7:0]
    syncI2CWrite(addr, 42, 8, (uint8_t[]){
        ((MS0_P3 & (0xffL << 8L)) >> 8L),
        ((MS0_P3 & (0xffL << 0L)) >> 0L),
        (((MS0_P1 & (3L << 16L)) >> 16L) | 0b00000000),
        ((MS0_P1 & (0xffL << 8L)) >> 8L),
        ((MS0_P1 & (0xffL << 0L)) >> 0L), 
        (((MS0_P3 & (15L << 16L)) >> 12L) | ((MS0_P2 & (15L << 16L)) >> 16L)),
        ((MS0_P2 & (0xffL << 8L)) >> 8L),
        ((MS0_P2 & (0xffL << 0L)) >> 0L),
    }); //MS0_P3[15:8] ; MS0_P3[7:0] ; R0_DIV[2:0];MS0_DIVBY4[1:0];MS0_P1[17:16] ; MS0_P1[15:8] ; MS0_P1[7:0] ; MS0_P3[19:16];MS0_P2[19:16] ; MS0_P2[15:8] ; MS0_P2[7:0]
    syncI2CWrite(addr, 50, 8, (uint8_t[]){
        ((MS1_P3 & (0xffL << 8L)) >> 8L),
        ((MS1_P3 & (0xffL << 0L)) >> 0L),
        (((MS1_P1 & (3L << 16L)) >> 16L) | 0b00000000),
        ((MS1_P1 & (0xffL << 8L)) >> 8L),
        ((MS1_P1 & (0xffL << 0L)) >> 0L), 
        (((MS1_P3 & (15L << 16L)) >> 12L) | ((MS1_P2 & (15L << 16L)) >> 16L)),
        ((MS1_P2 & (0xffL << 8L)) >> 8L),
        ((MS1_P2 & (0xffL << 0L)) >> 0L),
    }); //MS1_P3[15:8] ; MS1_P3[7:0] ; R1_DIV[2:0];MS1_DIVBY4[1:0];MS1_P1[17:16] ; MS1_P1[15:8] ; MS1_P1[7:0] ; MS1_P3[19:16];MS1_P2[19:16] ; MS1_P2[15:8] ; MS1_P2[7:0]
    syncI2CWrite(addr, 15, 1, (uint8_t[]){0x00}); //select xtal source for both PLLs
    syncI2CWrite(addr, 16, 1, (uint8_t[]){0b00001111}); //configure CLK0(PLLA)
    syncI2CWrite(addr, 17, 1, (uint8_t[]){0b00001111}); //configure CLK1(PLLA)
    syncI2CWrite(addr, 24, 1, (uint8_t[]){0b00000000}); //configure CLKs disable state
    syncI2CWrite(addr, 177, 1, (uint8_t[]){0xAC}); //soft reset PLLs
    syncI2CWrite(addr, 3, 1, (uint8_t[]){0b11111100}); //enable CLK0,CLK1
}

//Enable/Disable SI5351 clock output for TX
void SI5351SetTxClk(bool txClk) {
    //TODO
}

//Enable/Disable SI5351 clock output for RX
void SI5351SetRxClk(bool rxClk) {
    //TODO
}

//Generate 16 bits of CRC from UART command buffer
uint16_t generateCRC(uint8_t sym_count) {
    //TODO
    return 0x00;
}

//Check 16 bits of CRC from UART command buffer
bool checkCRC(uint8_t sym_count) {
    //TODO
    return true;
}

//Generate TX buffer with data
void generateTxBufferData(uint8_t sym_count) {
    ModemRXTXBufferLen = 0;
    ModemRXTXBufferPtr = 0;
    for(int i = 0; i < 16; i++) {
        ModemRXTXBuffer[ModemRXTXBufferLen++] = 0; //generate 16 zeroes as preamble
    }
    for(int i = 0; i < sizeof(SYNCWORD_CONST); i++) {
        char curr_byte = (SYNCWORD_CONST & (0xff << (i * 8))) >> (i * 8);
        for(int k = 0; k < 8; k++) {
            ModemRXTXBuffer[ModemRXTXBufferLen++] = (curr_byte & (1 << k)) >> k; //add syncword bit by bit
        }
    }
    for(int i = 0; i < 8; i++) {
       ModemRXTXBuffer[ModemRXTXBufferLen++] = 0x00; //generate 8 bits of type(0 for message)
    }
    uint8_t len_write = sym_count + 2; //data length + 2 checksym bytes
    for(int i = 0; i < 8; i++) {
        for(int k = 0; k < 8; k++) {
            ModemRXTXBuffer[ModemRXTXBufferLen++] = (len_write & (1 << i)) >> i; //write 64 bits of length
        }
    }
    for(int i = 0; i < sym_count; i++) {
        char curr_byte = UARTCommandBuffer[i];
        for(int k = 0; k < 8; k++) {
            ModemRXTXBuffer[ModemRXTXBufferLen++] = (curr_byte & (1 << k)) >> k; //add message bit by bit
        }
    }
    uint16_t crc = generateCRC(sym_count);
    for(int i = 0; i < 16; i++) {
        ModemRXTXBuffer[ModemRXTXBufferLen++] = (crc & (1 << i)) >> i; //add CRC bit by bit
    }
}

//Generate TX buffer with ACK packet
void generateTxBufferAck() {
    ModemRXTXBufferLen = 0;
    for(int i = 0; i < 16; i++) {
        ModemRXTXBuffer[ModemRXTXBufferLen++] = 0; //generate 16 zeroes as preamble
    }
    for(int i = 0; i < sizeof(SYNCWORD_CONST); i++) {
        char curr_byte = (SYNCWORD_CONST & (0xff << (i * 8))) >> (i * 8);
        for(int k = 0; k < 8; k++) {
            ModemRXTXBuffer[ModemRXTXBufferLen++] = (curr_byte & (1 << k)) >> k; //add syncword bit by bit
        }
    }
    for(int i = 0; i < 8; i++) {
       ModemRXTXBuffer[ModemRXTXBufferLen++] = 0x01; //generate 8 bits of type(0 for message)
    }
}

//Transmit generated sequence from ModemRXTXBuffer
void startTransmitting() {
    SI5351SetTxClk(true); //enable TX clock generator
    PORTB |= (1 << 2); //enable TX amplifier power
    _delay_ms(10); //wait for power transistor to open
    TCCR0A = (1 <<  WGM01); //CTC mode
    TCCR0B = (1 << CS01) | (1 << CS00); //FClk/64
    OCR0A = 94;
    TX_phase = 0;
    TX_bithalf = 0;
    if((StatusByte & 0b10)) {
        //Receiver enabled, disable it when transmitting
        SI5351SetRxClk(false); //disable RX clock generator
        TIMSK1 = 0; //disable timer1 ocrb interrupt
        PORTD &= ~(1 << 2); //clear dcd
        PORTD &= ~(1 << 6); //disable RX led
    }
    PORTD |= (1 << 7); //enable TX LED
    writeToUart('t');writeNLUart();
    TIMSK0 = (1 << OCIE0A); //enable timer 0 compare A interrupt
}

void commandWrite() {
    uint8_t sym_count = 0;
    while(sym_count < 80) {
        char c = readFromUart();
        if(c == '\r')
            continue; //ignore carriage return
        if(c == '\n')
            break; //finish reading
        if(c == '\b' || c == 0x7F) {
            if(sym_count != 0) {
                sym_count--;//don't try to erase 0 symbols
            }
            continue;
        }
        UARTCommandBuffer[sym_count++] = c;
    }
    writeToUart('c');writeNLUart();
    for(int i = 0; i < sym_count; i++) {
        writeToUart(UARTCommandBuffer[i]);
    }
    writeNLUart();
    char c = 0, d;
    while(true) {
        d = readFromUart(); //wait for s\r\n or s\n
        if(d != '\r' && d != '\n') {
            c = d;
        } else if(d == '\n') {
            break;
        }
    }
    if(c == 's') {
        generateTxBufferData(sym_count);
        startTransmitting();
    }
}

void commandAck() {
    generateTxBufferAck();
    startTransmitting();
}

void commandHelp() {
    for(int i = 0; i < sizeof(strHelp); i++) {
        writeToUart(strHelp[i]);
    }
}

void commandSetReceive(bool receiveEnabled) {
    if((StatusByte & 0b10) != receiveEnabled) {
        if(receiveEnabled) {
            SI5351SetRxClk(true); //enable RX clock generator
            //Interrupt setup
            EICRA = (1 << ISC10); //any logical level change will cause INT1
            EIMSK = (1 << INT1); //enable interrupts on INT1(PORTD3)
            //Timer setup
            TCCR1A = 0;
            TCCR1B = (1 << WGM12) | (1 << CS11); //FClk/8, CTC mode
            OCR1A = 750; //750 = 0.5ms
            OCR1B = 375; //call interrupt on half timer
            ModemRXTXBufferLen = 0; //clear pointer
            RX_phase = false;
            RX_prev_value = false;
            RX_syncword_received = false;
            TIMSK1 = (1 << OCIE1B); //enable timer1 ocrb interrupt
            StatusByte |= 0b10;
        } else {
            SI5351SetRxClk(false); //disable RX clock generator
            TIMSK1 = 0; //disable timer1 ocrb interrupt
            PORTD &= ~(1 << 2); //clear dcd
            PORTD &= ~(1 << 6); //disable RX led
            StatusByte &= ~0b10;
        }
    }
}

void commandSetEcho(bool echoEnabled) {
    if((StatusByte & 0b01) != echoEnabled) {
        if(echoEnabled) {
            StatusByte |= 0b01;
        } else {
            StatusByte &= ~0b01;
        }
    }
}

//Parse, process, and answer command from uart
void parseCommand() {
    uint8_t sym_count = 0;
    while(sym_count < 4) {
        char c = readFromUart();
        if(c == '\r')
            continue; //ignore carriage return
        if(c == '\n')
            break; //finish reading
        if(c == '\b' || c == 0x7F) {
            if(sym_count != 0) {
                sym_count--;//don't try to erase 0 symbols
            }
            continue;
        }
        UARTCommandBuffer[sym_count++] = c;
    }
    if(sym_count == 1) {
        switch(UARTCommandBuffer[0]) {
            case 'w':
                writeToUart('o');writeNLUart();
                commandWrite();
                return;
            case 'a':
                writeToUart('o');writeNLUart();
                commandAck();
                return;
            case '?':
                writeToUart('o');writeNLUart();
                commandHelp();
                return;
        }
    } else if(sym_count == 2) {
        switch(UARTCommandBuffer[0]) {
            case 'r':
                switch(UARTCommandBuffer[1]) {
                    case 'e':
                        writeToUart('o');writeNLUart();
                        commandSetReceive(true);
                        return;
                    case 'd':
                        writeToUart('o');writeNLUart();
                        commandSetReceive(false);
                        return;
                }
                break;
            case 'e':
                switch(UARTCommandBuffer[1]) {
                    case 'e':
                        writeToUart('o');writeNLUart();
                        commandSetEcho(true);
                        return;
                    case 'd':
                        writeToUart('o');writeNLUart();
                        commandSetEcho(false);
                        return;
                }
                break;
        }
    }
    writeToUart('e');writeNLUart();
}

//Main function
int main () {
    DDRB = 0b00000110;
    DDRC = 0b00000000;
    DDRD = 0b11100100; //setup ports I/O modes
    PORTD &= ~(7 << 5); //reset all LEDs
    PORTB &= ~(3 << 1); //reset TX power/data
    setupUART();
    sei(); //enable interrupts
    setupSI5351();
    waitForStart();
    while(true) {
        parseCommand();
    }
}

//UART rx complete interrupt
ISR(USART_RX_vect) {
    UARTRxRingBuff[UARTRxRingBuffWritePtr++] = UDR0;
    if(UARTRxRingBuffReadPtr == UARTRxRingBuffWritePtr) {
        UARTRxRingBuffWritePtr--;
    }
}

//UART udr empty interrupt
ISR(USART_UDRE_vect) {
    if(UARTTxRingBuffWritePtr != UARTTxRingBuffReadPtr) {
        UDR0 = UARTTxRingBuff[UARTTxRingBuffReadPtr++];
    } else {
        UCSR0B &= ~(1 << UDRIE0); //disable tx complete interrupt, to avoid endless interrupt calling
    }
}

//Timer 0 compare A interrupt(TX)
ISR(TIMER0_COMPA_vect) {
    bool curr_bit = ModemRXTXBuffer[ModemRXTXBufferPtr] > 0;
    curr_bit = (TX_phase != curr_bit); //calculate current phase
    if(!TX_bithalf) {
        //If first half-bit, transmit non-modified and not decrement buffer pointer
        if(curr_bit) {
            PORTB |= (1 << 1);
        } else {
            PORTB &= ~(1 << 1);
        }
    } else {
        //If last half-bit, transmit inverted bit and decrement buffer pointer
        if(!curr_bit) {
            PORTB |= (1 << 1);
        } else {
            PORTB &= ~(1 << 1);
        }
        TX_phase = curr_bit; //store current phase as previous
        ModemRXTXBufferPtr++; //increment buffer pointer
        if(ModemRXTXBufferPtr >= ModemRXTXBufferLen) {
            //End of buffer, disable timer, write to uart
            PORTB &= ~(1 << 2); //disable TX amplifier power
            PORTB &= ~(1 << 1); //reset TX data
            SI5351SetTxClk(false);
            PORTD &= ~(1 << 7); //disable TX LED
            TIMSK0 = 0;
            writeToUart('f');writeNLUart();
            if((StatusByte & 0b10)) {
                //Receiver was enabled, re-enable it
                RX_phase = false;
                RX_prev_value = false;
                RX_syncword_received = false;
                PORTD &= ~(1 << 2); //clear dcd
                PORTD &= ~(1 << 6); //disable RX led
                SI5351SetRxClk(true); //enable RX clock generator
                TIMSK1 = (1 << OCIE1B); //enable timer1 ocrb interrupt
            }
        }
    }
    TX_bithalf = !TX_bithalf;
}

//Timer 1 compare B interrupt(RX)
ISR(TIMER1_COMPB_vect) {
    RX_phase = !RX_phase;
    if(RX_phase) {
        bool current_value = (PIND & (1 << 3)) >> 3;
        if(RX_syncword_received) {
            //Syncword already received
            PORTD |= (1 << 2); //set dcd
            PORTD |= (1 << 6); //enable RX led
            RX_byte_receiving <<= 1;
            if(current_value != RX_prev_value) {
                RX_byte_receiving |= (1 << 0);
            } else {
                RX_byte_receiving &= ~(1 << 0);
            }
            RX_byte_bit_number++;
            if(RX_byte_bit_number > 7) {
                //Byte receiving finished, process it
                RX_byte_number++;
                //Byte numbers: (1)-type, (2-9)-len, (10-(10+len))-data
                if(RX_byte_number == 1) {
                    int bits_count = 0;
                    for(int i = 0; i < 8; i++) {
                        if(RX_byte_receiving & (1 << i)) {
                            bits_count++;
                        }
                    }
                    if(bits_count >= 4) {
                        //Type is ACK, stop receiving
                        writeToUart('a');writeNLUart();
                        RX_syncword_received = false;
                    }
                    //Else, type is data, continue receiving
                } else if(RX_byte_number <= 9) {
                    RX_packet_length <<= 1;
                    int bits_count = 0;
                    for(int i = 0; i < 8; i++) {
                        if(RX_byte_receiving & (1 << i)) {
                            bits_count++;
                        }
                    }
                    if(bits_count >= 4) {
                        RX_packet_length |= (1 << 0);
                    }
                } else {
                    if(ModemRXTXBufferLen < RX_packet_length) {
                        ModemRXTXBuffer[ModemRXTXBufferLen] = RX_byte_receiving;
                        ModemRXTXBufferLen++;
                    } else {
                        //Packet receiving finished!
                        if(checkCRC(ModemRXTXBufferLen)) {
                            writeToUart('r');writeNLUart();
                            for(int i = 0; i < ModemRXTXBufferLen; i++) {
                                writeToUart(ModemRXTXBuffer[i]);
                            }
                            writeNLUart();
                            RX_syncword_received = false;
                        }
                    }
                }
                RX_byte_bit_number = 0;
                RX_byte_receiving = 0;
            }
        } else {
            //Syncword hasn't received yet
            PORTD &= ~(1 << 2); //clear dcd
            PORTD &= ~(1 << 6); //disable RX led
            RX_syncword_buffer <<= 1;
            if(current_value != RX_prev_value) {
                RX_syncword_buffer |= (1 << 0);
            } else {
                RX_syncword_buffer &= ~(1 << 0);
            }
            int errors = 0;
            for(int i = 0; i < sizeof(SYNCWORD_CONST); i++) {
                uint8_t sw_byte = (SYNCWORD_CONST & (0xff << (i * 8))) >> (i * 8);
                uint8_t received = (RX_syncword_buffer & (0xff << (i * 8))) >> (i * 8);
                uint8_t diff = sw_byte ^ received;
                if(diff != 0) {
                    for(int k = 0; k < 8; k++) {
                        if(diff & (1 << k)) {
                            errors++;
                        }
                    }
                }
            }
            if(errors < 3) {
                //Got syncword with less than 3 errors! Entering bit-receiving mode
                RX_syncword_received = true;
                RX_byte_number = 0;
                RX_byte_receiving = 0;
                RX_byte_bit_number = 0;
                RX_packet_length = 0;
                ModemRXTXBufferLen = 0;
            }
        }
        RX_prev_value = current_value;
    }
}

//External interrupt 1(RX)
ISR(INT1_vect) {
    TCNT1 = 0; //reset timer on any logical change
}
