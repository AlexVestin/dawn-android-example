
# https://developer.android.com/ndk/guides/cmake
#ABI=x86_64
ABI=arm64-v8a
#ABI=armeabi-v7a
NDK_VERSION=23.1.7779620
MINSDKVERSION=24
ANDROID_PLATFORM=latest
rm -rf CMakeCache.txt CMakeFiles/

cmake \
    -DTARGET_ARCH=ANDROID \
    -D__ANDROID__=True \
    -DCMAKE_TOOLCHAIN_FILE=/home/alex/Android/Sdk/ndk/$NDK_VERSION/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ABI \
    -DANDROID_NATIVE_API_LEVEL=$MINSDKVERSION \
    -DANDROID_PLATFORM=$ANDROID_PLATFORM \
    .

#make
#rm -f bin/libMyApp.a
#mv libMyApp.a bin/
