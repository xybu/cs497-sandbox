#!/bin/bash

# Prerequite for unit test component
# pip install nose

# source code
git clone https://github.com/floodlight/loxigen.git
cd loxigen
make c
mkdir -p ../../inc/loci
mv loxi_output/loci ../../inc/loci
