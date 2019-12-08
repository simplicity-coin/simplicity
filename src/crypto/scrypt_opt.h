// Copyright (c) 2018-2019 The Simplicity developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SCRYPT_OPT_H
#define SCRYPT_OPT_H

#if defined HAVE_CONFIG_H
#include "config/simplicity-config.h"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <compat/byteswap.h>
#include "uint256.h"
#include "utilstrencodings.h"


//static const int SCRYPT_SCRATCHPAD_SIZE = 134218239;
//static const int N = 1048576;

bool scrypt_N_1_1_256_multi(void *input, uint256 hashTarget, int *nHashesDone, unsigned char *scratchbuf, int N);

bool scryptHash(const void *input, char *output, int N);
extern unsigned char *scrypt_buffer_alloc(int N, bool multiWay = true);
extern "C" void scrypt_core(uint32_t *X, uint32_t *V, int N);
void sha256_init(uint32_t *state);
extern "C" void sha256_transform(uint32_t *state, const uint32_t *block, int swap);

#if defined(__x86_64__) && !defined(ENABLE_AVX2)
#define SCRYPT_MAX_WAYS 12
#define HAVE_SCRYPT_3WAY 1
#define HAVE_SHA256_4WAY 1
#define SCRYPT_BEST_THROUGHPUT 3
extern "C" int sha256_use_4way();
extern "C" void sha256_init_4way(uint32_t *state);
extern "C" void sha256_transform_4way(uint32_t *state, const uint32_t *block, int swap);
extern "C" void scrypt_core_3way(uint32_t *X, uint32_t *V, int N);
#endif

#if defined(__x86_64__) && defined(ENABLE_AVX2)
#define SCRYPT_MAX_WAYS 12
#define HAVE_SCRYPT_3WAY 1
#define HAVE_SHA256_4WAY 1
#define SCRYPT_BEST_THROUGHPUT 3
extern "C" int sha256_use_4way();
extern "C" void sha256_init_4way(uint32_t *state);
extern "C" void sha256_transform_4way(uint32_t *state, const uint32_t *block, int swap);
extern "C" void scrypt_core_3way(uint32_t *X, uint32_t *V, int N);

//#define SCRYPT_MAX_WAYS 24
#define HAVE_SCRYPT_6WAY 1
//#define HAVE_SHA256_4WAY 1
#define HAVE_SHA256_8WAY 1
//#define SCRYPT_BEST_THROUGHPUT 6
extern "C" int sha256_use_8way();
extern "C" void sha256_init_8way(uint32_t *state);
extern "C" void sha256_transform_8way(uint32_t *state, const uint32_t *block, int swap);
//extern "C" int sha256_use_4way();
//extern "C" void sha256_init_4way(uint32_t *state);
//extern "C" void sha256_transform_4way(uint32_t *state, const uint32_t *block, int swap);
extern "C" void scrypt_core_6way(uint32_t *X, uint32_t *V, int N);
#endif

#if defined(__i386__)
#define SCRYPT_MAX_WAYS 4
#define HAVE_SHA256_4WAY 1
#define SCRYPT_BEST_THROUGHPUT 1
extern "C" int sha256_use_4way();
extern "C" void sha256_init_4way(uint32_t *state);
extern "C" void sha256_transform_4way(uint32_t *state, const uint32_t *block, int swap);
#endif

#if defined(__arm__) && defined(__APCS_32__)
#if !defined(__ARM_NEON)
#define SCRYPT_MAX_WAYS 1
#define SCRYPT_BEST_THROUGHPUT 1
#else
#define SCRYPT_MAX_WAYS 12
#define HAVE_SCRYPT_3WAY 1
#define HAVE_SHA256_4WAY 1
#define SCRYPT_BEST_THROUGHPUT 3
extern "C" int sha256_use_4way();
extern "C" void sha256_init_4way(uint32_t *state);
extern "C" void sha256_transform_4way(uint32_t *state, const uint32_t *block, int swap);
extern "C" void scrypt_core_3way(uint32_t *X, uint32_t *V, int N);
#endif
#endif

#if defined(__aarch64__)
#define SCRYPT_MAX_WAYS 3
#define HAVE_SCRYPT_3WAY 1
#define SCRYPT_BEST_THROUGHPUT 3
extern "C" void scrypt_core_3way(uint32_t *X, uint32_t *V, int N);
#endif

static inline uint32_t swab32(uint32_t v)
{
    return bswap_32(v);
}
#endif
