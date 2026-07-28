// =============================================================================
// libc/stdio.cpp -- Implementation of kprintf
// =============================================================================
#include "stdio.h"
#include "kernel/types.h"
#include "drivers/serial.h"
#include <stdarg.h>   // freestanding-safe: provided by the compiler, not glibc

namespace {

void print_uint(unsigned int value, unsigned int base, bool uppercase) {
    char buf[32];
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    if (value == 0) buf[i++] = '0';
    while (value > 0) {
        buf[i++] = digits[value % base];
        value /= base;
    }
    while (i > 0) serial::write_char(buf[--i]);
}

void print_int(int value) {
    if (value < 0) {
        serial::write_char('-');
        print_uint((unsigned int)(-value), 10, false);
    } else {
        print_uint((unsigned int)value, 10, false);
    }
}

} // namespace

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char* p = fmt; *p; p++) {
        if (*p != '%') {
            serial::write_char(*p);
            continue;
        }
        p++;
        switch (*p) {
            case 'd': print_int(va_arg(args, int)); break;
            case 'u': print_uint(va_arg(args, unsigned int), 10, false); break;
            case 'x': print_uint(va_arg(args, unsigned int), 16, false); break;
            case 'X': print_uint(va_arg(args, unsigned int), 16, true); break;
            case 'p':
                serial::write("0x");
                print_uint((unsigned int)va_arg(args, void*), 16, false);
                break;
            case 's': serial::write(va_arg(args, const char*)); break;
            case 'c': serial::write_char((char)va_arg(args, int)); break;
            case '%': serial::write_char('%'); break;
            default:
                serial::write_char('%');
                serial::write_char(*p);
        }
    }

    va_end(args);
}
