#!/bin/bash

Tengine_Dir=/mnt/e/Mygithub/Tengine_Demo/TE-BU-E000-openx86/pre-built/linux_x86
#EMBEDDED_CROSS_ROOT=/opt/hisi-linux/x86-arm/arm-hisiv500-linux/target/bin
#export PATH=${EMBEDDED_CROSS_ROOT}:${PATH}
CC=gcc 
XX=g++

if [ "$1" = "" ]; then 
	
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
fi
if [ "$1" = "run" ]; then 
	echo "Test Classify "
	./build/Classify/Classify -f tengine -m ./Classify/data/models/squeezenet.tmfile -l ./Classify/data/models/synset_words.txt -i ./Classify/data/images/cat.jpg -g 227,227 -r 1
	
	echo "Test MobileFace "
	./build/mobileface/MobileFace -m ./data/mobileface/mobileface.tmfile -i ./data/mobileface/mobileface01.jpg
	
	echo "Test MTCNN "
	./build/mtcnn/MTCNN -i ./mtcnn/data/images/mtcnn_face6.jpg -d ./mtcnn/data/models -r 1 

	echo "Test mobilenet_tflite "
	./build/tflite-int8-mobilenet/Mobilenet_TFLITE -m ./tflite-int8-mobilenet/data/models/mobilenet_quant_v1_224_1206.tmfile -l ./tflite-int8-mobilenet/data/models/imagenet_slim_labels.txt -i ./tflite-int8-mobilenet/data/images/cat.jpg
fi 