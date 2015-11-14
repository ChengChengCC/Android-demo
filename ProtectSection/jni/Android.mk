LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := protect_section
#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := protect_section.cpp
LOCAL_ARM_MODE  := arm
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

APP_ABI := armeabi armeabi-v7a x86
include $(BUILD_SHARED_LIBRARY)



