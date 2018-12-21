#include "VEConfig.h"
#include "VEGraphManager.h"
#include "VEPlayerConfig.h"
#include "FFmpegHdr.h"
#include "VEPlayerLogs.h"
#ifdef __APPLE__
#include<VideoToolbox/VideoToolbox.h>
#endif

#include <sys/time.h>
#include <time.h>
#include <queue>
#include "VEPlayerUtils.h"


int AVSync::setClockTime(int64_t ts)
{
    m_clockTime = ts;
    return 0;
}
int64_t AVSync::getClockTime()
{
    return m_clockTime;
}


VEGraphManager::VEGraphManager(HANDLE edit_config_param_handle,ve_player_callback callback,ve_player_status_callback preview_player_callback,void* userExt)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    av_register_all();
    m_paramHandle = edit_config_param_handle;
    m_veParam = *((VEConfig *)edit_config_param_handle);
    m_ve_data_callback = callback;
    m_ve_status_callback = preview_player_callback;
    m_userExt = userExt;
    m_isSeeking = 0;
    m_lastSeekTime = 0;
    m_pMainSource = NULL;
    m_lastSeekSegId = 0;
    m_lastSeekTimeStamp = 0;
    m_previewStatus = PV_STATUS_CLOSED;
    m_useHW = 1;
    open();
}


VEGraphManager::~VEGraphManager()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_pMainSource = NULL;
    m_previewStatus = PV_STATUS_CLOSING;
    stop();
    std::vector<VEMediaSource *>::iterator it = m_audioSourceManager.begin();
    for(; it != m_audioSourceManager.end();it++){
        delete *it;
    }
    
    std::vector<VEMediaSource *>::iterator it2 = m_videoSourceManager.begin();
    for(; it2 != m_videoSourceManager.end();it2++){
        delete *it2;
    }
    m_previewStatus = PV_STATUS_CLOSED;
}

int VEGraphManager::open()
{
    std::unique_lock<std::mutex> lock(m_mtxConfig);
    int duration = 0;
    int durationTranslate = 0;
    int ret = m_veParam.getTimelineDuration(1, &durationTranslate);
    if(ret != VE_ERR_OK || !m_veParam.m_tracks.size() || durationTranslate <= 0){
        statusCallback(VE_ERR_PREVIEW_PARAM);
        return -1;
    }
    m_previewStatus = PV_STATUS_LOADING;
    int hasMainTrack = 0;
    std::map<int,VETrackData>::iterator it = m_veParam.m_tracks.begin();
    for(it;it != m_veParam.m_tracks.end();it++){
        if(it->second.m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_SEQUENCE){
            VEMediaSource *source = new VEMediaSource();
            source->addTrack(&m_veParam, &(it->second), durationTranslate, hasMainTrack);
            if(it->second.m_track.type == VE_TRACK_VIDEO){
                m_videoSourceManager.push_back(source);
                if(m_pMainSource == NULL && hasMainTrack)
                    m_pMainSource = source;
            }else{
                m_audioSourceManager.push_back(source);
            }
            source->setAudioSink(&m_audioManager);
            source->setVideoSink(&m_videoManager);
            source->setGraphSink(this);
            if(it->second.m_transitions.size()){
                std::map<int,VETransitionData>::iterator tran_it = it->second.m_transitions.begin();
                for(tran_it;tran_it!=it->second.m_transitions.end();tran_it++){
                    insertCliptoSource(&m_veParam, &(it->second), tran_it->second.m_transition.clip_index_b, 1, durationTranslate);
                }
            }
        }else if(it->second.m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY){
            // insert clip to specified mediasource
            VETrackData *track = &(it->second);
            int clip_index = 0;
            for (clip_index;clip_index<track->m_clips.size();clip_index++){
                insertCliptoSource(&m_veParam, track, clip_index, 0, durationTranslate);
            }
        }
        else{
            PREVIEW_LOG_ERROR("%s param error \n",__FUNCTION__);
            statusCallback(VE_ERR_PREVIEW_PARAM);
        }
    }
    
    m_videoManager.open(this);
    m_videoManager.setDuration(durationTranslate);
    m_videoManager.step(0);
    m_audioManager.open(this);
    m_audioManager.setDuration(durationTranslate);
    m_duration = duration;
    m_previewStatus = PV_STATUS_LOADED;
    return 0;
}

