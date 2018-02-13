# position of the home dir
HOME_DIR = "~/nfvactor"

# machine environments, including ip address and the bess configuration files
# that are used to construct the basic bess connections.
r1_env = {"ip":"localhost",      "fname":"bess_r1_script"}

# runtime infomation on r1
r1_rt1 = {"runtime_id":1, \
 "input_port_mac":"52:54:01:00:00:01", \
 "output_port_mac":"52:54:01:00:00:02", \
 "control_port_mac":"52:54:01:00:00:03", \
 "rpc_ip":"127.0.0.1", \
 "rpc_port":10241, \
 "input_port":"rt1_iport", \
 "output_port":"rt1_oport", \
 "control_port":"rt1_cport", \
 "worker_core":2, \
 "log_name":"rt1_log.log", \
 "rt_name":"rt1"}

r1_rt2 = {"runtime_id":2, \
 "input_port_mac":"52:54:02:00:00:01", \
 "output_port_mac":"52:54:02:00:00:02", \
 "control_port_mac":"52:54:02:00:00:03", \
 "rpc_ip":"127.0.0.1", \
 "rpc_port":10242, \
 "input_port":"rt2_iport", \
 "output_port":"rt2_oport", \
 "control_port":"rt2_cport", \
 "worker_core":3, \
 "log_name":"rt2_log.log", \
 "rt_name":"rt2"}

r1_rt3 = {"runtime_id":3, \
 "input_port_mac":"52:54:03:00:00:01", \
 "output_port_mac":"52:54:03:00:00:02", \
 "control_port_mac":"52:54:03:00:00:03", \
 "rpc_ip":"127.0.0.1", \
 "rpc_port":10243, \
 "input_port":"rt3_iport", \
 "output_port":"rt3_oport", \
 "control_port":"rt3_cport", \
 "worker_core":4, \
 "log_name":"rt3_log.log", \
 "rt_name":"rt3"}
