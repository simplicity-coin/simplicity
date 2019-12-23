#if defined(_M_X64) || defined(__x86_64__)
#include "crypto/RandomX/src/jit_compiler_x86.cpp"
#elif defined(__aarch64__)
#include "crypto/RandomX/src/jit_compiler_a64.cpp"
#else
#include "crypto/RandomX/src/jit_compiler_fallback.hpp"
#endif
