# Build and run Tengine example image_classify for Android & Linux

1. Build example image_classify 
Set the correct ANDROID_NDK, TENGINE_DIR and OpenCV_DIR in 'example/image_classify/android_build_armv8.sh' or 'example/image_classify/linux_build.sh'.
-------------------------------------------------------------------------------
cd example/image_classify
mkdir build
cd build
------------------------------------------------------------------------------
build android:
../android_build_armv8.sh
make
-------------------------------------------------------------------------------
build linux
../linux_build.sh
make
------------------------------------------------------------------------------

2. Prepare files for android
Package the execution and library files into 'example/image_classify/android_pack' folder.
-------------------------------------------------------------------------------

Download necessary model and image files into 'example/image_classify/android_pack' folder,
and then adb push the files in 'android_pack' to android platform.
-------------------------------------------------------------------------------
adb push ./android_pack /data/local/tmp/
-------------------------------------------------------------------------------

3. Run example
a.Run 'Classify' on android platform.
-------------------------------------------------------------------------------
adb shell
cd /data/local/tmp
chmod +x Classify
export LD_LIBRARY_PATH=.

-------------------------------------------------------------------------------
./Classify -f tengine -m data/models/squeezenet.tmfile -l data/models/synset_words.txt -i data/images/cat.jpg
-------------------------------------------------------------------------------

b.Run 'Classify' on linux

-------------------------------------------------------------------------------
./Classify -f tengine -m ../../../data/models/squeezenet.tmfile -l ../../../data/models/synset_words.txt -i ../../../data/images/cat.jpg
-------------------------------------------------------------------------------

The usage of Classify is as below:
-------------------------------------------------------------------------------
[Usage]: ./Classify [-h]
    [-f model_format] [-p proto_file] [-m model_file]
    [-l label_file] [-i image_file] [-g img_h,img_w]
    [-s scale] [-w mean[0],mean[1],mean[2]] [-r repeat_count]

    Default image size is : 224, 224
    Default scale value is : 1.f
    Default mean values are : 104.007, 116.669, 122.679
    Default repeat count : 1
-------------------------------------------------------------------------------

