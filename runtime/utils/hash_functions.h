#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <rte_config.h>
#include <rte_hash_crc.h>

#include "../bessport/utils/common.h"

inline int uint32_keycmp(const void *key, const void *key_stored, size_t key_len) {
  const uint32_t *a = reinterpret_cast<const uint32_t*>(key);
  const uint32_t *b = reinterpret_cast<const uint32_t*>(key_stored);

  if(unlikely(*a != *b)){
    return 1;
  }

  return 0;
}

inline uint32_t uint32_hash(const void *key, uint32_t key_len, uint32_t init_val) {
#if __SSE4_2__ && __x86_64
  const uint32_t *a = reinterpret_cast<const uint32_t*>(key);

  init_val = crc32c_sse42_u32(*a, init_val);

  return init_val;
#else
  return rte_hash_crc(key, key_len, init_val);
#endif
}

inline int uint64_keycmp(const void *key, const void *key_stored, size_t key_len) {
  const uint64_t *a = reinterpret_cast<const uint64_t*>(key);
  const uint64_t *b = reinterpret_cast<const uint64_t*>(key_stored);

  if(unlikely(*a != *b)){
    return 1;
  }

  return 0;
}

inline uint32_t uint64_hash(const void *key, uint32_t key_len, uint32_t init_val) {
#if __SSE4_2__ && __x86_64
  const uint64_t *a = reinterpret_cast<const uint64_t*>(key);

  init_val = crc32c_sse42_u64(*a, init_val);

  return init_val;
#else
  return rte_hash_crc(key, key_len, init_val);
#endif
}

#endif
