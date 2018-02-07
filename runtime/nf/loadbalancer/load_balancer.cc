#include "load_balancer.h"

#include "../base/network_function_register.h"

bool registered_load_balancer =
    static_nf_register::get_register()
    .register_nf<load_balancer, load_balancer_shared_state, load_balancer_fs>("load_balancer", 3);
