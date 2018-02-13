#!/usr/bin/env python

import env_setup
import info
import time
import stats
import rpc
import cmd

# Configurable parameters.

pkt_size = 64
traffic_gen_pps = 440000
long_flow_rate = 4400
long_flow_duration = 10.0
small_flow_rate = 44000
small_flow_duration = 1.0
service_chain = "firewall,nat"

def main():
    print "Restarting bess daemon."
    #cmd_name = "sudo "+info.HOME_DIR+"/eval/script/reboot_bess.sh"
    #cmd.cmd(cmd_name)

    env_setup.remove_r1_tmp_file()
    env_setup.create_r1_tmp_file()

    print "Use packet with "+str(pkt_size)+"bytes size."
    env_setup.add_pkt("simple_tcp.template", pkt_size)

    print "Add traffic generator with "+str(traffic_gen_pps)+"pps total rate, "+str(long_flow_rate)+"pps flow rate, "+str(long_flow_duration)+"s flow duration"
    env_setup.add_variable_flows("fg1", info.r1_rt1, pps=traffic_gen_pps, flow_rate = long_flow_rate, flow_duration = long_flow_duration)

    print "Add traffic generator with "+str(traffic_gen_pps)+"pps total rate, "+str(small_flow_rate)+"pps flow rate, "+str(small_flow_duration)+"s flow duration"
    env_setup.add_variable_flows("fg2", info.r1_rt1, pps=traffic_gen_pps, flow_rate = small_flow_rate, flow_duration = small_flow_duration)

    env_setup.setup_bess({"ip":"localhost", "fname":"bess_r1_script_tmp"})

    print "Add virtual switch (runtime 1)."
    env_setup.launch_runtime(info.r1_env, info.r1_rt1, "null")

    print "Add runtime 2, with service chain "+service_chain+"."
    env_setup.launch_runtime(info.r1_env, info.r1_rt2, service_chain)

    print "Add runtime 3, with service chain "+service_chain+"."
    env_setup.launch_runtime(info.r1_env, info.r1_rt3, service_chain)

    time.sleep(1)
    print "Execute RPC calls to initialize cluster."
    rpc.AddOutputRts(info.r1_rt1, info.r1_rt2)
    rpc.AddOutputMac(info.r1_rt1, info.r1_rt2)

    rpc.AddOutputRts(info.r1_rt1, info.r1_rt3)

    rpc.AddReplicas(info.r1_rt2, info.r1_rt3)

    time.sleep(5)
    print "Start replication demo."
    env_setup.start_traffic_generator("fg1", info.r1_rt1)
    env_setup.start_traffic_generator("fg2", info.r1_rt1)

    print "Let repliation run for 5s."
    time.sleep(5)

    print "Wait for 5s to gather throughput stats."
    r1_port_stats = stats.get_server_port_stats(info.r1_env)
    r1_rt3_oport_stat_before = stats.get_runtime_oport_stat(r1_port_stats, info.r1_rt3)
    time.sleep(5)
    r1_port_stats = stats.get_server_port_stats(info.r1_env)
    r1_rt3_oport_stat_after = stats.get_runtime_oport_stat(r1_port_stats, info.r1_rt3)

    print "Output throughput of replica runtime 3 is:"
    print stats.get_runtime_throughput_perf(r1_rt3_oport_stat_before, r1_rt3_oport_stat_after)

if __name__ == '__main__':
    main()
