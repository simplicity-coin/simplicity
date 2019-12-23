#if defined(_M_X64) || defined(__x86_64__)
#include "crypto/RandomX/src/jit_compiler_x86_static.S"
#elif defined(__aarch64__)
#include "crypto/RandomX/src/jit_compiler_a64_static.S"
#endif
