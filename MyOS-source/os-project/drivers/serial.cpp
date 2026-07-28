// =============================================================================
// drivers/serial.cpp -- COM1 serial port driver implementation
// =============================================================================
#include "serial.h"
#include "kernel/io.h"

namespace {
    constexpr uint16_t COM1 = 0x3F8;

    bool transmit_empty() {
        return inb(COM1 + 5) & 0x20;
    }
}

namespace serial {

void init() {
    outb(COM1 + 1, 0x00);    // disable interrupts
    outb(COM1 + 3, 0x80);    // enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // divisor low byte -> 38400 baud
    outb(COM1 + 1, 0x00);    // divisor high byte
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void write_char(char c) {
    while (!transmit_empty()) { /* spin */ }
    outb(COM1, (uint8_t)c);
}

void write(const char* str) {
    while (*str) {
        if (*str == '\n') write_char('\r'); // terminals want CRLF
        write_char(*str++);
    }
}

} // namespace serial
