#include "pkt_counter.h"

#include "../base/network_function_register.h"

bool registered_pkt_counter =
    static_nf_register::get_register()
    .register_nf<pkt_counter, pkt_counter_shared_state, pkt_counter_fs>("pkt_counter", 1);
