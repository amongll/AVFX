#!/usr/bin/env bash

set -e
set +x

source build_toolchain.sh

EXTRA_CFLAGS=
EXTRA_LDFLAGS=


compile_for_abi()
{
    SOURCE_DIR=$2
    LOCAL_ABI=$1
    CROSS_PREFIX=arm-linux-androideabi

    if [ $LOCAL_ABI = "arm64" ];then
        CROSS_PREFIX=aarch64-linux-android
    fi

    export PATH="$MY_TOOLCHAIN_ROOT/$LOCAL_ABI/bin:$SAVE_PATH"
 #   export AS=${CROSS_PREFIX}-as
    export CC=${CROSS_PREFIX}-gcc
    export LD=${CROSS_PREFIX}-ld
    export AR=${CROSS_PREFIX}-ar
    export STRIP=${CROSS_PREFIX}-strip
    export CXX=${CROSS_PREFIX}-g++
    
    my_sysroot=${MY_TOOLCHAIN_ROOT}/$LOCAL_ABI/sysroot

    CAN_ABI=armeabi
    case "$LOCAL_ABI" in
        arm)
            EXTRA_CFLAGS='-march=armv5te -mtune=arm9tdmi -msoft-float'
        ;;
        armv7a)
            EXTRA_CFLAGS='-march=armv7-a -mcpu=cortex-a8 -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb'
            EXTRA_LDFLAGS='-Wl,--fix-cortex-a8'
            CAN_ABI=armeabi-v7a
        ;;
        arm64)
            CAN_ABI=arm64-v8a
        ;;
        *)
        ;;
    esac
    
    [ -f ${MY_ROOT}/${SOURCE_DIR}_config_opts.sh ] && source  ${MY_ROOT}/${SOURCE_DIR}_config_opts.sh
    mkdir -p ${MY_OUTPUT_ROOT}/$CAN_ABI/build/${SOURCE_DIR}
    cd ${MY_OUTPUT_ROOT}/$CAN_ABI/build/${SOURCE_DIR}
    curdir=$(pwd)
    if [ ${MY_VPATH_OK} -eq 0 ]; then
        cp -rf ${MY_ROOT}/${SOURCE_DIR}/* ${curdir}
    fi


    PKG_CONFIG_PATH=${MY_OUTPUT_ROOT}/$CAN_ABI/install/lib/pkgconfig

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
    
    if [ ${MY_VPATH_OK} -eq 0 ]; then
        echo \
        ./configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI/install \
         --host=$CROSS_HOST --with-sysroot=$my_sysroot ${MY_CONFIG_OPTS} \
         --extra-cflags="$EXTRA_CFLAGS"  --extra-ldflags="$EXTRA_LDFLAGS" \
        --sysroot=$my_sysroot

        ./configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI/install \
         --host=$CROSS_HOST --with-sysroot=$my_sysroot ${MY_CONFIG_OPTS} \
         --extra-cflags="$EXTRA_CFLAGS"  --extra-ldflags="$EXTRA_LDFLAGS" \
        --sysroot=$my_sysroot
        
    else
        echo \
        ${MY_ROOT}/${SOURCE_DIR}/configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI/install \
         --host=$CROSS_HOST --with-sysroot=$my_sysroot ${MY_CONFIG_OPTS} \
         --extra-cflags="$EXTRA_CFLAGS"  --extra-ldflags="$EXTRA_LDFLAGS" \
        --sysroot=$my_sysroot

        ${MY_ROOT}/${SOURCE_DIR}/configure --prefix=${MY_OUTPUT_ROOT}/$CAN_ABI/install \
         --host=$CROSS_HOST --with-sysroot=$my_sysroot ${MY_CONFIG_OPTS} \
         --extra-cflags="$EXTRA_CFLAGS"  --extra-ldflags="$EXTRA_LDFLAGS" \
        --sysroot=$my_sysroot
    fi

    make clean
    make uninstall
    make
    make install
}
MY_PLATFORM=android-16

for arg in $@
do
    case $arg in
        --platform=*) MY_PLATFORM=${arg#--platform=}  ;;  
        --enable-debug) MY_DEBUG=1 ;;
        --abi=*) MY_ABI=${arg#--abi=} ;;
        --project=*) MY_PROJ=${arg#--project=} ;;
    esac
done

SAVE_PATH=$PATH

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

case "$MY_ABI" in
    all)
        make_toolchain_for_abi arm
        make_toolchain_for_abi armv7a
        make_toolchain_for_abi arm64
    ;;
    all32)
        make_toolchain_for_abi arm
        make_toolchain_for_abi armv7a
    ;;
    arm)
        make_toolchain_for_abi arm
    ;;
    armv7a)
        make_toolchain_for_abi armv7a
    ;;
    arm64)
        make_toolchain_for_abi arm64
    ;;
esac

MY_ROOT=`pwd`
MY_OUTPUT_ROOT=$MY_ROOT/output

compile_spec() {
        spec=$1
        case "$MY_ABI" in
                all)
                        compile_for_abi arm $spec
                        compile_for_abi armv7a $spec
                        compile_for_abi arm64 $spec
                        ;;
                all32)
                        compile_for_abi arm  $spec
                        compile_for_abi armv7a $spec
                        ;;
                arm)
                        compile_for_abi arm  $spec
                        ;;
                armv7a)
                        compile_for_abi armv7a $spec
                        ;;
                arm64)
                        compile_for_abi arm64 $spec
                        ;;
                *)
                        echo "abi not supported"
                        exit 1
                        ;;
                esac
}


compile_spec $MY_PROJ
#compile_spec vedit_script 
