// =============================================================================
// libc/string.h -- Minimal freestanding string & memory utilities
// =============================================================================
// A normal C++ program gets memcpy, strlen, etc. from the C standard library.
// Our kernel has no operating system underneath it (it IS the operating
// system), so we implement the handful of functions we actually need
// ourselves. Add more here as the rest of the OS needs them.
// =============================================================================
#pragma once
#include "kernel/types.h"

extern "C" {
    void*  memset(void* dest, int value, size_t count);
    void*  memcpy(void* dest, const void* src, size_t count);
    void*  memmove(void* dest, const void* src, size_t count);
    int    memcmp(const void* a, const void* b, size_t count);

    size_t strlen(const char* str);
    int    strcmp(const char* a, const char* b);
    int    strncmp(const char* a, const char* b, size_t n);
    char*  strcpy(char* dest, const char* src);
    char*  strncpy(char* dest, const char* src, size_t n);
    char*  strcat(char* dest, const char* src);
    char*  strchr(const char* str, int ch);
}
