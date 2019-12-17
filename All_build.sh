#!/bin/bash

CURRENT_DIR=$PWD 
	echo "CURRENT_DIR=$CURRENT_DIR ----------"

if [ "$1" = "help" ] || [ "$1" = "-help" ] || [ "$1" = "-h" ] ; then 
	echo "Build : "
	echo "	$0 x86/arm32/arm64/hisix200/hisiv300/hisiv500"
	echo "Run : "
	echo "	$0 run"
fi

function BUILD () {
	echo "-----------build start ----------"
	echo "-----------Tengine_Dir=$Tengine_Dir----------"
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
	
	make ;
	echo "-----------build end ----------"
}

if [ "$1" = "x86" ]; then 
	echo "-----------build x86 ----------"
	Tengine_Dir=${CURRENT_DIR}/Tengine_lib/TE-BU-E000-x86/pre-built/linux_x86
	CC=gcc 
	XX=g++
	BUILD
elif [ "$1" = "arm32" ]; then 
	Tengine_Dir=$CURRENT_DIR/Tengine_lib/TE-BU-E000-arm32/pre-built/linux_arm32
	CC=gcc 
	XX=g++
	BUILD
elif [ "$1" = "arm64" ]; then
	Tengine_Dir=$CURRENT_DIR/Tengine_lib/TE-BU-E000-arm64/pre-built/linux_arm64
	CC=gcc
	XX=g++
	BUILD	
elif [ "$1" = "arm32_cross" ]; then 
	Tengine_Dir=$CURRENT_DIR/Tengine_lib/TE-BU-E000-openx86/pre-built/linux_x86
	CC=arm-linux-gnueabihf-gcc 
	XX=arm-linux-gnueabihf-g++
elif [ "$1" = "arm64_cross" ]; then
	Tengine_Dir=$CURRENT_DIR/Tengine_lib/TE-BU-E000-openx86/pre-built/linux_x86
	CC=aarch64-linux-gnu-gcc
	XX=aarch64-linux-gnu-g++	
elif [ "$1" = "hisix200" ]; then 
	Tengine_Dir=$CURRENT_DIR/Tengine_lib/TE-BU-E000-hisix200/pre-built/linux_x86
	EMBEDDED_CROSS_ROOT=/opt/hisi-linux/x86-arm/arm-hisix200-linux/target/bin
	export PATH=${EMBEDDED_CROSS_ROOT}:${PATH}
	CC=arm-himix200-linux-gcc 
	XX=arm-himix200-linux-g++
	BUILD	
elif [ "$1" = "hisiv300" ]; then 
	Tengine_Dir=$CURRENT_DIR/Tengine_lib/TE-BU-E000-hisiv300/pre-built/linux_x86
	EMBEDDED_CROSS_ROOT=/opt/hisi-linux/x86-arm/arm-hisiv300-linux/target/bin
	export PATH=${EMBEDDED_CROSS_ROOT}:${PATH}
	CC=arm-hisiv300-linux-gcc
	XX=arm-hisiv300-linux-g++
	BUILD
elif [ "$1" = "hisiv500" ]; then 
	Tengine_Dir=$CURRENT_DIR/Tengine_lib/TE-BU-E000-hisiv500/pre-built/linux_arm32
	EMBEDDED_CROSS_ROOT=/opt/hisi-linux/x86-arm/arm-hisiv500-linux/target/bin
	export PATH=${EMBEDDED_CROSS_ROOT}:${PATH}
	CC=arm-hisiv500-linux-gcc
	XX=arm-hisiv500-linux-g++
fi

if [ "$1" = "run" ]; then 
	echo "***********************************************************************"

	echo " Test Classify "
	./build/Classify/Classify -f tengine -m ./Classify/data/models/squeezenet.tmfile -l ./Classify/data/models/synset_words.txt -i ./Classify/data/images/cat.jpg -g 227,227 -r 1
	echo "***********************************************************************"
	
	echo " Test MobileFace "
	./build/mobileface/MobileFace -m ./mobileface/data/mobileface.tmfile -i ./mobileface/data/mobileface01.jpg
	
	echo "***********************************************************************"
	echo " Test MTCNN "
	./build/mtcnn/MTCNN -i ./mtcnn/data/images/mtcnn_face6.jpg -d ./mtcnn/data/models -r 1 

	echo "***********************************************************************"
	#echo "\n\nTest mobilenet_tflite "
	#./build/tflite-int8-mobilenet/Mobilenet_TFLITE -m ./tflite-int8-mobilenet/data/models/mobilenet_quant_v1_224_1206.tmfile -l ./tflite-int8-mobilenet/data/models/imagenet_slim_labels.txt -i ./tflite-int8-mobilenet/data/images/cat.jpg
fi 

if [ "$1" = "clean" ]; then 
	rm build -rf 
	rm save.jpg caffe_output_data.txt
fi