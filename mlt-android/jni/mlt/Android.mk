LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_framework 
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/lib/libmlt.so
LOCAL_EXPORT_C_INCLUDES := $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_core
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/lib/mlt/libmltcore.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_avformat
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/lib/mlt/libmltavformat.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_android
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/lib/mlt/libmlt-android.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_jni
LOCAL_SHARED_LIBRARIES := mlt_framework 
LOCAL_LDLIBS += -llog -landroid
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
    LOCAL_CFLAGS += "-DANDROID_XLOCALE"
endif
ifeq ($(NDK_DEBUG),1)
	LOCAL_CFLAGS += "-DDEBUG"
endif
LOCAL_SRC_FILES := MltFactoryJNI.c MltJNI.c
LOCAL_C_INCLUDES += $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/include/mlt
include $(BUILD_SHARED_LIBRARY)

