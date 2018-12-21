#include "VEAudioManager.h"
#include "VEPlayerConfig.h"
#include "VEPlayerLogs.h"
#include "VEPlayerUtils.h"
#include "VEGraphManager.h"

#include "FFmpegHdr.h"
#include "unistd.h"
#include <iterator>
#include "stdlib.h"

using namespace VEPlayerUtils;

bool AudioFrameQueue::putFrame(const AudioFrame2 &frame)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_queue.size() > MaxFrameNum)
        return false;
    m_queue.push(frame);
    return true;
}

bool AudioFrameQueue::getFrame(AudioFrame2 &frame)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_queue.size() == 0)
        return false;
    frame = m_queue.front();
    m_queue.pop();
    //frame.volume = m_volume;
    return true;
}

int AudioFrameQueue::size()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    return m_queue.size();
}

bool AudioFrameQueue::isEmpty()
{
    return size() > 0 ? false : true;
}

void AudioFrameQueue::flush()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    while (m_queue.size() > 0){
        AudioFrame2& packet = m_queue.front();
        ReleaseAudioFrame(&packet);
        m_queue.pop();
    }
}

VEAudioManager::VEAudioManager()
{
    m_qSize = 0;
    m_state = STATE_PAUSE;
    m_tid = 0;
    m_hasExit = 0;
    m_duration = 0;
    m_dropFramesUtil = 0;
}

VEAudioManager::~VEAudioManager()
{
    int idx = 0;
    for(idx ; idx < m_qSize ; idx++){
        delete  m_frameQueue[idx];
    }
}

int VEAudioManager::open(VEGraphManager *gm)
{
    m_sinkg = gm;
    m_audioRender.BwAudioCreate(AUDIO_RENDER_SAMPLERATE, AUDIO_RENDER_CHANNELS, 16);
    m_audioRender.BwAudioStart();
    
    if(0 != pthread_create(&m_tid, NULL, worker, (void *)this)){
        PREVIEW_LOG_ERROR("audio manager worker can not start! \n");
        return -1;
    }
    
    return 0;
}

int VEAudioManager::pasue()
{
    m_state = STATE_PAUSE;
    m_audioRender.BwAudioPause();
    return 0;
}

int VEAudioManager::play()
{
    m_state = STATE_PLAY;
    m_audioRender.BwAudioStart();
    return 0;
}


void* VEAudioManager::worker(void *param)
{
    VEAudioManager *pThis = (VEAudioManager *)param;
    if(pThis){
        pThis->thread();
    }
    return 0;
}

void VEAudioManager::thread()
{
    //JPlayerUtils::SetThreadHighPriority();
    while (!m_hasExit){
        if(m_qSize <= 0 || (m_state != STATE_PLAY && !m_dropFramesUtil)){
            usleep(30 *1000);
            continue;
        }
        
        AudioFrame2 dst;
        int idx = 0;
        if(m_frameQueue[idx]->getFrame(dst) == false){
            usleep(30 * 1000);
            continue;
        }
        
        if(m_dropFramesUtil){
            if(dst.timestamp < m_dropFramesUtil){
                PREVIEW_LOG_INFO("drop audio frame %lld \n",dst.timestamp);
            }else
                m_dropFramesUtil = 0;
            ReleaseAudioFrame(&dst);
            continue;
        }
        
        idx++;
        int mixed = 0;
        dst.volume = 100;
        for(idx ; idx < m_qSize ; idx++){
            AudioFrame2 src;
            if(1/*dst.timestamp >= m_frameQueue[idx]->m_startTime && dst.timestamp <= m_frameQueue[idx]->m_endTime*/){
                if(m_frameQueue[idx]->getFrame(src)){
                    mixed = 1;
                    mixer(dst, src);
                    ReleaseAudioFrame(&src);
                }
            }
        }
        
//        if(!mixed && dst.volume < 100){
//            AudioFrame2 src;
//            src.len = 0;
//            mixer(dst, src);
//        }
        
        int ret = m_audioRender.BwAudioWrite(dst.pData, dst.len);
        while (ret == 0 ){
            if(m_hasExit)
                break;
            usleep(15 * 1000);
            ret = m_audioRender.BwAudioWrite(NULL,0);
        }
        if(m_sinkg)
            m_sinkg->setClockTime(dst.timestamp);
        
        PREVIEW_LOG_DEBUG("now clock time is %lld \n",dst.timestamp);
        ReleaseAudioFrame(&dst);
        if(m_frameQueue[0]->isEmpty() && (m_duration - dst.timestamp) < 1000){
            m_sinkg->statusCallback(VE_PLAYER_PLAYBACK_COMPLETE);
            PREVIEW_LOG_INFO("audio detect completely \n");
        }
    }
}



inline int16_t RTC_ClampToInt16(int32_t input)
{
    if (input < -0x00008000) {
        return -0x8000;
    } else if (input > 0x00007FFF) {
        return 0x7FFF;
    } else {
        return static_cast<int16_t>(input);
    }
}

bool VEAudioManager::mixer(AudioFrame2 dst, AudioFrame2 src)
{
    int i = 0;
    int len = dst.len / 2;
    int16_t *pDst = (int16_t *)dst.pData;
    if(src.len){
        if(dst.len != src.len)
            return false;
        int16_t *pSrc = (int16_t *)src.pData;
        for(i;i<len;i++){
            int32_t wrap_guard =
            static_cast<int32_t>(*(pDst + i) * dst.volume / 100.0) + static_cast<int32_t>(*(pSrc + i) * src.volume / 100.0);
            *(pDst + i) = RTC_ClampToInt16(wrap_guard);
        }
        if(dst.volume != 100)
            dst.volume = 100;
    }else{
        for(i;i<len;i++){
            int32_t wrap_guard = static_cast<int32_t>(*(pDst + i) * dst.volume / 100.0);
            *(pDst + i) = RTC_ClampToInt16(wrap_guard);
        }
    }
    return true;
}

int VEAudioManager::close()
{
    m_audioRender.BwAudioSetVolume(0);
    m_hasExit = 1;
    if(m_tid){
        pthread_join(m_tid, NULL);
    }
    m_audioRender.BwAudioStop();
    m_audioRender.BwAudioDestroy();
    m_audioRender.BwAudioSetVolume(1);
    
    flush();
    
    return 0;
}

bool VEAudioManager::writeFrame(AudioFrame2 frame,int idx)
{
    if(idx >= m_qSize)
        return false;
    bool ret  = m_frameQueue[idx]->putFrame(frame);
    if(ret){
         PREVIEW_LOG_TRACE("write audio frame to queue pts = %lld volume = %d\n",frame.timestamp,frame.volume);
    }
    return ret;
}

int VEAudioManager::addSource()
{
    int idx = m_qSize;
    m_frameQueue[idx] = new AudioFrameQueue();
    m_qSize ++;
    return idx;
}

void VEAudioManager::flush()
{
    PREVIEW_LOG_INFO("AudioManager flush start \n");
    int i = 0;
    for(i;i<m_qSize;i++){
        m_frameQueue[i]->flush();
    }
    PREVIEW_LOG_INFO("AudioManager flush finish \n");
}

void VEAudioManager::setDuration(int64_t dur)
{
    m_duration = dur;
}

//void VEAudioManager::setQueueTime(int idx,int64_t start, int64_t end,int volume)
//{
//    m_frameQueue[idx]->m_startTime = start;
//    m_frameQueue[idx]->m_endTime = end;
//    m_frameQueue[idx]->m_volume = volume;
//}

int VEAudioManager::seekInternal(int64_t ts)
{
    m_dropFramesUtil = ts;
    return 0;
}


