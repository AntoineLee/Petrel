#ifndef __VE_AUDIO_FILTER_H__
#define __VE_AUDIO_FILTER_H__

extern "C"{
#include "libavutil/samplefmt.h"
#include "libavutil/channel_layout.h"
}

struct AVCodecContext;
struct AVFrame;
struct AVRational;
struct AVFilter;
struct AVFilterContext;
struct AVFilterInOut;
struct AVFilterGraph;

class VEAudioFilter{
public:
	friend int testVEAudioFilter();
public:
	VEAudioFilter();
	~VEAudioFilter();
	bool isInit();
	int init(AVCodecContext* ctx);
	int addFilters(const char* filtersExpr);
	int addFilters(AVCodecContext* ctx,const char* filtersExpr);
	int process(AVFrame * srcFrame,short** dest_pcm,int *destPcmLen);
	bool isSameFilters(const char* filtersExpr);

	static void initialize();
private:


	void releaseMem();

private:

	bool m_inited{false};
	char* m_filters{0};
	int m_samplerate{44100};
	int m_channels{2};
	int m_channelLayout{AV_CH_LAYOUT_STEREO};
	enum AVSampleFormat m_samplefmt{AV_SAMPLE_FMT_S16};
	AVRational m_timebase{1,44100};


	uint8_t* m_pcmBuf{NULL};
	int m_pcmBufDataLen{0};
	int m_pcmBufLen{0};

    AVFrame *m_frame{NULL};
    AVFrame *m_filtFrame{NULL};

    AVFilter *m_aBufferSrc{NULL};
    AVFilter *m_aBufferSink{NULL};

    AVFilterContext *m_bufferSinkCtx{NULL};
    AVFilterContext *m_bufferSrcCtx{NULL};
    AVFilterGraph *m_filterGraph{NULL};

    AVFilterInOut *m_outputs{NULL};
    AVFilterInOut *m_inputs{NULL};
};

#endif
