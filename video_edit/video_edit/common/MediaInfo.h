#ifndef MEDIA_INFO_H
#define MEDIA_INFO_H

#include <string.h>
#include <stdint.h>

typedef enum 
{
	JPLAYER_SOFT_DECODER = 0,
	JPLAYER_XBMC_DECODER
}JPlayer_Decoder_TYPE;

typedef enum
{
    JPLAYER_PKT_DATA = 0,
    JPLAYER_PKT_ENDSTREAM = 10
}JPLAYER_SPE_FRAME;


///媒体数据发送结构
typedef struct 
{
	uint8_t* data;      ///媒体数据
	int      size;      ///媒体数据长度
	int      flag;      ///媒体帧类型
	int      seq;
	int      isKey;
	int64_t timestamp; ///数据帧产生的时间，milliseconds
    int64_t dts;
    uint64_t duration;
    int      packet_type;
}StreamPacket;


///媒体数据帧类型
typedef enum 
{
	JPLAYER_FRAME_PCM      = 1, 
	JPLAYER_FRAME_YUV420P  = 2,
	JPLAYER_FRAME_YUV12    = 4,
	JPLAYER_FRAME_AUDIO    = 8,
	JPLAYER_FRAME_H264     = 16,
	JPLAYER_FRAME_NV12     = 32,
	JPLAYER_FRAME_NV21     = 64,
	JPLAYER_FRAME_HEVC     = 128,
	JPLAYER_FRAME_PIXELBUFFER     = 256,
	JPLAYER_FRAME_LENGTH
}JPlayer_FrameType;


typedef struct
{
    //video
    int  width;
    int  height;
    int  fps;
    const char *vdec_name;
    
    //audio
    const char *adec_name;
    int  channel;
    int  sample_rate;
    
    //general
    int bitrate;
}JPlayer_MediaInfo;


typedef struct 
{
	uint8_t* m_pData[3];       // 数据指针
	int   m_size[3];           // 数据长度
	int   m_width;             // 图像宽
	int   m_height;            // 图像高
	int   m_flag;              // 数据类型
	int64_t m_timestamp;      ///数据帧产生的时间，milliseconds 
	int   m_dts;
    uint64_t m_duration;
	uint64_t m_serialNum;
    int  m_sar_num;
    int  m_sar_den;
    int64_t m_host_time;
    int64_t m_host_stream_time;
} JPlayer_MediaFrame;

typedef enum
{
	DM_SCALE_IN = 0,            //默认  窗口内缩放
	DM_SCALE_OUT,
	DM_SCALE_FULL

}Display_Mode;

typedef enum
{
    JPLAYER_CP_AUDIOONLY = 0,
    JPLAYER_CP_VIDEOONLY,
    JPLAYER_CP_BOTH
}JPLAYER_STREAM_TYPE;
#endif

