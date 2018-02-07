#ifndef FLOW_KEY_H
#define FLOW_KEY_H

#include <cstdint>

static constexpr int flow_key_field_size = 2;

static constexpr int flow_key_size = 16;

struct flow_key_t{
  uint64_t field[flow_key_field_size];
};

#endif

