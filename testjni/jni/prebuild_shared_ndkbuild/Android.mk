LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE :=  prebuild_shared_ndkbuild
LOCAL_SRC_FILES := $(MY_PREBUILD_ROOT)/prebuild_shared_ndkbuild/libs/$(TARGET_ARCH_ABI)/libprebuild_shared_ndkbuild.so
include $(PREBUILT_SHARED_LIBRARY)


