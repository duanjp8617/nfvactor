#!/usr/bin/env python

import env_setup
import info
import time
import cmd

def main():
    print "Shutting down traffic generators."
    env_setup.shutdown_traffic_generator("fg1", info.r1_rt1)
    env_setup.shutdown_traffic_generator("fg2", info.r1_rt1)
    time.sleep(1)

    print "Shutting down runtimes."
    env_setup.shutdown_runtime(info.r1_env)
    env_setup.remove_runtime_logfile(info.r1_env)
    time.sleep(1)

    print "Reset bess daemon."
    env_setup.clean_bess(info.r1_env)
    print "Cleanup done."

if __name__ == '__main__':
    main()
