#include "ve_webm.h"
#include <string>
#include <pthread.h>

#include "TCriticalSection.h"
#include "VEQueue.h"

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C"{

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/pixfmt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/mem.h"
#include "libswresample/swresample.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavutil/opt.h>
int av_usleep(unsigned usec);
}

extern "C" AVCodec ff_libvpx_vp8_encoder;

#ifndef __VE_WEBM__
#define __VE_WEBM__

class VEWebm{
public:

	friend void * veWebmProcess(void* context);

	VEWebm(ve_webm_config *config);
	~VEWebm();
	VE_WEBM_ERR start();
	VE_WEBM_ERR stop();
	VE_WEBM_ERR sendFrame(ve_webm_frame* frame);
private:
	VE_WEBM_ERR openOutputFile();
	VE_WEBM_ERR closeOutputFile();
	VE_WEBM_ERR muxing();

private:
	ve_webm_config m_config;
	std::string m_outputFilename;

	int m_exit{1};
	comn::CriticalSection m_cs;
	pthread_t m_worker{0};

	AVFormatContext *m_outputFmtCtx{NULL};
	AVCodec* m_vEncoder{NULL};

    AVCodecContext* m_vEncContext{NULL};//不用释放
    AVFrame*    m_vEncFrame{NULL};	//for encode
    AVFrame*    m_vFrame{NULL};

    int64_t m_lastVMuxPktPts{0};
    int64_t m_lastVMuxPktDts{0};
    bool m_firstVMuxPkt{true};

    bool m_flushEncoder{false};

    int m_count{0};

	VEQueue<AVPacket>	m_vMuxPktQueue;

	int64_t m_vBitrate{1000};
	AVRational m_vTimebase{1,1000};
	int m_gop{100};

    int m_swsCtxWidth{0};
    int m_swsCtxHeight{0};
    int m_swsCtxFormat{-1};
    SwsContext* m_swsCtx{NULL};
};


#endif /* __VE_WEBM__ */
