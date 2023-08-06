/**
 * `murmurhash.h' - murmurhash
 *
 * copyright (c) 2014-2022 joseph werle <joseph.werle@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "murmurhash.h"

#ifdef __aarch64__
#include <arm_neon.h>
#endif

uint32_t
murmurhash (const char *key, uint32_t len, uint32_t seed) {
  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;
  uint32_t r1 = 15;
  uint32_t r2 = 13;
  uint32_t m = 5;
  uint32_t n = 0xe6546b64;
  uint32_t h = 0;
  uint32_t k = 0;
  uint8_t *d = (uint8_t *) key; // 32 bit extract from `key'
  const uint32_t *chunks = NULL;
  const uint8_t *tail = NULL; // tail - last 8 bytes
  int i = 0;
  int l = len / 4; // chunk length

  h = seed;

#ifdef __aarch64__
  int len_x4 = len / 16;
  int len_remain = len % 16;

  if (len_x4 != 0) {
    const uint32_t *chunk_index = (const uint32_t *)key;
    uint32x4_t v_c1 = vdupq_n_u32(c1);
    uint32x4_t v_c2 = vdupq_n_u32(c2);
    for (int i = 0; i < len_x4; i++) {
      uint32x4_t v_key = vld1q_u32(chunk_index);
      v_key = vmulq_u32(v_key, v_c1);
      uint32x4_t v_tmp1 = vshlq_u32(v_key, r1);
      uint32x4_t v_tmp2 = vushrq_u32(v_key, 32 - r1);
      v_key = vorrq_u32(v_tmp1, v_tmp2);
      v_key = vmulq_u32(v_key, v_c2);

      for (int j = 0; j < 4; j++) {
        h ^= v_key[j];
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
      }
    }
  }

  int len_x1 = len_remain / 4;
  chunks = (const uint32_t *)(d + len_x4 * 16 + len_x1 * 4);
  tail = (const uint8_t *)(d + len_x4 * 16 + len_x1 * 4);

  for (int i  = -len_x1; i != 0; ++i) {
    k = chunks[i];
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;

    h ^= k;
    h = (h << r2) | (h >> (32 - r2));
    h = h * m + n;
  }

  k = 0;

  // remainder
  switch (len & 3) { // `len % 4'
    case 3: k ^= (tail[2] << 16);
    case 2: k ^= (tail[1] << 8);

    case 1:
      k ^= tail[0];
      k *= c1;
      k = (k << r1) | (k >> (32 - r1));
      k *= c2;
      h ^= k;
  }

  h ^= len;

  h ^= (h >> 16);
  h *= 0x85ebca6b;
  h ^= (h >> 13);
  h *= 0xc2b2ae35;
  h ^= (h >> 16);

  return h;
#else

  chunks = (const uint32_t *) (d + l * 4); // body
  tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `key'

  // for each 4 byte chunk of `key'
  for (i = -l; i != 0; ++i) {
    // next 4 byte chunk of `key'
    k = chunks[i];

    // encode next 4 byte chunk of `key'
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;

    // append to hash
    h ^= k;
    h = (h << r2) | (h >> (32 - r2));
    h = h * m + n;
  }

  k = 0;

  // remainder
  switch (len & 3) { // `len % 4'
    case 3: k ^= (tail[2] << 16);
    case 2: k ^= (tail[1] << 8);

    case 1:
      k ^= tail[0];
      k *= c1;
      k = (k << r1) | (k >> (32 - r1));
      k *= c2;
      h ^= k;
  }

  h ^= len;

  h ^= (h >> 16);
  h *= 0x85ebca6b;
  h ^= (h >> 13);
  h *= 0xc2b2ae35;
  h ^= (h >> 16);

  return h;
#endif
}
