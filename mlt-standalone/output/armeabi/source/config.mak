version=0.9.9
soversion=6
prefix=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build
libdir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/lib
bindir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/bin
datadir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/share
mandir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/share/man
extra_versioning=false
melt_noversion=false
targetos=Android
targetarch=arm
arch=armv5
DEBUG_FLAGS=-g -O0 -DDEBUG
LARGE_FILE=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CFLAGS+=-Wall -DPIC $(TARGETARCH) $(TARGETCPU) $(OPTIMISATIONS) $(MMX_FLAGS) $(SSE_FLAGS) $(SSE2_FLAGS) $(DEBUG_FLAGS) $(LARGE_FILE)
CFLAGS+=-fPIC -pthread -g -O0 -Wall -pipe             -DANDROID -DANDROID -D__ANDROID__ -std=c99 --sysroot=/opt/lilei/android_compile_toolchain/arm/sysroot -march=armv5te -mtune=arm9tdmi -msoft-float
SHFLAGS=-shared
RDYNAMIC=-rdynamic
LDFLAGS+=-Wl,--as-needed  --sysroot=/opt/lilei/android_compile_toolchain/arm/sysroot
LIBSUF=.so
moduledir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/lib/mlt
mltdatadir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/share/mlt
unversionedmoduledir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/lib/mlt
unversionedmltdatadir=/opt/lilei/AVFX/mlt-standalone/output/armeabi/build/share/mlt
meltname=melt