int VEGraphManager::start()
{
    //没有状态判断，需要上层来保证状态
    std::unique_lock<std::mutex> lock(m_mtx);
    std::vector<VEMediaSource *>::iterator it = m_audioSourceManager.begin();
    for(; it != m_audioSourceManager.end();it++){
        (*it)->setHW(m_useHW);
        (*it)->start();
    }
    std::vector<VEMediaSource *>::iterator it2 = m_videoSourceManager.begin();
    for(; it2 != m_videoSourceManager.end();it2++){
        (*it2)->setHW(m_useHW);
        (*it2)->start();
    }
    m_previewStatus = PV_STATUS_PAUSE;
    return 0;
}

int VEGraphManager::stop()
{
    std::vector<VEMediaSource *>::iterator it = m_audioSourceManager.begin();
    for(; it != m_audioSourceManager.end();it++){
        (*it)->stop();
    }
    std::vector<VEMediaSource *>::iterator it2 = m_videoSourceManager.begin();
    for(; it2 != m_videoSourceManager.end();it2++){
        (*it2)->stop();
    }
    m_videoManager.close();
    m_audioManager.close();
    return 0;
}

int VEGraphManager::play()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_previewStatus != PV_STATUS_PAUSE){
        PREVIEW_LOG_WARN("do not need play as preview status wrong \n");
    }
    if(m_previewStatus == PV_STATUS_COMPLETE){ //预览完成后再次预览，直接返回预览完成
        if(m_ve_status_callback)
            m_ve_status_callback(this,VE_PLAYER_PLAYBACK_COMPLETE,m_userExt);
        return 0;
    }
    m_audioManager.play();
    m_videoManager.play();
    m_previewStatus = PV_STATUS_PLAY;
    return 0;
}

int VEGraphManager::pause()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_audioManager.pasue();
    m_videoManager.pasue();
    m_previewStatus = PV_STATUS_PAUSE;
    return 0;
}

int VEGraphManager::seekTo(int64_t ts,int force)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    PREVIEW_LOG_INFO("graph manager seekto %lld force = %d \n",ts,force);
    int segId = m_pMainSource->getSegId(ts);
    if((m_isSeeking && (VEPlayerUtils::GetCurrentTime() - m_lastSeekTime) < 500) && segId == m_lastSeekSegId && !force){
        PREVIEW_LOG_INFO("can not seek as seeking ignore %lld \n",ts);
        return 0;
    }
    m_isSeeking = 1;
    m_lastSeekTime = VEPlayerUtils::GetCurrentTime();
    m_lastSeekSegId = segId;
    int seekInternal = 1;
    if(m_lastSeekTimeStamp == 0){
        if(abs(m_avSync.getClockTime() - ts) > 500)
            seekInternal = 0;
    }
//#ifdef __APPLE__
    if((ts >= m_lastSeekTimeStamp) && ((ts - m_lastSeekTimeStamp) <= 300) && seekInternal && !force){
        PREVIEW_LOG_INFO("graph seekinternal ts = %lld last = %lld \n",ts,m_lastSeekTimeStamp);
        m_audioManager.seekInternal(ts);
        m_videoManager.seekInternal(ts);
        m_videoManager.step(1);
    }
    else
//#endif
    {
        PREVIEW_LOG_INFO("seek ts = %lld last = %lld \n",ts,m_lastSeekTimeStamp);
        m_audioManager.seekInternal(0);
        m_videoManager.seekInternal(0);
        std::vector<VEMediaSource *>::iterator it = m_audioSourceManager.begin();
        for(; it != m_audioSourceManager.end();it++){
            (*it)->seekTo(ts);
        }
        std::vector<VEMediaSource *>::iterator it2 = m_videoSourceManager.begin();
        for(; it2 != m_videoSourceManager.end();it2++){
            (*it2)->seekTo(ts);
        }
    }
    m_lastSeekTimeStamp = ts;

    return 0;
}

int VEGraphManager::setClockTime(int64_t ts)
{
    return m_avSync.setClockTime(ts);
}

int64_t VEGraphManager::getClockTime()
{
    return m_avSync.getClockTime();
}

