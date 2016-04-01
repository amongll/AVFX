LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ijkffmpeg
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/lib/libijkffmpeg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jansson 
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/lib/libjansson.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_framework 
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/lib/libmlt.so
LOCAL_EXPORT_C_INCLUDES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/include/mlt
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_core
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/lib/mlt/libmltcore.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mlt_avformat
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/lib/mlt/libmltavformat.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vedit_script
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/lib/libvedit_script.so
LOCAL_EXPORT_C_INCLUDES := $(MY_PREBUILD_ROOT)/$(TARGET_ARCH_ABI)/install/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vedit_script_jni
LOCAL_SHARED_LIBRARIES := jansson mlt_framework vedit_script
LOCAL_LDLIBS += -llog -landroid
#LOCAL_CFLAGS += ""
#ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
#    LOCAL_CFLAGS += " -DANDROID_XLOCALE"
#endif
ifeq ($(NDK_DEBUG),1)
	LOCAL_CFLAGS += " -DDEBUG"
endif

LOCAL_C_INCLUDES += $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/include 
LOCAL_C_INCLUDES += $(MY_PREBUILD_ROOT)/mlt-standalone/output/$(TARGET_ARCH_ABI)/build/include/mlt
LOCAL_SRC_FILES := VEditAndroid.cpp VEditMltRuntime_android.cpp VEditVM_android.cpp

include $(BUILD_SHARED_LIBRARY)

