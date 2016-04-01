version=0.9.9
soversion=6
prefix=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install
libdir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/lib
bindir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/bin
datadir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/share
mandir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/share/man
extra_versioning=false
melt_noversion=false
targetos=Android
targetarch=arm64-v8a
arch=arm64
DEBUG_FLAGS=-g -O0 -DDEBUG
LARGE_FILE=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CFLAGS+=-Wall -DPIC $(TARGETARCH) $(TARGETCPU) $(OPTIMISATIONS) $(MMX_FLAGS) $(SSE_FLAGS) $(SSE2_FLAGS) $(DEBUG_FLAGS) $(LARGE_FILE)
CFLAGS+=-fPIC -pthread -g -O0 -Wall -pipe             -DANDROID -DANDROID -D__ANDROID__ -std=c99 --sysroot=/opt/lilei/android_compile_toolchain/arm64/sysroot -DANDROID_XLOCALE
SHFLAGS=-shared
RDYNAMIC=-rdynamic
LDFLAGS+=-Wl,--as-needed  --sysroot=/opt/lilei/android_compile_toolchain/arm64/sysroot
LIBSUF=.so
moduledir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/lib/mlt
mltdatadir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/share/mlt
unversionedmoduledir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/lib/mlt
unversionedmltdatadir=/opt/lilei/AVFX/vedit_script_android/output/arm64-v8a/install/share/mlt
meltname=melt
