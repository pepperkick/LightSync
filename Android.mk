# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
#
LOCAL_PATH := $(call my-dir)

TARGET_ARCH_ABI := armeabi-v7a

HUE_SOURCES := \
	include/hueplusplus/ExtendedColorHueStrategy.cpp \
	include/hueplusplus/ExtendedColorTemperatureStrategy.cpp \
	include/hueplusplus/Hue.cpp \
	include/hueplusplus/HueCommandAPI.cpp \
	include/hueplusplus/HueLight.cpp \
	include/hueplusplus/jsoncpp.cpp \
	include/hueplusplus/SimpleBrightnessStrategy.cpp \
	include/hueplusplus/SimpleColorHueStrategy.cpp \
	include/hueplusplus/SimpleColorTemperatureStrategy.cpp \
	include/hueplusplus/UPnP.cpp \
	include/hueplusplus/LinHttpHandler.cpp

HUE_INCLUDES := $(LOCAL_PATH)/include

HUE_CFLAGS := -w -fexceptions

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(HUE_SOURCES)
LOCAL_C_INCLUDES += $(HUE_INCLUDES)
LOCAL_CFLAGS += $(HUE_CFLAGS)
LOCAL_MODULE := libhue
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog
LOCAL_MODULE    := lightsync
LOCAL_CFLAGS += -fexceptions
LOCAL_SRC_FILES := src/main.cpp include/utils/utils.cpp include/inline-hook/inlineHook.c include/inline-hook/relocate.c
LOCAL_STATIC_LIBRARIES := libhue
include $(BUILD_SHARED_LIBRARY)
