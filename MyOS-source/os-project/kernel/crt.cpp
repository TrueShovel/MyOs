// =============================================================================
// kernel/crt.cpp -- Minimal C++ runtime support
// =============================================================================
// In a normal C++ program, the C runtime (crt0.o, supplied by the OS's libc)
// calls every global/static object's constructor before main() runs. Our
// kernel has no libc, so boot.s calls init_constructors() itself, which
// walks the .init_array section the linker (boot/linker.ld) collected all
// constructor function pointers into.
//
// This also provides the two "pure virtual call" and stack-protector stub
// symbols the compiler can emit references to, so linking succeeds even
// though we don't use those specific C++ ABI features.
// =============================================================================
#include "types.h"

extern "C" {
    typedef void (*ctor_func)();
    extern ctor_func __init_array_start;
    extern ctor_func __init_array_end;

    void init_constructors() {
        for (ctor_func* f = &__init_array_start; f != &__init_array_end; f++) {
            (*f)();
        }
    }

    // Called if a pure virtual function is ever invoked (should never
    // happen if the code is correct, but the symbol must exist for the
    // linker to be happy about vtables).
    void __cxa_pure_virtual() {
        for (;;) { asm volatile ("cli; hlt"); }
    }

    // Required by the Itanium C++ ABI for static locals with non-trivial
    // constructors (thread-safe init guards). We're single-threaded during
    // early boot, so a no-op/simple guard is sufficient.
    int __cxa_guard_acquire(uint64_t* guard) {
        return !(*(char*)guard);
    }
    void __cxa_guard_release(uint64_t* guard) {
        *(char*)guard = 1;
    }
    void __cxa_guard_abort(uint64_t*) {}
}
