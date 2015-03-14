#!/bin/bash

ryu-manager --verbose --observe-links /usr/local/lib/python2.7/dist-packages/ryu/app/simple_switch.py \
--ofp-tcp-listen-port $1
