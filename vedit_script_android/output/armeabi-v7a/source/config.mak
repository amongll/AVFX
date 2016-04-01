version=0.9.9
soversion=6
prefix=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install
libdir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/lib
bindir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/bin
datadir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/share
mandir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/share/man
extra_versioning=false
melt_noversion=false
targetos=Android
targetarch=armv7-a
arch=armv7a
DEBUG_FLAGS=-g -O0 -DDEBUG
LARGE_FILE=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CFLAGS+=-Wall -DPIC $(TARGETARCH) $(TARGETCPU) $(OPTIMISATIONS) $(MMX_FLAGS) $(SSE_FLAGS) $(SSE2_FLAGS) $(DEBUG_FLAGS) $(LARGE_FILE)
CFLAGS+=-fPIC -pthread -g -O0 -Wall -pipe             -DANDROID -DANDROID -D__ANDROID__ -std=c99 --sysroot=/opt/lilei/android_compile_toolchain/armv7a/sysroot -march=armv7-a -mcpu=cortex-a8 -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb
SHFLAGS=-shared
RDYNAMIC=-rdynamic
LDFLAGS+=-Wl,--as-needed  --sysroot=/opt/lilei/android_compile_toolchain/armv7a/sysroot -Wl,--fix-cortex-a8
LIBSUF=.so
moduledir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/lib/mlt
mltdatadir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/share/mlt
unversionedmoduledir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/lib/mlt
unversionedmltdatadir=/opt/lilei/AVFX/vedit_script_android/output/armeabi-v7a/install/share/mlt
meltname=melt
