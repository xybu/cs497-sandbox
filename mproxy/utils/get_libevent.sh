#!/bin/bash

# this script downloads and compiles libevent from source
# assume the platform is linux.

check_libevent() {
	libevent_path=$(whereis libevent | cut -d ':' -f2 | xargs)

	if [ -n "$libevent_path" ] ; then
		echo -e "\033[92mCheck libevent: \"$libevent_path\" ... OK\033[0m"
		exit 0
	else
		echo -e "\033[91mCheck libevent: not found\033[0m"
	fi
}

check_libevent

if [ -x $(which apt-get) ] ; then
	sudo apt-get install libevent-dev
elif [ -x $(which yum) ] ; then
	sudo yum install libevent-dev
fi

echo -e "\033[94mSuccessfully installed libtins.\033[0m"

check_libevent
