#!/bin/bash

# This script installs the latest version of Mininet from source along with all 
# the packages it contains. Besides, it enables OpenFlow 1.3.
# The reason why we don't use the Ubuntu package `mininet` is that it is 
# several versions behind the latest.

cd /tmp

git clone git://github.com/mininet/mininet
cd mininet
./util/install.sh -ab3
