// =============================================================================
// drivers/keyboard.cpp -- PS/2 keyboard driver implementation
// =============================================================================
#include "keyboard.h"
#include "kernel/io.h"
#include "kernel/idt.h"
#include "libc/stdio.h"

namespace {
    constexpr uint16_t PS2_DATA_PORT = 0x60;

    // US QWERTY scan code set 1 -> ASCII, unshifted.
    const char scancode_ascii[128] = {
        0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0 /*ctrl*/,'a','s','d','f','g','h','j','k','l',';','\'','`',
        0 /*lshift*/,'\\','z','x','c','v','b','n','m',',','.','/',
        0 /*rshift*/,'*',0 /*alt*/,' ',0 /*capslock*/,
        // function keys / rest: unmapped for now
    };

    // Shifted variant for the same layout.
    const char scancode_ascii_shift[128] = {
        0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
        '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
        0,'A','S','D','F','G','H','J','K','L',':','"','~',
        0,'|','Z','X','C','V','B','N','M','<','>','?',
        0,'*',0,' ',0,
    };

    constexpr int QUEUE_SIZE = 256;
    char queue[QUEUE_SIZE];
    int queue_head = 0, queue_tail = 0;

    bool shift_down = false;
    bool ctrl_down = false;

    void push_char(char c) {
        int next = (queue_tail + 1) % QUEUE_SIZE;
        if (next != queue_head) { // drop input if the queue is full
            queue[queue_tail] = c;
            queue_tail = next;
        }
    }

    void irq1_handler(Registers*) {
        uint8_t scancode = inb(PS2_DATA_PORT);

        bool released = scancode & 0x80;
        uint8_t code = scancode & 0x7F;

        // Modifier keys: left/right shift are 0x2A/0x36, ctrl is 0x1D.
        if (code == 0x2A || code == 0x36) { shift_down = !released; return; }
        if (code == 0x1D) { ctrl_down = !released; return; }

        if (released) return; // we only care about key-down for typing

        if (code < 128) {
            char c = shift_down ? scancode_ascii_shift[code] : scancode_ascii[code];
            if (c) push_char(c);
        }
    }
}

namespace keyboard {

void init() {
    idt::register_handler(33, irq1_handler); // IRQ1 -> vector 33
    kprintf("[keyboard] PS/2 driver ready\n");
}

char poll_char() {
    if (queue_head == queue_tail) return 0;
    char c = queue[queue_head];
    queue_head = (queue_head + 1) % QUEUE_SIZE;
    return c;
}

bool is_shift_down() { return shift_down; }
bool is_ctrl_down() { return ctrl_down; }

} // namespace keyboard
