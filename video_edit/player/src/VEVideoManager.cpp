#include "VEVideoManager.h"
#include "VEPlayerLogs.h"
#include "VEPlayerUtils.h"
#include "VEGraphManager.h"
#include "libyuv.h"
#include "stdlib.h"
#include "unistd.h"
#ifdef __APPLE__
#include<VideoToolbox/VideoToolbox.h>
#endif

bool VideoFrameQueue::putFrame(const VideoFrame2 &frame,int force)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_queue.size() > MaxFrameNum && !force)
        return false;
    PREVIEW_LOG_INFO("video queue size = %d \n",m_queue.size());
    m_queue.push_back(frame);
    return true;
}

bool VideoFrameQueue::findFrame(int64_t ts,VideoFrame2 &frame)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_queue.size() == 0){
        PREVIEW_LOG_WARN("video frame queue no  frame \n");
        return false;
    }
    int count = 0;
    std::list<VideoFrame2>::iterator it = m_queue.begin();
    for(;it != m_queue.end();it++){
        if(it->timestamp <= ts)
            count ++;
        else
            break;
    }
    int i = 0;
    for(;i<(count-1);i++){
        frame = m_queue.front();
        m_queue.pop_front();
        VEPlayerUtils::ReleaseVideoFrame(&frame);
    }
    frame = m_queue.front();
    if(frame.test)
        return false;
    
    m_queue.pop_front();
    return true;
}

VideoFrameQueue::VideoFrameQueue(VEGraphManager *sinkg)
{
	m_sinkg = sinkg;
}
bool VideoFrameQueue::getFrame(VideoFrame2 &frame)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_queue.size() == 0)
        return false;
    frame = m_queue.front();
    m_queue.pop_front();
    return true;
}

int VideoFrameQueue::size()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    return m_queue.size();
}

bool VideoFrameQueue::isEmpty()
{
    return size() > 0 ? false : true;
}

void VideoFrameQueue::flush()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    
    while (m_queue.size() > 0){
        VideoFrame2 &frame = m_queue.front();
        VEPlayerUtils::ReleaseVideoFrame(&frame);
        
        m_queue.pop_front();
    }
}

bool VideoFrameQueue::insertFrameFront(VideoFrame2 &frame)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_queue.push_front(frame);
    return true;
}


VEVideoManager::VEVideoManager()
{
    m_bStepNeeded = 0;
    m_tid = NULL;
    m_hasExit = false;
    m_lastPts = 0;
    m_duration = 0;
    m_seekStep = 0;
    m_state = STATE_PAUSE;
    m_flushFlag = 0;
    m_sinkg = NULL;
    m_dropFramesUtil = 0;
    m_frameTimer = 0;
    m_lastFramePts = 0;
    m_qSize = 0;
    m_I420Size = 3 * 1024 * 1024;
    m_pI420Buffer = new uint8_t[m_I420Size];
}

VEVideoManager::~VEVideoManager()
{
    close();
    if(m_pI420Buffer){
        delete m_pI420Buffer;
        m_pI420Buffer = NULL;
    }
    int idx = 0;
    for(idx ; idx < m_qSize ; idx++){
        delete  m_videoFrameQ[idx];
    }
}

int VEVideoManager::pasue()
{
    m_state = STATE_PAUSE;
    m_frameTimer = 0;
    m_lastFramePts = 0;
    m_lastPts = 0;
    return 0;
}

int VEVideoManager::play()
{
    m_state = STATE_PLAY;
    return 0;
}

void VEVideoManager::setDuration(int64_t dur)
{
    m_duration = dur;
}

bool VEVideoManager::writeFrame(VideoFrame2 frame,int index,int force)
{
    bool ret = m_videoFrameQ[index]->putFrame(frame,force);
    if(ret){
        PREVIEW_LOG_TRACE("write video frame to queue pts = %lld \n",frame.timestamp);
    }
    return ret;
}

int VEVideoManager::step(int flag)
{
    m_bStepNeeded = 1;
    m_seekStep = flag;
    return 0;
}

int VEVideoManager::seekInternal(int64_t ts)
{
    m_dropFramesUtil = ts;
    m_lastPts = 0;
    return 0;
}

int VEVideoManager::open(VEGraphManager *gm)
{
    m_sinkg = gm;
    if(0 != pthread_create(&m_tid, NULL, worker, (void *)this)){
        PREVIEW_LOG_ERROR("videomanager thread can not start! \n");
        return -1;
    }
    
    return 0;
}

