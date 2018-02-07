#ifndef FLOW_HASH_H
#define FLOW_HASH_H

#include <rte_config.h>
#include <rte_hash_crc.h>

#include "flow_key.h"
#include "../../bessport/utils/common.h"

inline int flow_keycmp(const void *key, const void *key_stored, size_t key_len) {
  const uint64_t *a = ((flow_key_t *)key)->field;
  const uint64_t *b = ((flow_key_t *)key_stored)->field;

  switch (key_len >> 3) {
    default:
      promise_unreachable();
    case 8:
      if (unlikely(a[7] != b[7]))
        return 1;
    case 7:
      if (unlikely(a[6] != b[6]))
        return 1;
    case 6:
      if (unlikely(a[5] != b[5]))
        return 1;
    case 5:
      if (unlikely(a[4] != b[4]))
        return 1;
    case 4:
      if (unlikely(a[3] != b[3]))
        return 1;
    case 3:
      if (unlikely(a[2] != b[2]))
        return 1;
    case 2:
      if (unlikely(a[1] != b[1]))
        return 1;
    case 1:
      if (unlikely(a[0] != b[0]))
        return 1;
  }

  return 0;
}

inline uint32_t flow_hash(const void *key, uint32_t key_len, uint32_t init_val) {
#if __SSE4_2__ && __x86_64
  const uint64_t *a = ((flow_key_t *)key)->field;

  switch (key_len >> 3) {
    default:
      promise_unreachable();
    case 8:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 7:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 6:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 5:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 4:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 3:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 2:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 1:
      init_val = crc32c_sse42_u64(*a++, init_val);
  }

  return init_val;
#else
  return rte_hash_crc(key, key_len, init_val);
#endif
}

inline int actorid_keycmp(const void *key, const void *key_stored, size_t key_len){
  const uint64_t *a = reinterpret_cast<const uint64_t*>(key);
  const uint64_t *b = reinterpret_cast<const uint64_t*>(key_stored);

  switch (key_len >> 3) {
    default:
      promise_unreachable();
    case 8:
      if (unlikely(a[7] != b[7]))
        return 1;
    case 7:
      if (unlikely(a[6] != b[6]))
        return 1;
    case 6:
      if (unlikely(a[5] != b[5]))
        return 1;
    case 5:
      if (unlikely(a[4] != b[4]))
        return 1;
    case 4:
      if (unlikely(a[3] != b[3]))
        return 1;
    case 3:
      if (unlikely(a[2] != b[2]))
        return 1;
    case 2:
      if (unlikely(a[1] != b[1]))
        return 1;
    case 1:
      if (unlikely(a[0] != b[0]))
        return 1;
  }

  return 0;
}

inline uint32_t actorid_hash(const void *key, uint32_t key_len, uint32_t init_val) {
#if __SSE4_2__ && __x86_64
  const uint64_t *a = reinterpret_cast<const uint64_t*>(key);

  switch (key_len >> 3) {
    default:
      promise_unreachable();
    case 8:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 7:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 6:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 5:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 4:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 3:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 2:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 1:
      init_val = crc32c_sse42_u64(*a++, init_val);
  }

  return init_val;
#else
  return rte_hash_crc(key, key_len, init_val);
#endif
}

#endif
