// =============================================================================
// apps/terminal.cpp -- Terminal application implementation
// =============================================================================
#include "terminal.h"
#include "drivers/framebuffer.h"
#include "drivers/pit.h"
#include "kernel/memory/pmm.h"
#include "libc/string.h"

using namespace fb::colors;

Terminal::Terminal() {
    push_line("MyOS Terminal -- type 'help' for a list of commands");
    inputLine[0] = '\0';
}

void Terminal::push_line(const char* text) {
    if (historyCount == MAX_LINES) {
        // Scroll: drop the oldest line.
        for (int i = 1; i < MAX_LINES; i++) {
            strncpy(history[i - 1], history[i], LINE_LEN);
        }
        historyCount--;
    }
    strncpy(history[historyCount], text, LINE_LEN - 1);
    history[historyCount][LINE_LEN - 1] = '\0';
    historyCount++;
}

namespace {
    // Minimal manual unsigned-integer-to-string helper, since the kernel has
    // no snprintf. Appends digits to `dest` starting at `*pos`.
    void append_uint(char* dest, int* pos, uint64_t value) {
        char digits[20];
        int len = 0;
        if (value == 0) digits[len++] = '0';
        while (value > 0) { digits[len++] = '0' + (int)(value % 10); value /= 10; }
        for (int k = len - 1; k >= 0; k--) dest[(*pos)++] = digits[k];
    }
    void append_str(char* dest, int* pos, const char* s) {
        while (*s) dest[(*pos)++] = *s++;
    }
}

void Terminal::run_command() {
    char prompt_line[LINE_LEN + 2] = "> ";
    strcat(prompt_line, inputLine);
    push_line(prompt_line);

    if (strcmp(inputLine, "help") == 0) {
        push_line("Commands: help, clear, about, uptime, meminfo, echo <text>");
    } else if (strcmp(inputLine, "clear") == 0) {
        historyCount = 0;
    } else if (strcmp(inputLine, "about") == 0) {
        push_line("MyOS -- a small educational operating system written in C++");
    } else if (strcmp(inputLine, "uptime") == 0) {
        char buf[LINE_LEN];
        int pos = 0;
        append_str(buf, &pos, "Uptime: ");
        append_uint(buf, &pos, pit::ticks() / 100);
        buf[pos++] = 's';
        buf[pos] = '\0';
        push_line(buf);
    } else if (strcmp(inputLine, "meminfo") == 0) {
        char buf[LINE_LEN];
        int pos = 0;
        append_str(buf, &pos, "Free memory: ");
        append_uint(buf, &pos, pmm::free_frames() * 4);
        append_str(buf, &pos, " KiB");
        buf[pos] = '\0';
        push_line(buf);
    } else if (strncmp(inputLine, "echo ", 5) == 0) {
        push_line(inputLine + 5);
    } else if (inputLen == 0) {
        // empty line, do nothing
    } else {
        char buf[LINE_LEN] = "Unknown command: ";
        strcat(buf, inputLine);
        push_line(buf);
    }
}

void Terminal::on_key(char c) {
    if (c == '\n') {
        run_command();
        inputLine[0] = '\0';
        inputLen = 0;
    } else if (c == '\b') {
        if (inputLen > 0) inputLine[--inputLen] = '\0';
    } else if (inputLen < LINE_LEN - 1) {
        inputLine[inputLen++] = c;
        inputLine[inputLen] = '\0';
    }
}

void Terminal::draw(int x, int y, int w, int h) {
    int lineHeight = 12;
    int maxLines = h / lineHeight - 1;

    int start = historyCount > maxLines ? historyCount - maxLines : 0;
    int drawY = y;
    for (int i = start; i < historyCount; i++) {
        fb::draw_string(x, drawY, history[i], TEXT);
        drawY += lineHeight;
    }

    char prompt[LINE_LEN + 2] = "> ";
    strcat(prompt, inputLine);
    fb::draw_string(x, drawY, prompt, ACCENT);
}

App* create_terminal() { return new Terminal(); }
