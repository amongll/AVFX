LOCAL_PATH:= $(call my-dir)
MY_APP_JNI_ROOT := $(realpath $(LOCAL_PATH))
MY_PREBUILD_ROOT := $(realpath $(MY_APP_JNI_ROOT)/../..)

include $(call all-subdir-makefiles)

