#if defined (__x86_64__)
#pragma message("Building Argon2 with SIMD optimizations")
#include "crypto/argon2/src/opt.c"
#else
#pragma message("Building Argon2 without SIMD optimizations")
#include "crypto/argon2/src/ref.c"
#endif
