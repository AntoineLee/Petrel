#ifndef __MEDIA_SOURCE_H__
#define __MEDIA_SOURCE_H__

#include "FFmpegHdr.h"
#include "VEConfig.h"
#include "pthread.h"
#include <list>
#include <queue>
#include <mutex>
#include "VEAudioManager.h"
#include "VEVideoManager.h"
#include "VEBitstream.h"
#include "VEAudioFilter.h"
#include "VESoundTouch.h"

#define MAX_STREAM_CNT          5
#define VE_HW_DECODER           1
#define MAX_SEG_CNT             50
#define MAX_URL_SIZE            4096
#define AUDIO_FRAME_BUFFER_SIZE 1 * 1024 * 1024

enum SOURCE_TYPE{
    SOURCE_TYPE_VIDEO_MAIN = 0,
    SOURCE_TYPE_VIDEO_OVERLAY,
    SOURCE_TYPE_AUDIO
};
typedef struct{
    AVPacket *packet;
    int segmentId;
    int64_t pts;
    int64_t dts;
    int64_t reuseUntilMs;
    //int64_t ptsMin;
}MediaPacket;

class MediaPacketQueue
{
public:
    MediaPacketQueue();
    ~MediaPacketQueue();
    
    bool getPacket(MediaPacket &pack);
    bool putPacketBack(const MediaPacket& packet,int force=0);
    bool putPacketFront(const MediaPacket& packet);
    int getFirstId();
    
    int size();
    void flush();
private:
    std::mutex m_mtx;
    std::list<MediaPacket> m_queue;
    static const std::size_t MaxPacketNum = 200;
};

struct segment
{
    int track_id{-1}; //可能同一个source里的segment来自不同参数配置的track
    int clip_id{-1};
    int transition_id{-1};
    int64_t duration{0};
    int64_t start_time{0}; //文件时间
    int64_t end_time{0};   //文件时间
    int64_t last_time{-1};  //区分是否是图片
    int64_t test_segment{0}; //补充转场traker中间空白
    int64_t offset{0};     //虚拟文件中此分片开始时间(ms)
    int     rotate{0};
    int64_t insert_start_time{0}; //播放时间
    int64_t insert_end_time{0};   //播放时间
    int     volume{100};
    float   speed{1.0};
    int     pitch{0};
    int64_t pts_min{0};
    int64_t pts_max{0};
    char url[MAX_URL_SIZE];
    AVFormatContext *ctx;
    AVCodecContext *ctx_audio;
    AVCodecContext *ctx_video;
    int video_idx{-1};
    int audio_idx{-1};
    int src_seq{-1};
    int64_t duration_audio{0};
    int64_t duration_video{0};
    //int fps{0};
    int videoCounter{0};
    slv_info slv;
};

class VEMediaSource
{
public:
    VEMediaSource();
    ~VEMediaSource();
    
    int  start();
    int  stop();
    void flush();
    int  seekTo(int64_t ts); //虚拟文件时间ms
    void setAudioSink(VEAudioManager *am);
    void setVideoSink(VEVideoManager *vm);
    void setGraphSink(VEGraphManager *gm);
    int  getSegId(int64_t ts);
    int  getSourceType();
    int  getVideoQueueId();
    int  addTrack(VEConfig *config,VETrackData *track,int duration,int &hasMain); //增加一条新track
    int  insertSegment(VEConfig *config,VETrackData *track,int index,int transition);//插入clip
    int  newSource(VEConfig *config,VETrackData *track,int index,int transitoin,int duration); //增加一条新track with clip(transition or overlay clip)
    void setHW(int usehw);
    int64_t calRealTimestamp(int64_t translatTs);
    struct segment* addTestSegment(int offset,int duration);
    bool hasOverlay(int64_t ts);
    
private:
    static void* reader_worker(void *param);
    static void* decv_worker(void *param);
    static void* deca_worker(void *param);
    void read_thread();
    void decv_thread();
    void deca_thread();
    
    void openCodec(AVCodecContext *ctx);
    void storeAudioFrame(AVFrame *frame,int vol,float speed,int pitch,int segId);
#ifdef __APPLE__
    uint8_t* storeVideoFrameNative(AVFrame *frame,int *width,int *height);
#else
    uint8_t* storeVideoFrame(AVFrame *frame,int *width,int *height,int *size);
#endif
    
    void changeVideoDecoder(AVCodecContext *ctx,int segId);
    int  getSegmentVolume(int type,int segmentid);
    float getFrameSpeed(int type,int segmentid,int64_t timestamp);
    int  addTestSegment(segment *seg,int time);
    int  addTestFrame(MediaPacket packet);
    int  addFlushPkt();
    ve_filter *findFadeFilter(int trackId,int clipId,int64_t ts);
    
    int getSegRealDuration(segment *seg);
    struct segment* addNewSegment(VEConfig *config,VETrackData *track,int index,int transition,int insert_time);
    struct segment* addNewTestSegment(int insert_time,int insert_end_time);
    
    int replaceSegment(segment *segL, segment *segM, segment *segH, int index);
private:
    int m_curSegmentId;
    int m_numSegments;
    struct segment* m_segments[MAX_SEG_CNT];
    
    AVFormatContext *m_curFmtCtx;
    AVCodecContext  *m_curVidCtx;
    AVCodecContext  *m_curAudCtx;
    VEBitstream     *m_curBsf;
    VEAudioFilter   *m_curAfVol;
    SwrContext      *m_swrCtx;
    SwsContext      *m_swsCtx;
    
    pthread_t m_tidReader;
    pthread_t m_tidDecV;
    pthread_t m_tidDecA;
    
    MediaPacketQueue m_videoQ;
    MediaPacketQueue m_audioQ;
    
    int m_hasExit;
    int m_seekTS;
    volatile int m_seekFlags;
    
    VEAudioManager *m_sinka;
    VEVideoManager *m_sinkv;
    VEGraphManager *m_sinkg;
    
    VESoundTouch *m_pSoundTouch;
    uint8_t *m_pSoundTouchBuffer;
    
    uint8_t *m_pSilentBuffer;
    int      m_lenSilentBuf;
    uint8_t *m_pAudioFrameBuffer;
    int      m_posAudioFrameBuffer;
    int      m_audioSrcId;  //for audio manager
    int      m_videoSrcId;  //for video manager
    int      m_type;
    int64_t  m_minVideoPts;
    char     m_lastAudioFilter[255];
    int      m_duration; //实际播放时长
    int      m_useHW;
    
    std::mutex m_mtx_video;
    std::mutex m_mtx_audio;
    
    std::mutex m_mtx_seek;

    VEVTMode m_vtMode;
};

#endif
