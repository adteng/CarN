LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#OpenCV_INSTALL_MODULES:=on
#OPENCV_CAMERA_MODULES:=on

OPENCV_LIB_TYPE:=STATIC

include ../native/jni/OpenCV.mk

LOCAL_MODULE    := MarkingImg
LOCAL_SRC_FILES := com_teng_carn_MainActivity.cpp MarkingImg.cpp
LOCAL_LDLIBS    += -lm -llog -landroid
LOCAL_STATIC_LIBRARIES += android_native_app_glue
	
include $(BUILD_SHARED_LIBRARY) 

