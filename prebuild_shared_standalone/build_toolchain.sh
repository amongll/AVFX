#!/usr/bin/env bash

set -e
set -x

MY_TOOLCHAIN_ROOT=${MY_TOOLCHAIN_ROOT:$HOME/toolchain}

make_toolchain_for_abi() 
{
    unset TOOLCHAIN_TOUCH
    LOCAL_ABI=$1 
    MY_TOOLCHAIN_BUILD_FLAGS="$MY_TOOLCHAIN_BUILD_FLAGS --install-dir=$MY_TOOLCHAIN_ROOT/$LOCAL_ABI"
    TOOLCHAIN_TOUCH=$MY_TOOLCHAIN_ROOT/${LOCAL_ABI}_touch
    LOCAL_PLATFORM=$MY_PLATFORM
    TOOLCHAIN_NAME=arm-linux-androideabi-4.8
    CROSS_PREFIX=arm-linux-androideabi

    if [ $LOCAL_ABI = "arm64" ];then
        LOCAL_PLATFORM=android-21
        TOOLCHAIN_NAME=aarch64-linux-android-4.9
    
        CROSS_PREFIX=aarch64-linux-android
    fi  

    export CC=$MY_TOOLCHAIN_ROOT/$LOCAL_ABI/
    if [ ! -f "$TOOLCHAIN_TOUCH" ];then
        $ANDROID_NDK/build/tools/make-standalone-toolchain.sh \
             $MY_TOOLCHAIN_BUILD_FLAGS  \
                --platform=$LOCAL_PLATFORM \
                --toolchain=$TOOLCHAIN_NAME
        touch $TOOLCHAIN_TOUCH
    
    fi  
    unset TOLLCHAIN_TOUCH
}

