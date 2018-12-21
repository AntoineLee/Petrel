#ifndef __VE_MP4_WRITER_H__
#define __VE_MP4_WRITER_H__

#include "ve_interface.h"
#include "VECommon.h"

#define AV_USLEEP_GAP 30000

typedef enum{
	VE_ENC_NONE,//for android hw encode
	VE_ENC_H264,//for android x264 soft encode
    VE_ENC_VT_H264,//for ios videotoolbox encode
    VE_ENC_FDK_AAC,
}VE_ENC_TYPE;
typedef struct{
	int m_gop{25};
	int64_t m_vBitrate{1000000};
	int m_fps{25};
	int m_width{720};
	int m_height{1280};
	AVRational m_vTimebase{1,1000000};
    int m_yuvQueueLen{5};
    VE_ENC_TYPE m_type;
}ve_enc_v_config;


typedef struct{

	int m_samplerate{44100};
	int m_channels{2};
	int m_channelLayout{AV_CH_LAYOUT_STEREO};
	enum AVSampleFormat m_samplefmt{AV_SAMPLE_FMT_S16};
	int m_bytesPerSample;
	int m_aBitrate{0};
	AVRational m_aTimebase{1,44100};

    VE_ENC_TYPE m_type;

}ve_enc_a_config;


typedef struct{
	std::string m_filename;
	std::string m_fmt;
	int m_duration{0};


	ve_enc_v_config m_vConfig;

	ve_enc_a_config m_aConfig;

	int64_t m_startTime;//(av_gettime())
}ve_mp4_writer;

typedef enum{
	VE_YUV_VT_PIXELBUFFER,//for ios hw encoding
	VE_H264_PACKET,//for android hw encoding
	VE_RGBA,//for android soft encoding
}VE_YUV_TYPE;

typedef struct{
	uint8_t* m_data{0};
	int m_len{0};
	int m_width{0};
	int m_height{0};
	int64_t m_pts{0};//毫秒
	VE_YUV_TYPE m_format{VE_YUV_VT_PIXELBUFFER};

	//for android
    int m_frameType{ANDROID_MEDIACODEC_FRAME};
}ve_enc_yuv_buffer;

typedef struct{
	uint8_t* m_data{0};
	int m_len{0};
}ve_enc_pcm_buffer;
class VEExport;
class VEMp4Writer{
public:

	VEMp4Writer(ve_mp4_writer* config);
	~VEMp4Writer();

	VE_ERR start();
	void stop();

	int getYuvLen();

	VE_ERR write(ve_enc_yuv_buffer* yuv);
	VE_ERR writeAndEnc(ve_enc_pcm_buffer* pcm);

	VE_ERR writeExtradata(uint8_t* data,int len);

	float getProgress();

	void complete();
	void vComplete();
	bool IsComplete();

	friend void* veMuxingProcess(void* context);
	friend void* veVEncodingProcess(void* context);

	//for test
	friend int testVEMp4Writer();
	friend int testVEMp4Writer2();
private:
	VE_ERR aEncode(bool flush);
	VE_ERR vEncode(bool flush);
	VE_ERR open();
	void close();
	VE_ERR vEncoding();
	VE_ERR muxing();
private:

	comn::CriticalSection m_cs;

	ve_mp4_writer m_config;

	bool m_stop{false};
	VE_ERR m_status{VE_ERR_OK};
    pthread_t	m_muxingThread{0};//muxing线程

    pthread_t	m_vEncodingThread{0};//v_encoding线程
    int m_exit{1};
	AVFormatContext *m_outputFmtCtx{NULL};
	int m_writeHeader{0};

    AVCodec* m_vEncoder{NULL};
    AVCodec* m_aEncoder{NULL};
    AVCodecContext* m_vEncContext{NULL};
    AVFrame*    m_vEncFrame{NULL};	//for encode
    AVCodecContext* m_aEncContext{NULL};
    AVFrame*    m_aEncFrame{NULL};	//for encode

    uint8_t* m_pcmBuffer{NULL};
    int m_pcmBufferPos{0};
	int64_t m_lastAPts{-1};//上一个音频帧时间，相对整个虚拟文件列表，微秒
	int64_t m_lastAPts2{-1};//上一个音频帧时间,相对m_a_timebase

    VEBitstream *m_aacBsfc{NULL};

    bool m_vCompleted{false};
    bool m_completed{false};

	VEQueue<AVPacket>	m_aMuxPktQueue;

	VEQueue<AVPacket>	m_vMuxPktQueue;

	VEQueue<ve_enc_yuv_buffer> m_filteredYuvQueue;

	VEQueue<ve_enc_yuv_buffer> m_filteredYuvSlotQueue;

    int64_t m_firstMuxTs{0};
	bool m_firstVMuxPkt{true};
    int64_t m_lastAMuxPktPts{0};
    int64_t m_lastVMuxPktPts{0};
    int64_t m_lastVMuxPktDts{0};
    int64_t m_lastAMuxPktPtsInMs{0};
    int64_t m_lastVMuxPktPtsInMs{0};
    int64_t m_lastVMuxPktDtsInMs{0};
    int m_muxAPktCount{0};
    int m_muxVPktCount{0};

    int m_flushEncoder{0};
    int m_flushEncoderDone{0};

    float m_progress{0};
};


#endif /* __VE_MP4_WRITER_H__ */
