LOCAL_PATH:= $(call my-dir)
MY_APP_JNI_ROOT := $(realpath $(LOCAL_PATH))
MY_PREBUILD_ROOT := $(realpath /opt/lilei/AVFX/vedit_script_android/output)

include $(call all-subdir-makefiles)

