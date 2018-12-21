LOCAL_PATH := $(call my-dir)


VIDEO_EDIT_PATH := ../../../../video_edit
VIDEO_EDIT_COMMON_PATH := ../../../../video_edit/common
VIDEO_EDIT_MEDIACODEC_PATH := ../../../../video_edit/codec/mediacodec
VIDEO_EDIT_LIBVPX_PATH := ../../../../video_edit/codec/libvpx
VIDEO_EDIT_LIBX264_PATH := ../../../../video_edit/codec/libx264
VIDEO_EDIT_LIBFDK_PATH := ../../../../video_edit/codec/libfdk
VIDEO_EDIT_SRC_PATH := ../../../../video_edit/src
VIDEO_EDIT_SRC_EXPORT_PATH := ../../../../video_edit/src/export
VIDEO_EDIT_SRC_CONFIG_PATH := ../../../../video_edit/src/config
VIDEO_EDIT_SRC_UTILS_PATH := ../../../../video_edit/src/utils
VIDEO_EDIT_SRC_AUDIO_FILTER_PATH := ../../../../video_edit/src/audio_filter
VIDEO_EDIT_SRC_THUMBNAIL_PATH := ../../../../video_edit/src/thumbnail
VIDEO_EDIT_SRC_WEBM_PATH := ../../../../video_edit/src/webm

FFMPEG_PATH := ../../../../ffmpeg/ffmpeg-3.4/ffmpeg/fftools

VEPLAYER_PATH := ../../../../player/src/sound_touch
THIRD_PART_PATH_INC := ../../../../ext/include
THIRD_PART_PATH_LIB := ../../../../ext/lib/android

FFMPEG_LIB_PATH := ../../../../ffmpeg/ffmpeg-3.4/android


include $(CLEAR_VARS)
LOCAL_MODULE := yuv
LOCAL_SRC_FILES := $(THIRD_PART_PATH_LIB)/libyuv.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avfilter
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavfilter.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := x264
LOCAL_SRC_FILES := $(THIRD_PART_PATH_LIB)/libx264.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fdk_aac
LOCAL_SRC_FILES := $(THIRD_PART_PATH_LIB)/libFraunhoferAAC.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := soundtouch
LOCAL_SRC_FILES := $(THIRD_PART_PATH_LIB)/libsoundtouch.a
include $(PREBUILT_STATIC_LIBRARY)

#qy265
#include $(CLEAR_VARS)
#LOCAL_MODULE := qycodec
#LOCAL_SRC_FILES := ../../libqy265/Android/armeabi-v7a/libqycodec.so
#include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := vpx
LOCAL_SRC_FILES := $(THIRD_PART_PATH_LIB)/libvpx.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE := gtest
LOCAL_SRC_FILES := $(THIRD_PART_PATH_LIB)/libgtest.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE := libvetest

LOCAL_CXXFLAGS += -DANDROID_NDK -Wall -D_ANDROID -DNO_CRYPTO -DCRYPTO=3 -fexceptions -std=c++11

LOCAL_CFLAGS += -std=c99


LOCAL_CXXFLAGS += -g3

LOCAL_LDLIBS += -llog -lm -lz
LOCAL_LDLIBS += -lOpenSLES
LOCAL_LDLIBS += -lGLESv2 -lEGL -landroid

LOCAL_STATIC_LIBRARIES += yuv x264 fdk_aac libavfilter libavformat libavcodec libswresample libswscale libavutil libpng zlib json-c vpx cpufeatures soundtouch

LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_LDFLAGS := --coverage

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE



