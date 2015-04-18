#!/bin/bash

# this script downloads and compiles libevent from source
# assume the platform is linux.

if [ -x $(which apt-get) ] ; then
	sudo apt-get install libevent-dev
elif [ -x $(which yum) ] ; then
	sudo yum install libevent-dev
fi

echo -e "\033[94mSuccessfully installed libtins.\033[0m"