int VEVideoManager::close()
{
    m_hasExit = 1;
    if(m_tid){
        pthread_join(m_tid, NULL);
        m_tid = NULL;
    }
    m_sinkg = NULL;
    flush();
    return 0;
}

void* VEVideoManager::worker(void *param)
{
    VEVideoManager *pThis = (VEVideoManager *)param;
    if(pThis){
        pThis->thread();
    }
    return 0;
}

void VEVideoManager::thread()
{
    while (!m_hasExit){
        if(m_state != STATE_PLAY && !m_bStepNeeded){
            usleep(30 * 1000);
            continue;
        }
        
        std::unique_lock<std::mutex> lock(m_mtx);
        if(!isAllQueueReady()){
            lock.unlock();
            usleep(30 * 1000);
            continue;
        }
        
        int frameDuration = 0;
        VideoFrame2 frame;
        int idx = 0;
        if(!m_videoFrameQ[idx]->getFrame(frame)){
            lock.unlock();
            usleep(30 * 1000);
            continue;
        }
        idx = 1;
        int64_t curFramestamp = frame.timestamp;
        
        frameDuration = curFramestamp - m_lastFramePts;
        if(frameDuration < 0 || frameDuration > MAX_FRAME_DURATION)
            frameDuration = MAX_FRAME_DURATION;
        
        if(m_dropFramesUtil){
            if(frame.timestamp >= m_dropFramesUtil){
                m_dropFramesUtil = 0;
            }
        }
        
        int delay = 0;
        while (1 && !m_bStepNeeded && !m_flushFlag && !m_dropFramesUtil && m_frameTimer)
        {
            delay = frameDuration;
            if(m_hasExit)
                break;

            int diff ;
            diff = curFramestamp - m_sinkg->getClockTime();
            
            int sync_threshold = FFMAX(40, FFMIN(100, delay));
            if (!isnan(diff) && fabs(diff) <= 1000){
                if (diff <= -sync_threshold)
                    delay = FFMAX(0, delay + diff);
                else if (diff >= sync_threshold && delay > 100)
                    delay = delay + diff;
                else if (diff >= sync_threshold)
                    delay = 2 * delay;
            }
            delay = diff;
            
            if(VEPlayerUtils::GetCurrentTime() < m_frameTimer + delay){
                usleep(20 * 1000);
                continue;
            }else{
                PREVIEW_LOG_DEBUG("video refresh diff = %d delay = %d vpts = %lld apts = %lld duration = %d now = %lld frametimer = %lld",diff,delay,frame.timestamp,m_sinkg->getClockTime(),frameDuration,VEPlayerUtils::GetCurrentTime(),m_frameTimer);
                break;
            }
            
        }

        PREVIEW_LOG_DEBUG("now seek video frame time is %lld  queue size = %d \n",frame.timestamp,m_videoFrameQ[0]->size());
        m_lastFramePts = curFramestamp;
        if(m_sinkg && !m_flushFlag && !m_dropFramesUtil){
            if(m_state != STATE_PLAY){
                VideoFrame2 last = VEPlayerUtils::CopyVideoFrame(frame);
                m_videoFrameQ[0]->insertFrameFront(last);
            }
            
            if(m_lastPts && curFramestamp > m_lastPts && curFramestamp - m_lastPts <= 20){
                PREVIEW_LOG_WARN("drop video frame timestamp = %lld \n",curFramestamp);
            }else{
                m_lastPts = curFramestamp;
                m_sinkg->dataCallback(frame);
            }
            
            if(m_frameTimer == 0)
                m_frameTimer = VEPlayerUtils::GetCurrentTime();
            else
                m_frameTimer +=  delay;
            
            if (delay > 0 && VEPlayerUtils::GetCurrentTime() - m_frameTimer > 100)
                m_frameTimer = VEPlayerUtils::GetCurrentTime();
        }

        VEPlayerUtils::ReleaseVideoFrame(&frame);
        
        if(m_bStepNeeded && m_sinkg && !m_dropFramesUtil){
            m_bStepNeeded = 0;
            if(m_seekStep){
                m_sinkg->statusCallback(VE_PLAYER_SEEK_COMPLETE);
                PREVIEW_LOG_INFO("seekto complete \n");
            }
        }
    }
    return;
}

