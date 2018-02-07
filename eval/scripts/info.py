# position of the home dir
HOME_DIR = "~/nfa-ws"

# machine environments, including ip address and the bess configuration files
# that are used to construct the basic bess connections.
r1_env = {"ip":"localhost",      "fname":"bess_r1_script"}
r2_env = {"ip":"202.45.128.155", "fname":"bess_r2_script"}
r3_env = {"ip":"202.45.128.156", "fname":"bess_r3_script"}

# runtime infomation on r1
r1_rt1 = {"runtime_id":11, \
 "input_port_mac":"52:54:01:01:00:01", \
 "output_port_mac":"52:54:01:01:00:02", \
 "control_port_mac":"52:54:01:01:00:03", \
 "rpc_ip":"202.45.128.154", \
 "rpc_port":10241, \
 "input_port":"rt1_iport", \
 "output_port":"rt1_oport", \
 "control_port":"rt1_cport", \
 "worker_core":1, \
 "log_name":"rt1_log.log", \
 "rt_name":"rt1"}

r1_rt2 = {"runtime_id":12, \
 "input_port_mac":"52:54:01:02:00:01", \
 "output_port_mac":"52:54:01:02:00:02", \
 "control_port_mac":"52:54:01:02:00:03", \
 "rpc_ip":"202.45.128.154", \
 "rpc_port":10242, \
 "input_port":"rt2_iport", \
 "output_port":"rt2_oport", \
 "control_port":"rt2_cport", \
 "worker_core":2, \
 "log_name":"rt2_log.log", \
 "rt_name":"rt2"}

r1_rt3 = {"runtime_id":13, \
 "input_port_mac":"52:54:01:03:00:01", \
 "output_port_mac":"52:54:01:03:00:02", \
 "control_port_mac":"52:54:01:03:00:03", \
 "rpc_ip":"202.45.128.154", \
 "rpc_port":10243, \
 "input_port":"rt3_iport", \
 "output_port":"rt3_oport", \
 "control_port":"rt3_cport", \
 "worker_core":3, \
 "log_name":"rt3_log.log", \
 "rt_name":"rt3"}

r1_rt4 = {"runtime_id":14, \
 "input_port_mac":"52:54:01:04:00:01", \
 "output_port_mac":"52:54:01:04:00:02", \
 "control_port_mac":"52:54:01:04:00:03", \
 "rpc_ip":"202.45.128.154", \
 "rpc_port":10244, \
 "input_port":"rt4_iport", \
 "output_port":"rt4_oport", \
 "control_port":"rt4_cport", \
 "worker_core":4, \
 "log_name":"rt4_log.log", \
 "rt_name":"rt4"}

r1_rt5 = {"runtime_id":15, \
 "input_port_mac":"52:54:01:05:00:01", \
 "output_port_mac":"52:54:01:05:00:02", \
 "control_port_mac":"52:54:01:05:00:03", \
 "rpc_ip":"202.45.128.154", \
 "rpc_port":10245, \
 "input_port":"rt5_iport", \
 "output_port":"rt5_oport", \
 "control_port":"rt5_cport", \
 "worker_core":5, \
 "log_name":"rt5_log.log", \
 "rt_name":"rt5"}

r1_rt6 = {"runtime_id":16, \
 "input_port_mac":"52:54:01:06:00:01", \
 "output_port_mac":"52:54:01:06:00:02", \
 "control_port_mac":"52:54:01:06:00:03", \
 "rpc_ip":"202.45.128.154", \
 "rpc_port":10246, \
 "input_port":"rt6_iport", \
 "output_port":"rt6_oport", \
 "control_port":"rt6_cport", \
 "worker_core":6, \
 "log_name":"rt6_log.log", \
 "rt_name":"rt6"}

# runtime information on r2
r2_rt1 = {"runtime_id":21, \
 "input_port_mac":"52:54:02:01:00:01", \
 "output_port_mac":"52:54:02:01:00:02", \
 "control_port_mac":"52:54:02:01:00:03", \
 "rpc_ip":"202.45.128.155", \
 "rpc_port":10241, \
 "input_port":"rt1_iport", \
 "output_port":"rt1_oport", \
 "control_port":"rt1_cport", \
 "worker_core":1, \
 "log_name":"rt1_log.log", \
 "rt_name":"rt1"}

r2_rt2 = {"runtime_id":22, \
 "input_port_mac":"52:54:02:02:00:01", \
 "output_port_mac":"52:54:02:02:00:02", \
 "control_port_mac":"52:54:02:02:00:03", \
 "rpc_ip":"202.45.128.155", \
 "rpc_port":10242, \
 "input_port":"rt2_iport", \
 "output_port":"rt2_oport", \
 "control_port":"rt2_cport", \
 "worker_core":2, \
 "log_name":"rt2_log.log", \
 "rt_name":"rt2"}

r2_rt3 = {"runtime_id":23, \
 "input_port_mac":"52:54:02:03:00:01", \
 "output_port_mac":"52:54:02:03:00:02", \
 "control_port_mac":"52:54:02:03:00:03", \
 "rpc_ip":"202.45.128.155", \
 "rpc_port":10243, \
 "input_port":"rt3_iport", \
 "output_port":"rt3_oport", \
 "control_port":"rt3_cport", \
 "worker_core":3, \
 "log_name":"rt3_log.log", \
 "rt_name":"rt3"}

