#!/usr/bin/env python

import cmd
import info
import env_setup

def main():
    env_setup.launch_runtime(info.r1_env, info.r1_rt1, "null")
    #env_setup.launch_runtime(info.r1_env, info.r1_rt2)
    #env_setup.launch_runtime(info.r1_env, info.r1_rt3)
    #env_setup.launch_runtime(info.r1_env, info.r1_rt4)
    #env_setup.launch_runtime(info.r1_env, info.r1_rt5)
    #env_setup.launch_runtime(info.r1_env, info.r1_rt6)

    env_setup.launch_runtime(info.r2_env, info.r2_rt1, "firewall,nat,ips")
    #env_setup.launch_runtime(info.r2_env, info.r2_rt2)
    #env_setup.launch_runtime(info.r2_env, info.r2_rt3)
    #env_setup.launch_runtime(info.r2_env, info.r2_rt4)
    #env_setup.launch_runtime(info.r2_env, info.r2_rt5)
    #env_setup.launch_runtime(info.r2_env, info.r2_rt6)

    env_setup.launch_runtime(info.r3_env, info.r3_rt1, "firewall,nat,ips")
    #env_setup.launch_runtime(info.r3_env, info.r3_rt2)
    #env_setup.launch_runtime(info.r3_env, info.r3_rt3)
    #env_setup.launch_runtime(info.r3_env, info.r3_rt4)
    #env_setup.launch_runtime(info.r3_env, info.r3_rt5)
    #env_setup.launch_runtime(info.r3_env, info.r3_rt6)

if __name__ == '__main__':
    main()
