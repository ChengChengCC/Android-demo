LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := EnulatorCache
LOCAL_SRC_FILES := EnulatorCache.cpp

include $(BUILD_SHARED_LIBRARY)