r2_rt4 = {"runtime_id":24, \
 "input_port_mac":"52:54:02:04:00:01", \
 "output_port_mac":"52:54:02:04:00:02", \
 "control_port_mac":"52:54:02:04:00:03", \
 "rpc_ip":"202.45.128.155", \
 "rpc_port":10244, \
 "input_port":"rt4_iport", \
 "output_port":"rt4_oport", \
 "control_port":"rt4_cport", \
 "worker_core":4, \
 "log_name":"rt4_log.log", \
 "rt_name":"rt4"}

r2_rt5 = {"runtime_id":25, \
 "input_port_mac":"52:54:02:05:00:01", \
 "output_port_mac":"52:54:02:05:00:02", \
 "control_port_mac":"52:54:02:05:00:03", \
 "rpc_ip":"202.45.128.155", \
 "rpc_port":10245, \
 "input_port":"rt5_iport", \
 "output_port":"rt5_oport", \
 "control_port":"rt5_cport", \
 "worker_core":5, \
 "log_name":"rt5_log.log", \
 "rt_name":"rt5"}

r2_rt6 = {"runtime_id":26, \
 "input_port_mac":"52:54:02:06:00:01", \
 "output_port_mac":"52:54:02:06:00:02", \
 "control_port_mac":"52:54:02:06:00:03", \
 "rpc_ip":"202.45.128.155", \
 "rpc_port":10246, \
 "input_port":"rt6_iport", \
 "output_port":"rt6_oport", \
 "control_port":"rt6_cport", \
 "worker_core":6, \
 "log_name":"rt6_log.log", \
 "rt_name":"rt6"}

# runtime information on r3
r3_rt1 = {"runtime_id":31, \
 "input_port_mac":"52:54:03:01:00:01", \
 "output_port_mac":"52:54:03:01:00:02", \
 "control_port_mac":"52:54:03:01:00:03", \
 "rpc_ip":"202.45.128.156", \
 "rpc_port":10241, \
 "input_port":"rt1_iport", \
 "output_port":"rt1_oport", \
 "control_port":"rt1_cport", \
 "worker_core":1, \
 "log_name":"rt1_log.log", \
 "rt_name":"rt1"}

r3_rt2 = {"runtime_id":32, \
 "input_port_mac":"52:54:03:02:00:01", \
 "output_port_mac":"52:54:03:02:00:02", \
 "control_port_mac":"52:54:03:02:00:03", \
 "rpc_ip":"202.45.128.156", \
 "rpc_port":10242, \
 "input_port":"rt2_iport", \
 "output_port":"rt2_oport", \
 "control_port":"rt2_cport", \
 "worker_core":2, \
 "log_name":"rt2_log.log", \
 "rt_name":"rt2"}

r3_rt3 = {"runtime_id":33, \
 "input_port_mac":"52:54:03:03:00:01", \
 "output_port_mac":"52:54:03:03:00:02", \
 "control_port_mac":"52:54:03:03:00:03", \
 "rpc_ip":"202.45.128.156", \
 "rpc_port":10243, \
 "input_port":"rt3_iport", \
 "output_port":"rt3_oport", \
 "control_port":"rt3_cport", \
 "worker_core":3, \
 "log_name":"rt3_log.log", \
 "rt_name":"rt3"}

r3_rt4 = {"runtime_id":34, \
 "input_port_mac":"52:54:03:04:00:01", \
 "output_port_mac":"52:54:03:04:00:02", \
 "control_port_mac":"52:54:03:04:00:03", \
 "rpc_ip":"202.45.128.156", \
 "rpc_port":10244, \
 "input_port":"rt4_iport", \
 "output_port":"rt4_oport", \
 "control_port":"rt4_cport", \
 "worker_core":4, \
 "log_name":"rt4_log.log", \
 "rt_name":"rt4"}

r3_rt5 = {"runtime_id":35, \
 "input_port_mac":"52:54:03:05:00:01", \
 "output_port_mac":"52:54:03:05:00:02", \
 "control_port_mac":"52:54:03:05:00:03", \
 "rpc_ip":"202.45.128.156", \
 "rpc_port":10245, \
 "input_port":"rt5_iport", \
 "output_port":"rt5_oport", \
 "control_port":"rt5_cport", \
 "worker_core":5, \
 "log_name":"rt5_log.log", \
 "rt_name":"rt5"}

r3_rt6 = {"runtime_id":36, \
 "input_port_mac":"52:54:03:06:00:01", \
 "output_port_mac":"52:54:03:06:00:02", \
 "control_port_mac":"52:54:03:06:00:03", \
 "rpc_ip":"202.45.128.156", \
 "rpc_port":10246, \
 "input_port":"rt6_iport", \
 "output_port":"rt6_oport", \
 "control_port":"rt6_cport", \
 "worker_core":6, \
 "log_name":"rt6_log.log", \
 "rt_name":"rt6"}
