#!/bin/bash

# Xiangyu Bu <xybu92@live.com>

# Clean the mess if any
sudo killall controller ryu-manager
sudo mn -c

# Start Ryu
ryu-manager --verbose /usr/local/lib/python2.7/dist-packages/ryu/app/simple_switch.py < /dev/null &> ./simple_switch.log &

# Create the topo
sudo mn --controller remote --switch ovsk --mac --custom ./demo_mininet_topo.py --topo demo --test pingall

# Terminate Ryu controller
sleep 1 && sudo killall ryu-manager
