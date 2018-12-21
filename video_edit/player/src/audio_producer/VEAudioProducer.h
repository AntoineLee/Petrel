#ifndef __VE_AUDIOPRODUCER_H_
#define __VE_AUDIOPRODUCER_H_
//#include "video_edit_entry.h"
#include "pthread.h"
#include "FFmpegHdr.h"
#include "VEAudioFilter.h"
#include "VESoundTouch.h"
#include <vector>
#include "VEConfig.h"
#include <iterator>


class VEConfig;
class AudioProducer
{
public:
    AudioProducer(HANDLE edit_config_param_handle, ve_audio_producer_callback callback,ve_audio_producer_status_callback preview_player_callback,void* userExt);
    ~AudioProducer();
    int start(int idx,int cmd_id,int64_t start_time,int64_t end_time);
private:
    static void* worker(void *param);
    int    findParameters(int idx,int cmd_id,int64_t start_time,int64_t end_time);
    int    producer();
    int    storeAudioFrame(AVFrame *frame);
    int    reportStatus(int status);
    ve_filter *findFadeFilter(int64_t ts);
private:
    ve_audio_producer_callback m_pcmCallback;
    ve_audio_producer_status_callback m_statusCallback;
    VEConfig *m_veParam;
    VEClipData *m_curClip;
    
    AVFormatContext *m_fmtCtx;
    AVCodecContext  *m_codecCtx;
    VEAudioFilter   *m_afilter;
    VESoundTouch *m_psoundTouch;
    uint8_t *m_pSoundTouchBuffer;
    
    int64_t m_startTime;
    int64_t m_endTime;
    int     m_volume;
    float   m_speed;
    int     m_pitch;
    char    m_fileName[255];
    char    m_lastFilter[255];
    
    uint8_t *m_pPCMBuffer;
    int      m_pcmBufSize;
    int      m_pcmBufPos;
    void    *m_userExt;
    bool     m_hasExit;
    
    pthread_t m_tid;
};

#endif

