#/bin/bash

Tengine_Dir=/home/openailab/TE-BU-E000-rk3399-V1.11-R20191211DJ_mssd_tflite/pre-built/linux_arm64
#EMBEDDED_CROSS_ROOT=/opt/hisi-linux/x86-arm/arm-hisiv500-linux/target/bin
#export PATH=${EMBEDDED_CROSS_ROOT}:${PATH}


if [ ! -d $Tengine_Dir/include ] || [ ! -d $Tengine_Dir/lib ] ; then 
	echo "Please check tengine dir configure ."
	exit 
fi 
mkdir build 
cd build 

cmake -DTENGINE_DIR=$Tengine_Dir \
	-DCMAKE_C_COMPILER=gcc \
	-DCMAKE_CXX_COMPILER=g++ \
	..

make  

./Classify -m ../data/models/mobilenet_quant_v1_224_1206.tmfile -l ../data/models/imagenet_slim_labels.txt -i ../data/images/cat.jpg