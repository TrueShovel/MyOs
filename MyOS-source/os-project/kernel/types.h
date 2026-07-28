// =============================================================================
// types.h -- Fixed-width integer types for the kernel
// =============================================================================
// A freestanding kernel has no access to <cstdint> from a hosted C++ standard
// library, so we define our own minimal fixed-width types here. Every file in
// the kernel includes this instead of <cstdint>.
// =============================================================================
#pragma once

using uint8_t  = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;

using int8_t  = signed char;
using int16_t = signed short;
using int32_t = signed int;
using int64_t = signed long long;

using size_t  = uint32_t;   // 32-bit kernel (i386 target)
using uintptr_t = uint32_t;

#define nullptr_t decltype(nullptr)