ve_filter_callback_param callback_param;
int VEGraphManager::dataCallback(VideoFrame2 frame)
{
    std::unique_lock<std::mutex> lock(m_mtxConfig);
    if(m_ve_data_callback){
        memset(&callback_param, 0, sizeof(callback_param));
        std::map<int,int> trackIdxMap;
        std::queue<VideoFrame2> overlayFramesQueue;
        
        int trackNum = 0;
        if(!frame.test){
            ve_v_frame_callback_param frameParam = VEPlayerUtils::VideoFrame2ParamFrame(&m_veParam, frame);
            callback_param.multitracks[0].tracks[trackNum].frame_data[0] = frameParam;
            VEConfig::getTrackVFilters(&m_veParam, frame.trackId, frame.timestamp, callback_param.multitracks[0].tracks[trackNum]);
            VEConfig::getTimelineVFilters(&m_veParam, frame.timestamp, callback_param.multitracks[0]);
            trackIdxMap[trackNum] = frame.trackId;
            callback_param.multitracks[0].tracks[trackNum].track_id = frame.trackId;
            trackNum ++;
        }
        
        std::vector<VEMediaSource *>::iterator itSrc = m_videoSourceManager.begin();
        for(; itSrc != m_videoSourceManager.end();itSrc++){
            if((*itSrc)->getSourceType() == SOURCE_TYPE_VIDEO_OVERLAY){
                int qId = (*itSrc)->getVideoQueueId();
                VideoFrame2 frame2;
                int tryOverlayTimes = 0;
getoverlay:     if(m_videoManager.getOverlay(frame.timestamp, qId, frame2) == 0){
                    if((*itSrc)->hasOverlay(frame.timestamp)){
                        int bTransitionFrame = 0;
                        int i = 0;
                        for(i;i<trackNum;i++){
                            if(trackIdxMap[i] == frame2.trackId){
                                bTransitionFrame = 1;
                                break;
                            }
                        }
                        int track_index = trackNum;
                        if(bTransitionFrame){
                            track_index = i;
                        }
                        ve_v_frame_callback_param frameParam2 = VEPlayerUtils::VideoFrame2ParamFrame(&m_veParam, frame2);
                        if(frameParam2.transition_frame
                           && !frame.test) { // TODO temporary fix, need a better way.
                            int tmpTrackNum = 0;
                            if(trackNum > 0)
                                tmpTrackNum = trackNum - 1;
                            callback_param.multitracks[0].tracks[tmpTrackNum].frame_data[1] = frameParam2;
                        }else
                            callback_param.multitracks[0].tracks[trackNum].frame_data[0] = frameParam2;
                        
                        VEConfig::getTrackVFilters(&m_veParam, frame2.trackId, frame2.timestamp, callback_param.multitracks[0].tracks[trackNum]);
                        VEConfig::getTimelineVFilters(&m_veParam, frame2.timestamp, callback_param.multitracks[0]);
                        trackIdxMap[trackNum] = frame2.trackId;
                        callback_param.multitracks[0].tracks[trackNum].track_id = frame2.trackId;
                        overlayFramesQueue.push(frame2);
                        if(!bTransitionFrame)
                            trackNum ++;
                    }else{
                        VEPlayerUtils::ReleaseVideoFrame(&frame2);
                    }
                }else{
                    if((*itSrc)->hasOverlay(frame.timestamp) && tryOverlayTimes < 10){
                        PREVIEW_LOG_WARN("datacallback try get overlay frame(%lld) %d times \n",frame.timestamp,tryOverlayTimes);
                        usleep(20 * 1000);
                        tryOverlayTimes ++;
                        goto getoverlay;
                    }
                }
                    
            }
        }
        m_currentTime = frame.timestamp;
        callback_param.cur_time = frame.timestamp;
        callback_param.multitracks[0].tracks_num = trackNum;
        callback_param.timeline_id = m_veParam.m_id;
        int64_t datacallStart = VEPlayerUtils::GetCurrentTime();
        m_ve_data_callback(this,&callback_param,m_userExt);
        PREVIEW_LOG_TRACE("datacallback cost %lld ms frame timestamp = %lld tracknums = %d overlays = %d clipid = %d \n",VEPlayerUtils::GetCurrentTime() - datacallStart,frame.timestamp,trackNum,overlayFramesQueue.size(),frame.clipId);
        
        while (overlayFramesQueue.size() > 0){
            VideoFrame2& frame = overlayFramesQueue.front();
            VEPlayerUtils::ReleaseVideoFrame(&frame);
            overlayFramesQueue.pop();
        }
    }

    return 0;
}

