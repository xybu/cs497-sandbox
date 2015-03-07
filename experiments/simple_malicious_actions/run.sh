#!/bin/bash

BENIGN_RYU_PORT=6633
MALICIOUS_RYU_PORT=6634

# create multiple terminal windows to show the commands
if [ -x "$(which xfce4-terminal)" ] ; then
	echo -e "\033[92mFind \"xfce4-terminal\".\033[0m"
	TERMINAL_CMD='xfce4-terminal -e'
elif [ -x "$(which gnome-terminal)" ] ; then
	echo -e "\033[92mFind \"gnome-terminal\".\033[0m"
	TERMINAL_CMD='gnome-terminal -e'
elif [ -x "$(which konsole)" ] ; then
	echo -e "\033[92mFind \"konsole\".\033[0m"
	TERMINAL_CMD='konsole -e'
elif [ -x "$(which xterm)" ] ; then
	echo -e "\033[92mFind \"xterm\".\033[0m"
	TERMINAL_CMD='xterm -e'
else
	echo -e "\033[91mCould not determine your terminal command.\033[0m"
	exit 1
fi

echo -e "\033[94mSome housekeeping...\033[0m"
# clean before start
sudo killall controller ryu-manager
sudo mn -c

# start a benign controller
echo -e "\033[94mStarting benign Ryu controller in a new shell...\033[0m"
$TERMINAL_CMD "ryu-manager --observe-links /usr/local/lib/python2.7/dist-packages/ryu/app/simple_switch_13.py \
--ofp-tcp-listen-port $BENIGN_RYU_PORT" & # < /dev/null &> ./simple_switch.log &

# start malicious Ryu controller
echo -e "\033[94mStarting malicious Ryu controller in a new shell...\033[0m"
$TERMINAL_CMD "ryu-manager --observe-links ./malicious_ryu_13.py \
--ofp-tcp-listen-port $MALICIOUS_RYU_PORT" & #  < /dev/null &> ./simple_switch.log &

sleep 3

# start customized mininet
echo -e "\033[94mStarting mininet...\033[0m"
sudo python ./mn.py

echo -e "\033[94mHousekeeping again...\033[0m"
# kill all children proc
pkill -P $$
