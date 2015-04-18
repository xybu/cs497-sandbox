#!/bin/bash

# Prerequite for unit test component
# pip install nose

# source code
git clone https://github.com/floodlight/loxigen.git
cd loxigen
make c
rm -rf ../../inc/loci
mkdir -p ../../inc/loci
mv loxi_output/loci ../../inc
cd ..
rm -rf loxigen
