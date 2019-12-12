#/bin/bash

cmake -DTENGINE_DIR=/home/rk/TE-3399-1.7.1/pre-built/linux_arm64 \
	-DCMAKE_C_COMPILER=gcc \
	-DCMAKE_CXX_COMPILER=g++ \
	..
