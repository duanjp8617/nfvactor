#!/usr/bin/env python

import cmd
import info
import env_setup

def main():
    env_setup.remove_r1_tmp_file()
    env_setup.create_r1_tmp_file()
    env_setup.add_pkt("simple_tcp.template", 64)
    env_setup.add_variable_flows("fg1", info.r1_rt1, pps=440000, flow_rate = 4400, flow_duration = 10.0)
    env_setup.add_variable_flows("fg2", info.r1_rt1, pps=440000, flow_rate = 44000, flow_duration = 1.0)
    #env_setup.add_fixed_flows("fg1", info.r1_rt1, flow_arrival_interval=500, flow_pps=10, flow_duration=3600, concurrent_flows=100000)

    env_setup.setup_bess({"ip":"localhost", "fname":"bess_r1_script_tmp"})
    env_setup.setup_bess(info.r2_env)
    env_setup.setup_bess(info.r3_env)


    #env_setup.start_traffic_generator("fg1", info.r1_rt1)

if __name__ == '__main__':
    main()
