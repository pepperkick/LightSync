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


include $(CLEAR_VARS)
LOCAL_MODULE := hook
#LOCAL_SRC_FILES := $(LOCAL_PATH)/../obj/local/armeabi-v7a/libhook.a
#LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include

include $(CLEAR_VARS)
ALL_SOURCES := \
	include/librws/rws_common.c \
	include/librws/rws_error.c \
	include/librws/rws_frame.c \
	include/librws/librws.c \
	include/librws/rws_list.c \
	include/librws/rws_memory.c \
	include/librws/rws_socketpriv.c \
	include/librws/rws_socketpub.c \
	include/librws/rws_string.c \
	include/librws/rws_thread.c


ALL_INCLUDES := $(LOCAL_PATH)/include

ALL_CFLAGS := -w

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(ALL_SOURCES)
LOCAL_C_INCLUDES += $(ALL_INCLUDES)
LOCAL_CFLAGS += $(ALL_CFLAGS)
LOCAL_MODULE := librws
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog
LOCAL_MODULE    := gsi
LOCAL_SRC_FILES := main.c include/utils/utils.c include/inline-hook/inlineHook.c include/inline-hook/relocate.c
LOCAL_STATIC_LIBRARIES := librws
include $(BUILD_SHARED_LIBRARY)
