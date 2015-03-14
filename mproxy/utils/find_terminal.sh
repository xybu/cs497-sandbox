#!/bin/bash

if [ -x "$(which xfce4-terminal)" ] ; then
	echo -e "\033[92mFind \"xfce4-terminal\".\033[0m"
	TERMINAL_CMD='xfce4-terminal -e'
elif [ -x "$(which gnome-terminal)" ] ; then
	echo -e "\033[92mFind \"gnome-terminal\".\033[0m"
	TERMINAL_CMD='gnome-terminal -e'
elif [ -x "$(which konsole)" ] ; then
	echo -e "\033[92mFind \"konsole\".\033[0m"
	TERMINAL_CMD='konsole -e'
elif [ -x "$(which mate-terminal)" ] ; then
	echo -e "\033[92mFind \"mate-terminal\".\033[0m"
	TERMINAL_CMD='mate-terminal -e'
elif [ -x "$(which xterm)" ] ; then
	echo -e "\033[92mFind \"xterm\".\033[0m"
	TERMINAL_CMD='xterm -e'
else
	echo -e "\033[91mCould not determine your terminal command.\033[0m"
	exit 1
fi
