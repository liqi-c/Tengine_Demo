
CURRENT_DIR=$PWD 
Tengine_Dir=${CURRENT_DIR}/Tengine_lib/TE-BU-E000-x86/pre-built-64
CC=gcc 
XX=g++

mkdir build_$1 
cd build_$1 

cmake -DTENGINE_DIR=$Tengine_Dir \
	-DCMAKE_C_COMPILER=$CC \
	-DCMAKE_CXX_COMPILER=$XX \
	..

make 