#! /bin/sh

sudo kill -9 $(ps -ef | grep server_main | grep -v grep | awk '{print $2}')

sudo rm ~/nfa-ws/eval/scripts/*.log
