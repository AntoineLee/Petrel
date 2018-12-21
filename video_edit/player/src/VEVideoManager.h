#ifndef __VIDEO_MANAGER_H__
#define __VIDEO_MANAGER_H__

#include <queue>
#include <mutex>
#include <list>
#include "pthread.h"
#include "stdint.h"
#include "FFmpegHdr.h"
#include "ve_interface.h"
#include "VEPlayerConfig.h"

#define MAX_FRAME_DURATION 1000
#define MAX_VQUEUE_NUM  50

class VEGraphManager;
typedef struct VideoFrame2
{
    uint8_t *pData{NULL};
    int size{0};
    int64_t timestamp{0};
    int64_t clipTimestamp{0};
    int width{0};
    int height{0};
    int rotate{0};
    int native{0};
    int test{0};
    int overlayId{-1};
    int clipId{-1};
    int trackId{-1};
    int transitionId{-1};
    int64_t startTime{-1};
    int64_t endTime{-1};
    
#ifdef __ANDROID__
    int textureId{-1};
    float* matrix{NULL};
#endif
}VideoFrame2;

class VideoFrameQueue
{
public:
	VideoFrameQueue(VEGraphManager *sinkg);
    bool getFrame(VideoFrame2 &frame);
    bool putFrame(const VideoFrame2 &frame,int force);
    bool insertFrameFront(VideoFrame2 &frame);
    bool popFrame();
    bool findFrame(int64_t ts,VideoFrame2 &frame);
    int size();
    void flush();
    bool isEmpty();
    
private:
    std::mutex m_mtx;
    std::list<VideoFrame2> m_queue;
    static const int MaxFrameNum = 5;
    VEGraphManager *m_sinkg;
};

class VEVideoManager
{
public:
    VEVideoManager();
    ~VEVideoManager();
    
    int open(VEGraphManager *gm);
    int close();
    
    bool writeFrame(VideoFrame2 frame,int index,int force = 0);
    int  step(int flag);
    void flush(int flushID = 360);
    void setDuration(int64_t dur);
    int addSource();
    int pasue();
    int play();
    int seekInternal(int64_t ts);
    int isAllQueueReady();
    int getOverlay(int64_t ts,int qId,VideoFrame2 &frame);
private:
    static void* worker(void *param);
    void thread();
private:
    VideoFrameQueue *m_videoFrameQ[MAX_VQUEUE_NUM];
    VEGraphManager *m_sinkg;
    
    int m_qSize;
    int m_seekStep;
    int m_state;
    int m_hasExit;
    int m_I420Size;
    int m_flushFlag;
    bool m_bStepNeeded;
    
    pthread_t m_tid;
    
    uint8_t *m_pI420Buffer;
    int64_t m_lastPts;
    int64_t m_duration;
    int64_t m_dropFramesUtil;
    int64_t m_frameTimer;
    int64_t m_lastFramePts;
    
    std::mutex m_mtx;
};




#endif
