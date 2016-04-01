LOCAL_PATH:= $(call my-dir)



include $(CLEAR_VARS)
LOCAL_MODULE:=  prebuild_shared_standalone
LOCAL_SRC_FILES:= $(MY_PREBUILD_ROOT)/prebuild_shared_standalone/output/$(TARGET_ARCH_ABI)/lib/libprebuild_shared_standalone.so
include $(PREBUILT_SHARED_LIBRARY)