LOCAL_SRC_FILES := $(VIDEO_EDIT_MEDIACODEC_PATH)/stdatomic.c \
				   $(VIDEO_EDIT_MEDIACODEC_PATH)/mediacodec.c \
				   $(VIDEO_EDIT_MEDIACODEC_PATH)/mediacodec_surface.c \
				   $(VIDEO_EDIT_MEDIACODEC_PATH)/mediacodec_sw_buffer.c \
				   $(VIDEO_EDIT_MEDIACODEC_PATH)/mediacodec_wrapper.c \
				   $(VIDEO_EDIT_MEDIACODEC_PATH)/mediacodecdec.c \
				   $(VIDEO_EDIT_MEDIACODEC_PATH)/mediacodecdec_common.c \
				   $(VIDEO_EDIT_LIBVPX_PATH)/libvpxdec.c \
				   $(VIDEO_EDIT_LIBVPX_PATH)/libvpxenc.c \
				   $(VIDEO_EDIT_SRC_AUDIO_FILTER_PATH)/af_afade.c \
				   $(VIDEO_EDIT_COMMON_PATH)/VESemaphore.cpp \
				   $(VIDEO_EDIT_COMMON_PATH)/VELock.cpp \
				   $(VIDEO_EDIT_LIBFDK_PATH)/libfdk-aacenc.c \
				   $(VIDEO_EDIT_LIBX264_PATH)/libx264.c \
				   $(VIDEO_EDIT_SRC_PATH)/ve_interface.cpp \
				   $(VIDEO_EDIT_SRC_PATH)/ve_thumbnail.cpp \
				   $(VIDEO_EDIT_SRC_PATH)/ve_webm.cpp \
				   $(VIDEO_EDIT_SRC_PATH)/ve_webm_wrap.cpp \
				   $(VIDEO_EDIT_SRC_THUMBNAIL_PATH)/VEThumbnail.cpp \
				   $(VIDEO_EDIT_SRC_UTILS_PATH)/VEUtils.cpp \
				   $(VIDEO_EDIT_SRC_UTILS_PATH)/VEBitstream.cpp \
				   $(VIDEO_EDIT_SRC_UTILS_PATH)/VESwsscale.cpp \
				   $(VIDEO_EDIT_SRC_AUDIO_FILTER_PATH)/VEAudioFilter.cpp \
				   $(VIDEO_EDIT_SRC_WEBM_PATH)/VEWebm.cpp \
				   $(VIDEO_EDIT_SRC_CONFIG_PATH)/VEConfig.cpp \
				   $(VIDEO_EDIT_SRC_EXPORT_PATH)/VEExport.cpp \
				   $(VIDEO_EDIT_SRC_EXPORT_PATH)/VEMp4Writer.cpp \
				   $(VIDEO_EDIT_SRC_EXPORT_PATH)/VESource.cpp
				    

LOCAL_SRC_FILES += $(VEPLAYER_PATH)/VESoundTouch.cpp

LOCAL_SRC_FILES += ../VEUnitTest.cpp \
					../VEUnitTestConfig.cpp \
				   ../VEUnitTestGTestMain.cpp \
				   
LOCAL_STATIC_LIBRARIES += gtest


LOCAL_C_INCLUDES := $(LOCAL_PATH)/ \
					$(LOCAL_PATH)/../../../ \
					$(LOCAL_PATH)/../../../../ext/include/ \
					$(LOCAL_PATH)/../../../../ffmpeg/ffmpeg-3.4/android/include\
					$(LOCAL_PATH)/../../../../ffmpeg/ffmpeg-3.4/ffmpeg \
					$(LOCAL_PATH)/../../../../player/src/sound_touch/ \
					$(LOCAL_PATH)/../../../../video_edit/ \
					$(LOCAL_PATH)/../../../../video_edit/src/export/ \
					$(LOCAL_PATH)/../../../../video_edit/src/utils/ \
					$(LOCAL_PATH)/../../../../video_edit/src/audio_filter/ \
					$(LOCAL_PATH)/../../../../video_edit/src/webm/ \
					$(LOCAL_PATH)/../../../../video_edit/src/thumbnail/ \
					$(LOCAL_PATH)/../../../../video_edit/src/config/ \
					$(LOCAL_PATH)/../../../../video_edit/common \
					$(LOCAL_PATH)/../../../../video_edit/codec/mediacodec \
					$(LOCAL_PATH)/../../../../render_common/jni/

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

include $(BUILD_EXECUTABLE)


