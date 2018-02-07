#ifndef NFA_IPV4_FIELD_H
#define NFA_IPV4_FIELD_H

struct nfa_ipv4_field{
  uint16_t pos;
  uint16_t offset;
  uint32_t size;
  uint64_t mask;

  static void nfa_init_ipv4_field(nfa_ipv4_field* f){
    // protocol
    f[0].pos = 0;
    f[0].offset = 23;
    f[0].size = 1;
    f[0].mask = ((uint64_t)1 << (f[0].size * 8)) - 1;

    // src/dst ip
    f[1].pos = 1;
    f[1].offset = 26;
    f[1].size = 8;
    f[1].mask = 0xffffffffffffffff;

    // src/dst port
    f[2].pos = 9;
    f[2].offset = 34;
    f[2].size = 4;
    f[2].mask = ((uint64_t)1 << (f[2].size * 8)) - 1;
  }
};

#endif
