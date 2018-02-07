#!/usr/bin/env python

import cmd
import info
import env_setup

def main():
    env_setup.shutdown_runtime(info.r1_env)
    env_setup.shutdown_runtime(info.r2_env)
    env_setup.shutdown_runtime(info.r3_env)

    env_setup.remove_runtime_logfile(info.r1_env)
    env_setup.remove_runtime_logfile(info.r2_env)
    env_setup.remove_runtime_logfile(info.r3_env)

if __name__ == '__main__':
    main()
