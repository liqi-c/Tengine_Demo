#/bin/bash

Tengine_Dir=/mnt/e/Mygithub/Tengine_Demo/TE-BU-E000-openx86/pre-built/linux_x86
#EMBEDDED_CROSS_ROOT=/opt/hisi-linux/x86-arm/arm-hisiv500-linux/target/bin
#export PATH=${EMBEDDED_CROSS_ROOT}:${PATH}
CC=gcc 
XX=g++

if [ ! -d $Tengine_Dir/include ] || [ ! -d $Tengine_Dir/lib ] ; then 
	echo "Please check tengine dir configure ."
	exit 
fi 
if [ -d build ]; then 
	rm build -rf 
fi 
mkdir build 
cd build 

cmake -DTENGINE_DIR=$Tengine_Dir \
	-DCMAKE_C_COMPILER=$CC \
	-DCMAKE_CXX_COMPILER=$XX \
	..

make  

## Test like below :

cp $Tengine_Dir/lib ./data/ -rf 
