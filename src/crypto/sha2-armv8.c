/*
 *  ARMv8-A Cryptography Extension SHA256 support functions
 *
 *  Copyright (C) 2016, CriticalBlue Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#include <stdint.h>
#include <string.h>

#if defined(__aarch64__)

#include <arm_neon.h>

static const uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define Rx(T0, T1, K, W0, W1, W2, W3)      \
    W0 = vsha256su0q_u32( W0, W1 );    \
    d2 = d0;                           \
    T1 = vaddq_u32( W1, K );           \
    d0 = vsha256hq_u32( d0, d1, T0 );  \
    d1 = vsha256h2q_u32( d1, d2, T0 ); \
    W0 = vsha256su1q_u32( W0, W2, W3 );

#define Ry(T0, T1, K, W1)                  \
    d2 = d0;                           \
    T1 = vaddq_u32( W1, K  );          \
    d0 = vsha256hq_u32( d0, d1, T0 );  \
    d1 = vsha256h2q_u32( d1, d2, T0 );

#define Rz(T0)                             \
    d2 = d0;                           \
    d0 = vsha256hq_u32( d0, d1, T0 );  \
    d1 = vsha256h2q_u32( d1, d2, T0 );

void sha256_transform(uint32_t *state, const uint32_t *block, int swap)
{
    /* declare variables */
    uint32x4_t k0, k1, k2, k3, k4, k5, k6, k7, k8, k9, ka, kb, kc, kd, ke, kf;
    uint32x4_t s0, s1;
    uint32x4_t w0, w1, w2, w3;
    uint32x4_t d0, d1, d2;
    uint32x4_t t0, t1;

    /* set K0..Kf constants */
    k0 = vld1q_u32(&sha256_k[0x00]);
    k1 = vld1q_u32(&sha256_k[0x04]);
    k2 = vld1q_u32(&sha256_k[0x08]);
    k3 = vld1q_u32(&sha256_k[0x0c]);
    k4 = vld1q_u32(&sha256_k[0x10]);
    k5 = vld1q_u32(&sha256_k[0x14]);
    k6 = vld1q_u32(&sha256_k[0x18]);
    k7 = vld1q_u32(&sha256_k[0x1c]);
    k8 = vld1q_u32(&sha256_k[0x20]);
    k9 = vld1q_u32(&sha256_k[0x24]);
    ka = vld1q_u32(&sha256_k[0x28]);
    kb = vld1q_u32(&sha256_k[0x2c]);
    kc = vld1q_u32(&sha256_k[0x30]);
    kd = vld1q_u32(&sha256_k[0x34]);
    ke = vld1q_u32(&sha256_k[0x38]);
    kf = vld1q_u32(&sha256_k[0x3c]);

    /* load state */
    s0 = vld1q_u32(&state[0]);
    s1 = vld1q_u32(&state[4]);

    /* load message */
    w0 = vld1q_u32(block);
    w1 = vld1q_u32(block + 4);
    w2 = vld1q_u32(block + 8);
    w3 = vld1q_u32(block + 12);

    if (swap) {
        w0 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(w0)));
        w1 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(w1)));
        w2 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(w2)));
        w3 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(w3)));
    }

    /* initialize t0, d0, d1 */
    t0 = vaddq_u32(w0, k0);
    d0 = s0;
    d1 = s1;

    /* perform rounds of four */
    Rx(t0, t1, k1, w0, w1, w2, w3);
    Rx(t1, t0, k2, w1, w2, w3, w0);
    Rx(t0, t1, k3, w2, w3, w0, w1);
    Rx(t1, t0, k4, w3, w0, w1, w2);
    Rx(t0, t1, k5, w0, w1, w2, w3);
    Rx(t1, t0, k6, w1, w2, w3, w0);
    Rx(t0, t1, k7, w2, w3, w0, w1);
    Rx(t1, t0, k8, w3, w0, w1, w2);
    Rx(t0, t1, k9, w0, w1, w2, w3);
    Rx(t1, t0, ka, w1, w2, w3, w0);
    Rx(t0, t1, kb, w2, w3, w0, w1);
    Rx(t1, t0, kc, w3, w0, w1, w2);
    Ry(t0, t1, kd, w1);
    Ry(t1, t0, ke, w2);
    Ry(t0, t1, kf, w3);
    Rz(t1);

    /* update state */
    s0 = vaddq_u32(s0, d0);
    s1 = vaddq_u32(s1, d1);

    /* save state */
    vst1q_u32(&state[0], s0);
    vst1q_u32(&state[4], s1);
}
#endif
