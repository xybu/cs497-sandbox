#!/bin/bash

# this script downloads and compiles libtins from source
# assume the platform is linux.

check_libtins() {
	libtins_path=$(whereis libtins | cut -d ' ' -f2 | xargs)

	if [ -n "$libtins_path" ] ; then
		echo -e "\033[92mCheck libtins: \"$libtins_path\" ... OK\033[0m"
		exit 0
	else
		echo -e "\033[91mCheck libtins: not found\033[0m"
	fi
}

check_libtins

wget https://github.com/mfontanini/libtins/archive/master.tar.gz
tar xvf master.tar.gz
rm -fv master.tar.gz

if [ -x $(which apt-get) ] ; then
	sudo apt-get install libpcap-dev libssl-dev cmake
elif [ -x $(which yum) ] ; then
	sudo yum install libpcap-devel openssl-devel cmake
fi

cd libtins-master
mkdir build && cd build
cmake ../ -DLIBTINS_ENABLE_CXX11=1
make
sudo make install
sudo ldconfig

cd ../..
rm -rfv libtins-master

echo -e "\033[94mSuccessfully installed libtins.\033[0m"

check_libtins
