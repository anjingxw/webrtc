LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)

WEBRTC_LIBS_PATH := ../../../build_android_libs/lib2018


LOCAL_MODULE := webrtc
LOCAL_SRC_FILES := $(WEBRTC_LIBS_PATH)/$(TARGET_ARCH_ABI)/libwebrtc.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

# LOCAL_MODULE := baseevent
# LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarsbaseevent.a
# include $(PREBUILT_STATIC_LIBRARY)
# include $(CLEAR_VARS)

LOCAL_MODULE := xnrtc

LOCAL_CFLAGS += -DWEBRTC_ANDROID -DWEBRTC_POSIX

SRC := $(wildcard $(LOCAL_PATH)/../AudioFactory/*.cpp)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(LOCAL_PATH)/../CallWraper/*.cpp)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(LOCAL_PATH)/../Dtmf/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(LOCAL_PATH)/../Udp/*.cpp)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(LOCAL_PATH)/*.cpp)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ $(LOCAL_PATH)/../AudioFactory  $(LOCAL_PATH)/../CallWraper
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Udp  $(LOCAL_PATH)/../Dtmf
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../src

LOCAL_STATIC_LIBRARIES += webrtc
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM -ldl -lstdc++ -lOpenSLES
#LOCAL_LDFLAGS += libwebrtc.a
LOCAL_CPPFLAGS += -fno-rtti -fvisibility=default
LOCAL_LDFLAGS += -Wl
include $(BUILD_SHARED_LIBRARY)