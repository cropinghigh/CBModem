#pragma once

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

volatile uint8_t ModemRXTXBuffer[831]; //1byte=1bit
volatile int ModemRXTXBufferLen = 0; //buffer size
volatile int ModemRXTXBufferPtr = 0; //buffer pointer
volatile char ModemRXTXDataByteBuffer[82]; //80 bytes + 2byte checksum

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
