// =============================================================================
// drivers/mouse.cpp -- PS/2 mouse driver implementation
// =============================================================================
#include "mouse.h"
#include "kernel/io.h"
#include "kernel/idt.h"
#include "libc/stdio.h"

namespace {
    constexpr uint16_t PS2_DATA    = 0x60;
    constexpr uint16_t PS2_STATUS  = 0x64;
    constexpr uint16_t PS2_COMMAND = 0x64;

    int screenW = 320, screenH = 200;
    int cursorX = 160, cursorY = 100;
    bool btnLeft = false, btnRight = false;

    uint8_t packet[3];
    int packet_index = 0;

    void wait_write() {
        int timeout = 100000;
        while (timeout-- && (inb(PS2_STATUS) & 2)) {}
    }
    void wait_read() {
        int timeout = 100000;
        while (timeout-- && !(inb(PS2_STATUS) & 1)) {}
    }

    void mouse_write(uint8_t data) {
        wait_write();
        outb(PS2_COMMAND, 0xD4); // "next byte goes to the mouse"
        wait_write();
        outb(PS2_DATA, data);
    }

    uint8_t mouse_read() {
        wait_read();
        return inb(PS2_DATA);
    }

    void irq12_handler(Registers*) {
        uint8_t data = inb(PS2_DATA);
        packet[packet_index++] = data;

        if (packet_index == 3) {
            packet_index = 0;

            uint8_t flags = packet[0];
            if (flags & 0x08) { // always-1 bit sanity check: valid packet
                int8_t dx = (int8_t)packet[1];
                int8_t dy = (int8_t)packet[2];

                cursorX += dx;
                cursorY -= dy; // PS/2 Y axis is inverted relative to screen Y

                if (cursorX < 0) cursorX = 0;
                if (cursorY < 0) cursorY = 0;
                if (cursorX >= screenW) cursorX = screenW - 1;
                if (cursorY >= screenH) cursorY = screenH - 1;

                btnLeft = flags & 0x01;
                btnRight = flags & 0x02;
            }
        }
    }
}

namespace mouse {

void init(int screen_width, int screen_height) {
    screenW = screen_width;
    screenH = screen_height;
    cursorX = screen_width / 2;
    cursorY = screen_height / 2;

    // Enable the auxiliary PS/2 device (the mouse port on the 8042 controller).
    wait_write(); outb(PS2_COMMAND, 0xA8);

    // Enable IRQ12 in the controller configuration byte.
    wait_write(); outb(PS2_COMMAND, 0x20);
    uint8_t status = mouse_read() | 0x02;
    wait_write(); outb(PS2_COMMAND, 0x60);
    wait_write(); outb(PS2_DATA, status);

    // Tell the mouse to use default settings, then enable data reporting.
    mouse_write(0xF6); mouse_read(); // ack
    mouse_write(0xF4); mouse_read(); // ack, start streaming packets

    idt::register_handler(44, irq12_handler); // IRQ12 -> vector 44
    kprintf("[mouse] PS/2 driver ready\n");
}

int x() { return cursorX; }
int y() { return cursorY; }
bool left_button() { return btnLeft; }
bool right_button() { return btnRight; }

} // namespace mouse
