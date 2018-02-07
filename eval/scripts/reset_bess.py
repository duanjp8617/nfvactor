#!/usr/bin/env python

import cmd
import info
import env_setup

def main():
    env_setup.clean_bess(info.r1_env)
    env_setup.clean_bess(info.r2_env)
    env_setup.clean_bess(info.r3_env)

if __name__ == '__main__':
    main()
