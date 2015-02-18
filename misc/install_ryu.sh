#!/bin/bash

# install_ryu.sh
# This script installs Ryu on local machine.
# 
# Xiangyu Bu <xybu92@live.com>

# Ryu requires pip package manager and CPython binding
sudo apt-get install python-pip python-dev

# Because Ryu does not list its dependecies well, 
# we install them manually.
# ECDSA cryptographic signature library
sudo pip install ecdsa
# Dynamic plugins for Python applications
sudo pip install stevedore
# Lightweight stackless thread module in Python
sudo pip install greenlet
# Just in case the following are not installed yet.
sudo pip install eventlet
sudo pip install routes
sudo pip install webob
sudo pip install paramiko

# Now we can install Ryu.
sudo pip install ryu
