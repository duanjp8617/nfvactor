#include "firewall.h"

#include "../base/network_function_register.h"

bool registered_firewall =
    static_nf_register::get_register()
    .register_nf<firewall, firewall_shared_state, firewall_fs>("firewall", 2);
