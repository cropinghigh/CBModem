#define F_CPU 12000000UL

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "includes/i2c.h"
#include "includes/si5351.h"
#include "includes/uart.h"
#include "includes/dsp.h"

//1-4 bytes
#define SYNCWORD_CONST 0b11011000111101000010100011010011

#define SI5351_I2C_ADDR 0b1100000 //si5351 addr

const char strAbout[] = "HF modem firmware. Author: https://github.com/cropinghigh";

const char strHelp[] = "Help: \n\rw - enter text to transmit\n\rre - enable receiving\n\rrd - disable receiving\n\ree - enable echo\n\red - disable echo\n\r";


char UARTCommandBuffer[80];

uint8_t StatusByte = 0b01; //0 bit - is echo enabled, 1 bit - is receiving enabled

// //Do init sequence
// void waitForStart() {
//     writeToUart('s');writeNLUart();
// _sym1:
//     char c = readFromUart(); //wait for s\r\n or s\n
//     if(c != 's')
//         goto _sym1;
//     c = readFromUart();
//     if(c == '\n')
//         goto _end;
//     if(c != '\r')
//         goto _sym1;
//     c = readFromUart();
//     if(c != '\n')
//         goto _sym1;
// _end:
//     writeToUart('o');writeNLUart();
//     PORTD |= (1 << 5); //enable USB led
// }
// 
// void commandWrite() {
//     uint8_t sym_count = 0;
//     while(sym_count < 80) {
//         char c = readFromUart();
//         if(c == '\r')
//             continue; //ignore carriage return
//         if(c == '\n')
//             break; //finish reading
//         if(c == '\b' || c == 0x7F) {
//             if(sym_count != 0) {
//                 sym_count--;//don't try to erase 0 symbols
//             }
//             continue;
//         }
//         UARTCommandBuffer[sym_count++] = c;
//     }
//     writeToUart('c');writeNLUart();
//     for(int i = 0; i < sym_count; i++) {
//         writeToUart(UARTCommandBuffer[i]);
//     }
//     writeNLUart();
//     char c = 0, d;
//     while(true) {
//         d = readFromUart(); //wait for s\r\n or s\n
//         if(d != '\r' && d != '\n') {
//             c = d;
//         } else if(d == '\n') {
//             break;
//         }
//     }
//     if(c == 's') {
//         generateTxBufferData(sym_count);
//         startTransmitting();
//     }
// }
// 
// void commandAck() {
//     generateTxBufferAck();
//     startTransmitting();
// }
// 
// void commandHelp() {
//     for(int i = 0; i < sizeof(strHelp); i++) {
//         writeToUart(strHelp[i]);
//     }
// }
// 
// void commandSetReceive(bool receiveEnabled) {
//     if((StatusByte & 0b10) != receiveEnabled) {
//         if(receiveEnabled) {
//             SI5351SetRxClk(true); //enable RX clock generator
//             //Interrupt setup
//             EICRA = (1 << ISC10); //any logical level change will cause INT1
//             EIMSK = (1 << INT1); //enable interrupts on INT1(PORTD3)
//             //Timer setup
//             TCCR1A = 0;
//             TCCR1B = (1 << WGM12) | (1 << CS11); //FClk/8, CTC mode
//             OCR1A = 750; //750 = 0.5ms
//             OCR1B = 375; //call interrupt on half timer
//             ModemRXTXBufferLen = 0; //clear pointer
//             RX_phase = false;
//             RX_prev_value = false;
//             RX_syncword_received = false;
//             TIMSK1 = (1 << OCIE1B); //enable timer1 ocrb interrupt
//             StatusByte |= 0b10;
//         } else {
//             SI5351SetRxClk(false); //disable RX clock generator
//             TIMSK1 = 0; //disable timer1 ocrb interrupt
//             PORTD &= ~(1 << 2); //clear dcd
//             PORTD &= ~(1 << 6); //disable RX led
//             StatusByte &= ~0b10;
//         }
//     }
// }
// 
// void commandSetEcho(bool echoEnabled) {
//     if((StatusByte & 0b01) != echoEnabled) {
//         if(echoEnabled) {
//             StatusByte |= 0b01;
//         } else {
//             StatusByte &= ~0b01;
//         }
//     }
// }
// 
// //Parse, process, and answer command from uart
// void parseCommand() {
//     uint8_t sym_count = 0;
//     while(sym_count < 4) {
//         char c = readFromUart();
//         if(c == '\r')
//             continue; //ignore carriage return
//         if(c == '\n')
//             break; //finish reading
//         if(c == '\b' || c == 0x7F) {
//             if(sym_count != 0) {
//                 sym_count--;//don't try to erase 0 symbols
//             }
//             continue;
//         }
//         UARTCommandBuffer[sym_count++] = c;
//     }
//     if(sym_count == 1) {
//         switch(UARTCommandBuffer[0]) {
//             case 'w':
//                 writeToUart('o');writeNLUart();
//                 commandWrite();
//                 return;
//             case 'a':
//                 writeToUart('o');writeNLUart();
//                 commandAck();
//                 return;
//             case '?':
//                 writeToUart('o');writeNLUart();
//                 commandHelp();
//                 return;
//         }
//     } else if(sym_count == 2) {
//         switch(UARTCommandBuffer[0]) {
//             case 'r':
//                 switch(UARTCommandBuffer[1]) {
//                     case 'e':
//                         writeToUart('o');writeNLUart();
//                         commandSetReceive(true);
//                         return;
//                     case 'd':
//                         writeToUart('o');writeNLUart();
//                         commandSetReceive(false);
//                         return;
//                 }
//                 break;
//             case 'e':
//                 switch(UARTCommandBuffer[1]) {
//                     case 'e':
//                         writeToUart('o');writeNLUart();
//                         commandSetEcho(true);
//                         return;
//                     case 'd':
//                         writeToUart('o');writeNLUart();
//                         commandSetEcho(false);
//                         return;
//                 }
//                 break;
//         }
//     }
//     writeToUart('e');writeNLUart();
// }

//Main function
int main () {
    DDRB = 0b00000110;
    DDRC = 0b00000000;
    DDRD = 0b11100100; //setup ports I/O modes
    PORTD &= ~(7 << 5); //reset all LEDs
    PORTB &= ~(3 << 1); //reset TX power/data
    uart_setup();
    sei(); //enable interrupts
    i2c_setup();
    si5351_setup();
    si5351_set_clk_enabled(true);
    while(true) {
    }
}


