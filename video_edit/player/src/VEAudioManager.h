#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__
#include <queue>
#include <mutex>
#include "stdint.h"
#include <pthread.h>
#include "VEAudioRender.h"

class VEGraphManager;

#define MAX_QUEUE_NUM  50
typedef struct
{
    uint8_t *pData{NULL};
    int len{0};
    int64_t timestamp{0};
    int64_t clipTimestamp{0};
    int volume{100};
    int clipId{-1};
    int trackId{-1};
    int transitionId{-1};
}AudioFrame2;

class AudioFrameQueue
{
public:
    AudioFrameQueue()
    {
        m_startTime = m_endTime = 0;
        m_volume = 100;
    }
    bool getFrame(AudioFrame2 &frame);
    bool putFrame(const AudioFrame2 &frame);
    int size();
    void flush();
    bool isEmpty();
public:
    int64_t m_startTime;//虚拟时间
    int64_t m_endTime;
    int     m_volume;
    
private:
    std::mutex m_mtx;
    std::queue<AudioFrame2> m_queue;
    static const int MaxFrameNum = 15;
};

class VEAudioManager
{
public:
    VEAudioManager();
    ~VEAudioManager();
    int open(VEGraphManager *gm);
    int close();
    bool writeFrame(AudioFrame2 frame,int idx);
    int addSource();
    int play();
    int pasue();
    void flush();
    void setDuration(int64_t dur);
    //void setQueueTime(int idx,int64_t start,int64_t end,int volume);
    
    int seekInternal(int64_t ts);
private:
    static void* worker(void *param);
    void thread();
    bool mixer(AudioFrame2 dst,AudioFrame2 src);
private:
    pthread_t m_tid;
    AudioFrameQueue *m_frameQueue[MAX_QUEUE_NUM];
    int m_qSize;
    AudioRender m_audioRender;
    int m_hasExit;
    VEGraphManager *m_sinkg;
    int m_state;
    int64_t m_duration;
    int64_t m_dropFramesUtil;
};



#endif
