#!/bin/bash

export ANDROID_NDK=/root/sf/android-ndk-r16

cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
     -DANDROID_ABI="arm64-v8a" \
     -DANDROID_PLATFORM=android-21 \
     -DANDROID_STL=c++_shared \
     -DTENGINE_DIR=/root/work/auto_build/firefly3399/.tengine/deliver_Firefly3399-Explore_R20190813/pre-built/android_arm64 \
     -DANDROID_ALLOW_UNDEFINED_SYMBOLS=TRUE \
     ..

