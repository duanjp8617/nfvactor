#!/usr/bin/env python

import env_setup
import info
import time
import stats
import rpc
import cmd

# Parameters that can be tuned.
pkt_size = 64
flow_rate = 10
flow_duration = 3600
concurrent_flows = 100000
service_chain = "firewall,nat"

def main():
    print "Restarting bess daemon."

    env_setup.remove_r1_tmp_file()
    env_setup.create_r1_tmp_file()

    print "Use packet with "+str(pkt_size)+"bytes size."
    env_setup.add_pkt("simple_tcp.template", pkt_size)

    print "Add traffic generator with "+str(flow_rate)+"pps flow rate, "+str(flow_duration)+"s flow duration, "+str(concurrent_flows)+" concurrent number of flows."
    env_setup.add_fixed_flows("fg1", info.r1_rt1, flow_arrival_interval=500, flow_pps=flow_rate, flow_duration=flow_duration, concurrent_flows=concurrent_flows)

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

    time.sleep(1)
    print "Start migration demo."
    env_setup.start_traffic_generator("fg1", info.r1_rt1)

    time.sleep(5)
    print "Start migration."
    rpc.AddOutputMac(info.r1_rt1, info.r1_rt3)
    rpc.SetMigrationTarget(info.r1_rt2, info.r1_rt3)
    rpc.MigrateTo(info.r1_rt2, info.r1_rt3, 120000)

    time.sleep(5)
    print "Read migration stats from runtime 2:"
    print stats.get_runtime_migration_stat(stats.read_runtime_log(info.r1_env, info.r1_rt2))

if __name__ == '__main__':
    main()
