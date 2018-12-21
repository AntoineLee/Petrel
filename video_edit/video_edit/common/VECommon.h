#ifndef __VECOMMON__
#define __VECOMMON__

#define FFMPEG_TAG "[VE_AV_LOG]"

#include "libyuv.h"
#include <sys/stat.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <deque>
#include <algorithm>
#include <map>
#include <deque>
#include <thread>
#include <vector>
#include <list>

#include "VELog.h"
#include "TCriticalSection.h"
#include "TConditionVariable.h"

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif


extern "C"{

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/pixdesc.h"
#include "libavutil/mem.h"
#include "libswresample/swresample.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavutil/opt.h>



int ffmpeg_translate(int argc, char **argv,int bench);
av_cold void avcodec_register(AVCodec *codec);
int GetAndroidVersion();

}

extern "C"{
    int64_t av_gettime(void);
    int av_usleep(unsigned usec);
    int my_av_videotoolbox_default_init(AVCodecContext *avctx);
    void my_av_videotoolbox_default_free(AVCodecContext *avctx);
    int ff_stream_add_bitstream_filter(AVStream *st, const char *name, const char *args);
    AVHWAccel *av_hwaccel_next(const AVHWAccel *hwaccel);
    int av_jni_set_java_vm(void *vm, void *log_ctx);
}
extern "C" AVCodec ff_libqy265_encoder;
extern "C" AVCodec ff_libqy265_decoder;
//extern "C" AVCodec ff_libopenh264_encoder;
extern "C" AVCodec ff_libx264_encoder;
extern "C" AVCodec ff_libfdk_aac_encoder;
extern "C" AVCodec ff_h264_decoder;
#ifdef __ANDROID__
extern "C" AVCodec ff_libmedia_codec_enc_encoder;
extern "C" AVCodec ff_h264_mediacodec_decoder;
extern "C" AVCodec ff_hevc_mediacodec_decoder;
#endif
#ifdef __APPLE__
extern "C" AVCodec ff_h264_videotoolbox_encoder;
extern "C" AVHWAccel my_ff_h264_videotoolbox_hwaccel;
extern "C" AVCodec h264_videotoolbox_decoder;
extern "C" AVCodec hevc_videotoolbox_decoder;
extern "C" AVCodec ff_aac_at_encoder;
extern "C" AVCodec ff_aac_at_decoder;
#endif
extern "C" AVCodec ff_libvpx_vp8_decoder;
extern "C" AVCodec ff_libvpx_vp9_decoder;
extern "C" AVCodec ff_libvpx_vp8_encoder;
extern "C" int termination_ffmpeg_process;
extern "C" int ffmepg_progress_percent;
extern "C" int ffmepg_progress_duration;



void veLogCallbackFfmpeg(void *ptr,int level,const char *fmt,va_list vl);


#include "VEQueue.h"
#include "VEBitstream.h"

#endif