int VEGraphManager::statusCallback(int status)
{
    if(status == VE_PLAYER_SEEK_COMPLETE){
        m_isSeeking = 0;
        m_lastSeekTime = 0;
    }
    if(status == VE_PLAYER_PLAYBACK_COMPLETE){
        m_previewStatus = PV_STATUS_COMPLETE;
    }
    if(m_ve_status_callback)
        m_ve_status_callback(this,status,m_userExt);
    
    return 0;
}

//int64_t VEGraphManager::getDuration()
//{
//    return m_duration;
//}

int64_t VEGraphManager::getCurrentTime()
{
    return m_currentTime;
}

int VEGraphManager::getSegVolume(int track_id, int clip_id)
{
    std::unique_lock<std::mutex> lock(m_mtxConfig);
    
    int vol = 100;
    if(&m_veParam){
        std::map<int,VETrackData>::iterator it = m_veParam.m_tracks.begin();
        for(it;it!=m_veParam.m_tracks.end();it++){
            if(it->second.m_track.track_id == track_id){
                int i = 0;
                for(i;i<it->second.m_clips.size();i++){
                    if(it->second.m_clips[i].m_clip.clip_id == clip_id){
                        VEConfig::ajustVolume(&m_veParam, &(it->second), &(it->second.m_clips[i].m_clip), &vol);
                    }
                }
            }
        }
    }
   
    return vol;
}


#ifdef __ANDROID__
void VEGraphManager::setRender(void* render)
{
    m_render = render;
}
void* VEGraphManager::getRender()
{
    return m_render;
}
#endif

int VEGraphManager::insertCliptoSource(VEConfig *config, VETrackData *track, int index, int transition, int duration)
{
    int i = 0;
    VEMediaSource *newSource = NULL;
    if(track->m_track.type == VE_TRACK_AUDIO){
        for(i;i<m_audioSourceManager.size();i++){
            VEMediaSource *source = m_audioSourceManager[i];
            if(source->insertSegment(config, track, index, transition) == 0){
                return 0;
            }
        }
        newSource = new VEMediaSource();
        newSource->newSource(config, track, index, transition, duration);
        m_audioSourceManager.push_back(newSource);
    }else if (track->m_track.type == VE_TRACK_VIDEO){
        for(i;i<m_videoSourceManager.size();i++){
            VEMediaSource *source = m_videoSourceManager[i];
            if(source->insertSegment(config, track, index, transition) == 0){
                return 0;
            }
        }
        newSource = new VEMediaSource();
        newSource->newSource(config, track, index, transition, duration);
        m_videoSourceManager.push_back(newSource);
    }
    
    if(newSource){
        newSource->setAudioSink(&m_audioManager);
        newSource->setVideoSink(&m_videoManager);
        newSource->setGraphSink(this);
    }
    return 0;
}

ve_filter *VEGraphManager::findFadeFilter(int track_id, int clip_id, int64_t ts)
{
    std::unique_lock<std::mutex> lock(m_mtxConfig);
    if(&m_veParam){
        std::map<int,VETrackData>::iterator it2 = m_veParam.m_tracks.begin();
        for(it2;it2!=m_veParam.m_tracks.end();it2++){
            if(it2->second.m_track.track_id == track_id){
                int i = 0;
                for(i;i<it2->second.m_clips.size();i++){
                    if(it2->second.m_clips[i].m_clip.clip_id == clip_id){
                        VEClipData *clip = &(it2->second.m_clips[i]);
                        std::map<int,VEFilterData>::iterator it = clip->m_filters.begin();
                        for(it;it!=clip->m_filters.end();it++){
                            ve_filter &filter = it->second.m_filter;
                            if(filter.type == VE_FILTER_AUDIO && (filter.start_time <= ts && filter.end_time >= ts)){
                                if(filter.af_type == VE_AUDIO_FILTER_FADE_IN || filter.af_type == VE_AUDIO_FILTER_FADE_OUT){
                                    return &filter;
                                }
                            }
                        }
                        
                    }
                }
            }
        }
    }
    return NULL;
}

void VEGraphManager::refreshHandle(HANDLE config)
{
    std::unique_lock<std::mutex> lock(m_mtxConfig);
    m_veParam = *((VEConfig *)config);
}

void VEGraphManager::setHW(int usehw)
{
    m_useHW = usehw;
}


