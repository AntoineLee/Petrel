#ifndef __VE_SOURCE_H__
#define __VE_SOURCE_H__

#include "VEConfig.h"
#include "ve_interface.h"
#define VE_SOURCE_PCM_SAMPLE_SIZE 4096

class VEAudioFilter;
class VEBitstream;
class VESwsscale;
class VESoundTouch;
typedef enum{
	VE_SOURCE_TYPE_NONE = -1,
	VE_SOURCE_TYPE_VIDEO,//视频
	VE_SOURCE_TYPE_PICTURE,//图片
	VE_SOURCE_TYPE_AUDIO,//音频
}VE_SOURCE_TYPE;


class VESourceListener{
public:
    virtual void setStatus(int status) = 0;
};

typedef struct VEPacket{
    AVPacket m_pkt;
    int m_loopCount{0};
}VEPacket;

typedef struct{
    uint8_t* m_data{NULL};//数据内容
    int m_len{0};//数据长度
    int m_format{VE_COLOR_YUV420P};
    int m_width{0};
    int m_height{0};
    int m_rotate{VE_ROTATE_0};
    int m_clipTs{-1};//clip原始时间
    int m_ts{-1};//时间
    int m_vCurTime{-1};//虚拟时间
    
    int m_refCount{0};

    int m_transitionId{-1};
    int m_transitionFrame{0};
    const char* m_transitionAction{NULL};

}ve_dec_yuv_buffer;
class VESource;
typedef struct{
    uint8_t* m_data{NULL};
    int m_len{0};
    int m_samplerate{44100};
    int m_sampleSize{1024};
    int m_ts{-1};//时间ms

    VESource* m_source{NULL};
}ve_dec_pcm_buffer;


typedef struct{
	int m_volume;
	float m_samplerateForSpeed;


	//for afde
	VE_AUDIO_FILTER_TYPE m_fade{VE_AUDIO_FILTER_NONE};
	int64_t m_startSamples{0};
	int64_t m_nbSamples{0};
	int m_gainMin{0};
	int m_gainMax{100};
}ve_audio_filter_param;

class VEExport;
class VESource{

public:
	static VE_ERR getSourceInfo(const char *filename,ve_clip_info * info,slv_info* slv = NULL);
	static VE_ERR openSource(AVFormatContext **fmtCtx,const char* filename);
	static void getAudioFilterString(ve_audio_filter_param *param,char* outStr/* len > 256*/);
	friend class VEExport;
	friend int testVESourceH264Video();
public:
	VESource(VE_SOURCE_TYPE type,VEConfig *config,VEClipData* clipData,VESourceListener* listener);
	~VESource();

	friend void* veSourceReadpacketProcess(void* context);
	friend void* veSourceVideoProcess(void* context);
	friend void* veSourceAudioProcess(void* context);
	friend int testVESourceGetDurationForPicture();

	VE_ERR start();
	VE_ERR stop();

	bool isStart();
	bool isStop();

	//gotPicture 1 获得数据，0 当前时间没有数据
	//eof 1,素材结束

	VE_ERR getYuvBuffer(ve_dec_yuv_buffer* yuv,int *gotPicture,int* eof,int vCurTime);

	//gotPcm 1获得数据，0 当前时间没有数据
	VE_ERR getPcmBuffer(ve_dec_pcm_buffer* pcm,int* gotPcm,int* eof,int vCurTime);

	void returnYuv();
	void returnPcm();

	static void setUnitTest(bool test);

	int getClipId();
	VEClipData* getClipData();
private:
	VE_ERR init();
	VE_ERR release();
	void signal();
	bool isOpen();
	VE_ERR open();

	VE_ERR readPacket();
	VE_ERR videoProcess();
	VE_ERR audioProcess();

	VE_ERR prepareYuv(int loopCount);
	VE_ERR preparePcm(int loopCount);

	VE_ERR refreshAudioFilters(int curTime,int slvCurTime = -1);

private:
	static int getDurationForPicture(AVFormatContext *fmtCtx,int streamIndex);


private:
	VE_SOURCE_TYPE m_type;

	VEConfig *m_config{NULL};

	VEClipData m_clipData;

	static bool m_unitTest;

    VESourceListener* m_listener{NULL};

    pthread_t	m_readThread{0};//读包线程
    pthread_t	m_videoThread{0};//视频解码线程
    pthread_t	m_audioThread{0};//音频解码线程

    int m_exit{1};
    VE_ERR	m_status{VE_ERR_OK};

    int m_stop{0};
    int m_eof{0};
	int m_seekTo{0};
	int m_seek{0};
	int m_loop{0};
	int m_loopCount{0};//
	int m_clipDuration{0};//切片时长，不变速
	int m_offset{0};//当前位置，不变速

    comn::CriticalSection m_cs;


	VEQueue<VEPacket>	m_aPktQueue;

	VEQueue<VEPacket>	m_vPktQueue;

	VEQueue<ve_dec_yuv_buffer>	m_yuvQueue;
	VEQueue<ve_dec_yuv_buffer>	m_yuvPool;
	VEQueue<ve_dec_pcm_buffer>	m_pcmQueue;
	VEQueue<ve_dec_pcm_buffer>	m_pcmPool;

	ve_dec_yuv_buffer m_yuvBuffer;
	ve_dec_pcm_buffer m_purPcm;

    //source相关
    AVFormatContext *m_fmtCtx{NULL};
    int m_videoStreamIndex{-1};
    int m_audioStreamIndex{-1};

    int m_flushDecoderDone{0};

    AVCodec* m_vDecoder{NULL};
    AVCodec* m_aDecoder{NULL};
    AVCodecContext* m_vDecContext{NULL};
    AVCodecContext* m_aDecContext{NULL};
    AVFrame*    m_vFrame{NULL};	//for decode
    AVFrame*    m_aFrame{NULL};	//for decode


	AVRational m_aTimebase{1,44100};
	AVRational m_vTimebase{1,1000000};

	int m_aDuration{0};
	int m_vDuration{0};

	int64_t m_firstVPktPts{-1};
	int64_t m_firstAPktPts{-1};

    
    int m_firstVPktPtsInMs{0};
    int m_firstAPktPtsInMs{0};
    int m_firstPktPtsInMs{-1};

    int m_foundFirstPktPts{0};

    int m_curPcmPts{-1};

	VEBitstream *m_h264Mp4ToAnnexBBsfc{NULL};

	VESwsscale* m_swsscale{NULL};

	void *m_vtMode{NULL};

	int m_curVCount{0};

    uint8_t* m_yuv{NULL};		//for decode
    int m_yuvLen{0};		//for decode


    VEAudioFilter* m_audioFilters{NULL};
    VESoundTouch *m_soundTouch{NULL};
    uint8_t *m_soundTouchBuffer{NULL};

	int m_samplerate{44100};
	int m_channels{2};
	int m_channelLayout{AV_CH_LAYOUT_STEREO};
	enum AVSampleFormat m_samplefmt{AV_SAMPLE_FMT_S16};

    uint8_t* m_pcmBuffer{0};
    int m_pcmBufferLen{0};
    int m_pcmPos{0};
    int m_pcmLeft{0};
    int m_outputPcmLen{VE_SOURCE_PCM_SAMPLE_SIZE};

    int m_firstPcm{1};

};

#endif
