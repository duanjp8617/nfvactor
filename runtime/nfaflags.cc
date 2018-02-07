#include <iostream>
#include <string>

#include "nfaflags.h"

using namespace std;

DEFINE_bool(boolean_flag, true, "a test boolean flag");

DEFINE_string(string_flag, "a,b,c", "comma spearated string flag");

DEFINE_int32(temp_core, 5, "Temporary lcore binding.");

DEFINE_string(input_port, "", "Name of the input port");

DEFINE_string(output_port, "", "Name of the output port");

DEFINE_string(control_port, "", "Name of the control port");

DEFINE_int32(rpc_timeout, 10, "RPC timeout in milliseconds, default value is 10ms");

DEFINE_int32(runtime_id, -1, "The ID of the runtime.");

DEFINE_string(input_port_mac, "nil", "The mac address of the input port.");

DEFINE_string(output_port_mac, "nil", "The mac address of the output port.");

DEFINE_string(control_port_mac, "nil", "The mac address of the control port");

DEFINE_string(rpc_ip, "nil", "The IP address that the RPC server listens on.");

DEFINE_int32(rpc_port, -1, "The port that the RPC server listens on");

DEFINE_int32(worker_core, -1, "The core that the worker thread binds on.");

DEFINE_string(default_input_mac,  "01:02:03:04:05:06", "Default mac address when sending packets out from input port.");

DEFINE_string(default_output_mac, "02:03:04:05:06:07", "Default mac address when sending packets out from output port.");

DEFINE_string(service_chain, "null", "the type of service chain eg: firewall,http_parser,flow_monitor.");


