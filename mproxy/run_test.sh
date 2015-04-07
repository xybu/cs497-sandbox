#!/bin/bash

source $(dirname "$0")/utils/find_terminal.sh

$TERMINAL_CMD "ryu-manager --verbose --observe-links /usr/local/lib/python2.7/dist-packages/ryu/app/simple_switch_13.py \
--ofp-tcp-listen-port 6635" &

sleep 2

$TERMINAL_CMD "./main"

sleep 2

$TERMINAL_CMD "sudo mn --switch ovsk,protocols=OpenFlow13 --controller remote --topo tree,depth=2,fanout=4 --test all" &
