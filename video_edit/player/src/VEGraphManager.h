#ifndef __GRAPH_MANAGER_H__
#define __GRAPH_MANAGER_H__
#include "stdint.h"
#include <vector>
#include <iterator>
#include "VEAudioManager.h"
#include "VEVideoManager.h"
#include "VEMediaSource.h"
#include <mutex>
#include <string>
#include "VEPlayerConfig.h"


class VEConfig;

class AVSync
{
public:
    AVSync(){
        m_clockTime = 0;
    }
    int setClockTime(int64_t ts);
    int64_t getClockTime();
private:
    int64_t m_clockTime;
    
};

enum{
    STATE_STOP = 0,
    STATE_PAUSE,
    STATE_PLAY
};



class VEGraphManager
{
public:
    VEGraphManager(HANDLE edit_config_param_handle,ve_player_callback callback,ve_player_status_callback preview_player_callback,void* userExt);
    ~VEGraphManager();
    
    int open();
    int stop();
    int start();
    
    int play();
    int pause();
    int seekTo(int64_t ts,int force);
    
    int setClockTime(int64_t ts);
    int64_t getClockTime();
    int dataCallback(VideoFrame2 frame);
    int statusCallback(int status);
    //int64_t getDuration();
    int64_t getCurrentTime();
    int getSegVolume(int track_id,int clip_id);
    ve_filter *findFadeFilter(int track_id,int clip_id,int64_t ts);
#ifdef __ANDROID__
    void setRender(void* render);
    void* getRender();
#endif
    void refreshHandle(HANDLE config);
    void setHW(int usehw);
    
private:
    int insertCliptoSource(VEConfig* config,VETrackData *track,int index,int transition,int duration);
    
private:
    AVSync m_avSync;
    std::vector<VEMediaSource *> m_videoSourceManager;
    std::vector<VEMediaSource *> m_audioSourceManager;
    VEAudioManager m_audioManager;
    VEVideoManager m_videoManager;
    VEConfig m_veParam;
    HANDLE m_paramHandle;
    ve_player_callback m_ve_data_callback;
    ve_player_status_callback m_ve_status_callback;
    void * m_userExt;
    int64_t m_duration;
    int64_t m_currentTime;
    std::mutex m_mtx;
    std::mutex m_mtxConfig;
    int m_isSeeking;
    int64_t m_lastSeekTime;
    VEMediaSource *m_pMainSource;
    int m_lastSeekSegId;
    int64_t m_lastSeekTimeStamp;
    std::string m_strCmd[MAX_VIDEO_FRAME_FILTER];
    int m_previewStatus;
    int m_useHW;
#ifdef __ANDROID__
    void* m_render{NULL};
#endif
};

#endif
