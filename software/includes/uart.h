#pragma once

volatile char UARTRxRingBuff[256];
volatile uint8_t UARTRxRingBuffReadPtr = 0;
volatile uint8_t UARTRxRingBuffWritePtr = 0;
volatile char UARTTxRingBuff[256];
volatile uint8_t UARTTxRingBuffReadPtr = 0;
volatile uint8_t UARTTxRingBuffWritePtr = 0;
volatile bool UARTechoenabled = true;

//Setup UART module
void uart_setup() {
    UCSR0A = (1 << U2X0); //double the USART transmission speed
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0) | (1 << UDRIE0); //enable transmitter; enable receiver; enable rx complete interrupt
    UCSR0C = (3 << UCSZ00); //8-bit character size
    static_assert((F_CPU == 12000000UL), "CPU frequency is not 12M!");
    UBRR0 = 12; //115200 b/s while F_CPU==12M
    UARTRxRingBuffReadPtr = UARTRxRingBuffWritePtr = UARTTxRingBuffReadPtr = UARTTxRingBuffWritePtr = 0;
    UARTechoenabled = true;
}

//Set UART symbol echo enabled
void uart_set_echo(bool echo) {
    UARTechoenabled = echo;
}

//Write char to UART, buffer overflows ignored
void uart_write_char(char c) {
    UARTTxRingBuff[UARTTxRingBuffWritePtr++] = c;
    if(UARTTxRingBuffWritePtr == UARTTxRingBuffReadPtr) {
        UARTTxRingBuffWritePtr--;
    }
    UCSR0B |= (1 << UDRIE0); //allow UART UDR Empty interrupt
}

//Print uint to UART as decimal number
void uart_write_uint(unsigned int v) {
    char a[32];
    itoa(v, a, 10);
    for(int i = 0; i < 32; i++) {
        char c = a[i];
        if(c == '\0') {
            break;
        }
        uart_write_char(c);
    }
}

//Write CR+LF to UART
void uart_write_newline() {
    uart_write_char('\r');
    uart_write_char('\n');
}

//Write null-terminated string to UART
void uart_write_cstring(char* c) {
    while(*(c++) != 0) {uart_write_char(*c);}
}

//Read char from uart, blocking. Print back if echo enabled.
char uart_read_char() {
    while(UARTRxRingBuffReadPtr == UARTRxRingBuffWritePtr) {}
    char c = UARTRxRingBuff[UARTRxRingBuffReadPtr++];
    if(UARTechoenabled) {
        uart_write_char(c);
    }
    return c;
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
