#!/usr/bin/env bash

source build_toolchain.sh

set -e
set +x

compile_for_abi()
{
    LOCAL_ABI=$1
    cd $MY_ROOT
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
            CAN_ABI=armeabi
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

    COMMON_MY_CFG_FLAGS=
    MY_EXTRA_CFLAGS="-DANDROID -D__ANDROID__ -std=c99"
    MY_EXTRA_LDFLAGS=
    source ./module.sh
    
    rm -rf ${MY_OUTPUT_ROOT}/$CAN_ABI/build
    mkdir -p  ${MY_OUTPUT_ROOT}/$CAN_ABI/build
    rm -fr ${MY_OUTPUT_ROOT}/$CAN_ABI/source
    cp -r ${MY_ROOT}/source  ${MY_OUTPUT_ROOT}/$CAN_ABI/source
    cd ${MY_OUTPUT_ROOT}/$CAN_ABI/source
    
    COMMON_MY_CFG_FLAGS="$COMMON_MY_CFG_FLAGS --target-os=Android --ff-root=/opt/lilei/ijkplayer-master/android/contrib/build \
        --ff-libname=ijkffmpeg"

    MY_EXTRA_CFLAGS="$MY_EXTRA_CFLAGS --sysroot=$my_sysroot"
    MY_EXTRA_LDFLAGS="$MY_EXTRA_LDFLAGS --sysroot=$my_sysroot"

    case "$LOCAL_ABI" in
        arm)
        CROSS_HOST=arm-linux-android
        COMMON_MY_CFG_FLAGS="$COMMON_MY_CFG_FLAGS --ff-arch=armv5 --target-arch=arm"
        MY_EXTRA_CFLAGS="$MY_EXTRA_CFLAGS -march=armv5te -mtune=arm9tdmi -msoft-float"
        ;;
        armv7a)
        CROSS_HOST=armv7a-linux-android
        COMMON_MY_CFG_FLAGS="$COMMON_MY_CFG_FLAGS --ff-arch=armv7a --target-arch=armv7-a"
        MY_EXTRA_CFLAGS="$MY_EXTRA_CFLAGS -march=armv7-a -mcpu=cortex-a8 -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb"
        MY_EXTRA_LDFLAGS="$MY_EXTRA_LDFLAGS -Wl,--fix-cortex-a8"
        ;;
        arm64)
        MY_PLATFORM=android-21
        COMMON_MY_CFG_FLAGS="$COMMON_MY_CFG_FLAGS --ff-arch=arm64 --target-arch=arm64-v8a"
        MY_EXTRA_CFLAGS="$MY_EXTRA_CFLAGS -DANDROID_XLOCALE"
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

    MY_CFLAGS="-O3 -Wall -pipe \
     -DANDROID -DNDEBUG"

    if [ ""$MY_DEBUG = "1" ]; then
        MY_CFLAGS="-g -O0 -Wall -pipe \
            -DANDROID"
    fi 

    COMMON_MY_CFG_FLAGS="$COMMON_MY_CFG_FLAGS --target-os=Android"
    COMMON_MY_CFG_FLAGS="$COMMON_MY_CFG_FLAGS --android-platform=$MY_PLATFORM"
    
    echo \
    ./configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI/build \
        --host=$CROSS_HOST --with-sysroot=$my_sysroot \
        $COMMON_MY_CFG_FLAGS \
        $MY_CFLAGS \
        --extra-cflags="$MY_CFLAGS $MY_EXTRA_CFLAGS" \
        --extra-ldflags="$MY_EXTRA_LDFLAGS"
    

    ./configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI/build \
        --host=$CROSS_HOST --with-sysroot=$my_sysroot \
        $COMMON_MY_CFG_FLAGS \
        $MY_CFLAGS \
        --extra-cflags="$MY_CFLAGS $MY_EXTRA_CFLAGS" \
        --extra-ldflags="$MY_EXTRA_LDFLAGS"

    make clean
    make uninstall
    make
    make install
}

MY_PLATFORM=android-9
SAVE_PATH=$PATH

for arg in $@
do
    case $arg in
        --platform=*) MY_PLATFORM=${arg#--platform=}  ;;
        --enable-debug) MY_DEBUG=1 ;;
        *)MY_ABI=$arg ;;
    esac
done

case "$MY_PLATFORM" in
        android-*) 
            platform_ver=${MY_PLATFORM#android}
            if [ $platform_ver -ge 9 -o $platform_ver -le 23 ]; then
                echo "platform $MY_PLATFORM accepted"
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

MY_ROOT=`pwd`
MY_OUTPUT_ROOT=$MY_ROOT/output

MY_TOOLCHAIN_ROOT=${MY_TOOLCHAIN_ROOT:-"/opt/lilei/android_compile_toolchain"}
echo "toolchain root:$MY_TOOLCHAIN_ROOT"


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

