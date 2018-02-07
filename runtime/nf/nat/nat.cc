#include "nat.h"

#include "../base/network_function_register.h"

bool registered_nat =
    static_nf_register::get_register()
    .register_nf<nat, nat_shared_state, nat_fs>("nat", 4);
