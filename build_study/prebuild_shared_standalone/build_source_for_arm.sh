#!/usr/bin/env bash

set -e
set +x

source build_toolchain.sh


compile_for_abi()
{
    LOCAL_ABI=$1
    CROSS_PREFIX=arm-linux-androideabi

    if [ $LOCAL_ABI = "arm64" ];then
        CROSS_PREFIX=aarch64-linux-android
    fi

    export PATH="$MY_TOOLCHAIN_ROOT/$LOCAL_ABI/bin:$SAVE_PATH"
    export CC=${CROSS_PREFIX}-gcc
    export LD=${CROSS_PREFIX}-ld
    export AR=${CROSS_PREFIX}-ar
    export STRIP=${CROSS_PREFIX}-strip
    
    my_sysroot=${MY_TOOLCHAIN_ROOT}/$LOCAL_ABI/sysroot

    CAN_ABI=armeabi
    case "$LOCAL_ABI" in
        arm)
        ;;
        armv7a)
            CAN_ABI=armeabi-v7a
        ;;
        arm64)
            CAN_ABI=arm64-v8a
        ;;
        *)
        ;;
    esac
    
    mkdir -p ${MY_OUTPUT_ROOT}/$CAN_ABI/build
    cd ${MY_OUTPUT_ROOT}/$CAN_ABI/build

    case "$LOCAL_ABI" in
        arm)
        CROSS_HOST=arm-linux-android
        ;;
        armv7a)
        CROSS_HOST=armv7-linux-android
        ;;
        arm64)
        CROSS_HOST=aarch64-linux-android
        ;;
        *)
        ;;
    esac

    echo $PATH
    echo $CC
    echo $LD
    echo $AR
    echo $STRIP
    
    echo \
    ${MY_ROOT}/source/configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI \
        --host=$CROSS_HOST --with-sysroot=$my_sysroot

    ${MY_ROOT}/source/configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI \
        --host=$CROSS_HOST --with-sysroot=$my_sysroot

    make clean
    make uninstall
    make
    make install
}

MY_ABI=$1
MY_PLATFORM=android-16
SAVE_PATH=$PATH

if [ $# -ge 2 ];then
    MY_PLATFORM=$2
    case "$MY_PLATFORM" in
        android-*) 
            platform_ver=${MY_PLATFORM#android}
            if [ $platform_ver -ge 16 -o $platform_ver -le 23 ]; then
                echo "AA"
            else
                echo "platform not supported"
                exit 1
            fi 
        ;;
        *)
            echo "platform not support"
            exit 1
        ;;
    esac
fi

MY_ROOT=`pwd`
MY_OUTPUT_ROOT=$MY_ROOT/output


case "$MY_ABI" in
    all)
        make_toolchain_for_abi arm
        make_toolchain_for_abi armv7a
        make_toolchain_for_abi arm64
        compile_for_abi arm 
        compile_for_abi armv7a
        compile_for_abi arm64
    ;;
    all32)
        make_toolchain_for_abi arm
        make_toolchain_for_abi armv7a
        compile_for_abi arm 
        compile_for_abi armv7a
    ;;
    arm)
        make_toolchain_for_abi arm
        compile_for_abi arm 
    ;;
    armv7a)
        make_toolchain_for_abi armv7a
        compile_for_abi armv7a
    ;;
    arm64)
        make_toolchain_for_abi arm64
        compile_for_abi arm64
    ;;
    *)
        echo "abi not supported"
        exit 1
    ;;
esac

