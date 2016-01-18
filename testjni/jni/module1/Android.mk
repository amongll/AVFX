LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := my.c
LOCAL_LDLIBS:= -llog -ldl
LOCAL_SHARED_LIBRARIES := prebuild_shared_ndkbuild prebuild_shared_standalone
LOCAL_MODULE:=  testjni_module1
include $(BUILD_SHARED_LIBRARY)

