#ifndef NFAFLAGS_H
#define NFAFLAGS_H

#include <gflags/gflags.h>

DECLARE_bool(boolean_flag);
DECLARE_string(string_flag);
DECLARE_int32(temp_core);

DECLARE_string(input_port);
DECLARE_string(output_port);
DECLARE_string(control_port);

DECLARE_int32(rpc_timeout);

DECLARE_int32(runtime_id);
DECLARE_string(input_port_mac);
DECLARE_string(output_port_mac);
DECLARE_string(control_port_mac);
DECLARE_string(rpc_ip);
DECLARE_int32(rpc_port);

DECLARE_int32(worker_core);

DECLARE_string(default_input_mac);
DECLARE_string(default_output_mac);

DECLARE_string(service_chain);

#endif
