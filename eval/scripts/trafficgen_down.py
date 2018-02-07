#!/usr/bin/env python

import env_setup
import info

def main():
    env_setup.shutdown_traffic_generator("fg1", info.r1_rt1)
    #env_setup.shutdown_traffic_generator("fg2", info.r1_rt1)

if __name__ == '__main__':
    main()