void VEVideoManager::flush(int flushID)
{
    PREVIEW_LOG_INFO("VideoManager flush start  %d \n",flushID);
    m_flushFlag = 1;
    std::unique_lock<std::mutex> lock(m_mtx);
    if(flushID == 360){
        int i = 0;
        for(i;i<m_qSize;i++){
            m_videoFrameQ[i]->flush();
        }
    }else{
        m_videoFrameQ[flushID]->flush();
    }
    
// #ifdef __ANDROID__
//     if(m_sinkg){
//         m_sinkg->statusCallback(VIDEO_EDIT_PREVIEW_FRAME_DROP);
//     }
// #endif

    m_flushFlag = 0;
    m_lastPts = 0;
    PREVIEW_LOG_INFO("VideoManager flush finish \n");
}


int VEVideoManager::addSource()
{
    int idx = m_qSize;
    m_videoFrameQ[idx] = new VideoFrameQueue(m_sinkg);
    m_qSize ++;
    return idx;
}

int VEVideoManager::isAllQueueReady()
{
    int i = 0;
    for(;i<m_qSize;i++){
        if(m_videoFrameQ[i]->size() == 0)
            break;
    }
    if(i >= m_qSize)
        return 1;
    else
        return 0;
}

int VEVideoManager::getOverlay(int64_t ts, int qId,VideoFrame2 &frame)
{
    if(qId >= m_qSize)
        return -1;
    
    if(m_videoFrameQ[qId]->findFrame(ts, frame)){
        //保存前一帧，seekinternal时，需要保持画中画视频帧不变
        if(ts >= frame.timestamp){
            VideoFrame2 dst = VEPlayerUtils::CopyVideoFrame(frame);
            m_videoFrameQ[qId]->insertFrameFront(dst);
        }
        return 0;
    }
    return -1;
}

//int VideoManager::getOverlays(ve_callback_param_overlay (&overlays)[MAX_VIDEO_FRAME_FILTER],int *len,int64_t ts)
//{
//    *len = 0;
//    if(m_qSize <= 1)
//        return 0;
//    int idx = 1;
//    for(;idx<m_qSize;idx++)
//    {
//        VideoFrame2 frame;
//        if(m_videoFrameQ[idx]->findFrame(ts, frame))
//        {
//            if(ts > frame.timestamp && (ts - frame.timestamp < 100))
//            {
//                VideoFrame2 dst = copyVideoFrame(frame);
//                m_videoFrameQ[idx]->insertFrameFront(dst);
//            }
//            overlays[*len].data[0] = frame.pData;
//            overlays[*len].width = frame.width;
//            overlays[*len].height = frame.height;
//            overlays[*len].len   = frame.size;
//            overlays[*len].rotate = frame.rotate;
//            overlays[*len].overlay_video_id = frame.overlayId;
//#ifdef __APPLE__
//            overlays[*len].format = VE_COLOR_IOS_PIXELBUFFER;
//#else
//            overlays[*len].format = VE_COLOR_YUV420P;
//#endif
//            *len = *len + 1;
//        }
//    }
//    return 0;
//}
//
//int VideoManager::getOverlay(ve_callback_param_overlay &overlay, int64_t ts,int q_id, int hasOverlay)
//{
//    if(q_id >= m_qSize)
//        return -1;
//    if(hasOverlay)
//    {
//        VideoFrame2 frame;
//        if(m_videoFrameQ[q_id]->findFrame(ts, frame))
//        {
//            //if(ts >= frame.timestamp && (ts - frame.timestamp < 100))
//            {
//                VideoFrame2 dst = copyVideoFrame(frame);
//                m_videoFrameQ[q_id]->insertFrameFront(dst);
//            }
//            overlay.data[0] = frame.pData;
//            overlay.width = frame.width;
//            overlay.height = frame.height;
//            overlay.len   = frame.size;
//            overlay.rotate = frame.rotate;
//            overlay.overlay_video_id = frame.overlayId;
//#ifdef __APPLE__
//            overlay.format = VE_COLOR_IOS_PIXELBUFFER;
//#else
//            overlay.format = frame.size > (2 * (frame.width * frame.height)) ? VE_COLOR_BGRA : VE_COLOR_YUV420P;
//#endif
//        }
//        else
//            return -1;
//    }
//    else
//    {
//        VideoFrame2 frame;
//        if(m_videoFrameQ[q_id]->findFrame(ts, frame))
//        {
//            if(frame.native == 0)
//            {
//                delete [] frame.pData;
//            }
//            else
//            {
//#ifdef __APPLE__
//                CVPixelBufferRelease((CVImageBufferRef)frame.pData);
//#endif
//            }
//        }
//    }
//    return 0;
//}
