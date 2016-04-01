LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=my.c

LOCAL_LDLIBS:= -llog
LOCAL_MODULE:=prebuild_shared_ndkbuild

include $(BUILD_SHARED_LIBRARY)

