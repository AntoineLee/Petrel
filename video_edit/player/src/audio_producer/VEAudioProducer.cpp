#include "VEAudioProducer.h"
#include "VEConfig.h"
#include "VEPlayerLogs.h"
#include "VEPlayerConfig.h"
#include "VESource.h"

AudioProducer::AudioProducer(HANDLE edit_config_param_handle, ve_audio_producer_callback callback,ve_audio_producer_status_callback preview_player_callback,void* userExt)
{
    m_veParam = (VEConfig *)edit_config_param_handle;
    m_pcmCallback = callback;
    m_statusCallback = preview_player_callback;
    m_userExt = userExt;
    
    m_pcmBufSize = 0;
    m_pcmBufPos = 0;
    m_pPCMBuffer = NULL;
    m_tid = 0;
    m_hasExit = 0;
    
    m_startTime = m_endTime = 0;
    m_volume = 100;
    m_speed = 1.0;
    m_pitch = 0;
    
    m_fmtCtx = NULL;
    m_codecCtx = NULL;
    m_afilter = NULL;
    m_psoundTouch = NULL;
    m_pSoundTouchBuffer = NULL;
    m_curClip = NULL;
    memset(m_lastFilter, 0, 255);
}

AudioProducer::~AudioProducer()
{
    m_hasExit = true;
    if(m_tid){
        pthread_join(m_tid, NULL);
        m_tid = NULL;
    }
    
    if(m_fmtCtx){
        avformat_close_input(&m_fmtCtx);
        m_fmtCtx = NULL;
    }
    
    if(m_codecCtx){
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = NULL;
    }
    
    if(m_pPCMBuffer){
        delete []m_pPCMBuffer;
        m_pPCMBuffer = NULL;
    }
    
    if(m_pSoundTouchBuffer){
        delete []m_pSoundTouchBuffer;
        m_pSoundTouchBuffer = NULL;
    }
    
    if(m_psoundTouch){
        delete m_psoundTouch;
        m_psoundTouch = NULL;
    }
    
    if(m_afilter){
        delete m_afilter;
        m_afilter = NULL;
    }
}

int AudioProducer::start(int idx, int cmd_id, int64_t start_time, int64_t end_time)
{
    if(findParameters(idx, cmd_id, start_time, end_time) < 0){
        reportStatus(AP_STATUS_FILE_OPEN_FAILED);
        PRODUCER_LOG_ERROR("start failed  \n");
        return -1;
    }
    
    m_pcmBufSize = (int)(((end_time - start_time) / 1000) * 4096 * 45 * 1.0 / m_speed) + 1024 * 1024;
    m_pPCMBuffer = new uint8_t[m_pcmBufSize];
    PRODUCER_LOG_INFO("pcmbuffer size = %d \n",m_pcmBufSize);
    
    if(0 != pthread_create(&m_tid, NULL, worker, (void *)this)){
        PRODUCER_LOG_ERROR("worker can not start! \n");
        return -1;
    }
    return 0;
}

void* AudioProducer::worker(void *param)
{
    AudioProducer *producer = (AudioProducer *)param;
    producer->producer();
    return NULL;
}

int AudioProducer::producer()
{
    m_fmtCtx = avformat_alloc_context();
    int ret = 0;
    int idx = 0;
    if(avformat_open_input(&m_fmtCtx, m_fileName, NULL, NULL) < 0){
        reportStatus(AP_STATUS_FILE_OPEN_FAILED);
        PRODUCER_LOG_ERROR("can not open file %s \n",m_fileName);
        return -1;
    }
    ret = avformat_find_stream_info(m_fmtCtx, NULL);
    idx = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if(idx < 0){
        reportStatus(AP_STATUS_FILE_NO_AUDIO);
        PRODUCER_LOG_ERROR("can not find best audio stream \n");
        return -1;
    }
    if(m_startTime != 0){
        int64_t seekTo = m_startTime * m_fmtCtx->streams[idx]->time_base.den / m_fmtCtx->streams[idx]->time_base.num / 1000;
        int ret = av_seek_frame(m_fmtCtx, idx,seekTo, AVSEEK_FLAG_BACKWARD);
    }
    
    //open codec
    m_codecCtx = avcodec_alloc_context3(NULL);
    avcodec_copy_context(m_codecCtx, m_fmtCtx->streams[idx]->codec);
    AVCodec *codec = avcodec_find_decoder(m_codecCtx->codec_id);
    if(avcodec_open2(m_codecCtx, codec, NULL) < 0){
        reportStatus(AP_STATUS_FILE_DECODE_FAILED);
        return -1;
    }
    
    while (!m_hasExit){
        AVPacket pkt;
        av_init_packet(&pkt);
        ret = av_read_frame(m_fmtCtx, &pkt);
        if(ret < 0){
            PRODUCER_LOG_INFO("av_read_frame return,but endTime is not reached \n");
            break;
        }
        if(pkt.stream_index != idx){
            av_free_packet(&pkt);
            continue;
        }
        int64_t pts = av_rescale_q(pkt.pts, m_fmtCtx->streams[pkt.stream_index]->time_base, AV_TIME_BASE_Q) / 1000;
        pkt.pts = pkt.dts = pts;
        if(pts >= m_startTime && pts <= m_endTime){
            AVFrame *frame = av_frame_alloc();
            int got = 0;
            ret = avcodec_decode_audio4(m_codecCtx, frame, &got, &pkt);
            if(ret && got){
                storeAudioFrame(frame);
            }
            av_frame_free(&frame);
        }
       
        av_free_packet(&pkt);
        if(pts >= m_endTime){
            PRODUCER_LOG_INFO("%s reached endTime \n");
            break;
        }
    }
    
    m_pcmCallback(this,m_pPCMBuffer,m_pcmBufPos,m_userExt);
    reportStatus(AP_STATUS_FILE_COMPLETE);
    return 0;
}

int AudioProducer::findParameters(int idx, int cmd_id, int64_t start_time, int64_t end_time)
{
    m_startTime = start_time;
    m_endTime = end_time;
    
    if(m_veParam){
        std::map<int,VETrackData>::iterator it2 = m_veParam->m_tracks.begin();
        for(it2;it2!=m_veParam->m_tracks.end();it2++){
            if(it2->second.m_track.track_id == idx){
                int i = 0;
                for(i;i<it2->second.m_clips.size();i++){
                    if(it2->second.m_clips[i].m_clip.clip_id == cmd_id){
                        strcpy(m_fileName,it2->second.m_clips[i].m_filename.c_str());
                        VEConfig::ajustVolume(m_veParam, &(it2->second), &(it2->second.m_clips[i].m_clip), &m_volume);
                        VEConfig::ajustSpeed(m_veParam, &(it2->second), &(it2->second.m_clips[i].m_clip), &m_speed);
                        m_pitch = it2->second.m_clips[i].m_clip.pitch;
                        m_curClip = &(it2->second.m_clips[i]);
                    }
                }
            }
        }
    }
    
    return m_curClip == NULL ? -1 : 0;
}

int AudioProducer::storeAudioFrame(AVFrame *frame)
{
    uint8_t *pTmp = NULL;
    int pcmLen = 0;
    char expr[256];
    
    float speedTmp = m_speed;
    if(m_curClip->m_clip.slv.active){
        int slv_index = 0;
        for(slv_index;slv_index<m_curClip->m_clip.slv.len;slv_index++){
            if(m_curClip->m_clip.slv.start_time[slv_index]<=frame->best_effort_timestamp && m_curClip->m_clip.slv.end_time[slv_index]>=frame->best_effort_timestamp){
                speedTmp = m_curClip->m_clip.slv.speed[slv_index] * speedTmp;
                break;
            }
        }
    }
    
    ve_audio_filter_param audio_param;
    audio_param.m_samplerateForSpeed = m_codecCtx->sample_rate * speedTmp;
    audio_param.m_volume = m_volume;
    
    ve_filter *fade_filter = findFadeFilter(frame->best_effort_timestamp);
    if(fade_filter){
        audio_param.m_fade = fade_filter->af_type;
        audio_param.m_gainMax = fade_filter->gain_max;
        audio_param.m_gainMin = fade_filter->gain_min;
        audio_param.m_nbSamples =  ((int64_t)(fade_filter->end_time - fade_filter->start_time)) * frame->sample_rate / 1000;
        audio_param.m_startSamples = ((int64_t)fade_filter->start_time) * frame->sample_rate / 1000;
    }

    VESource::getAudioFilterString(&audio_param,expr);
    if(strcmp(m_lastFilter, expr)){
        if(m_afilter)
            delete m_afilter;
        m_afilter = new VEAudioFilter();
        m_codecCtx->time_base.num = 1;
        m_codecCtx->time_base.den = 1000;
        m_afilter->addFilters(m_codecCtx,expr);
        strcpy(m_lastFilter, expr);
    }
    
    if(m_psoundTouch == NULL && m_pitch != 0){
        if(m_psoundTouch == NULL){
            m_psoundTouch = new VESoundTouch();
        }
        if(m_pSoundTouchBuffer == NULL){
            m_pSoundTouchBuffer = new uint8_t[1024*1024];
        }
        m_psoundTouch->configSoundTouchPitch(m_pitch, /*frame->channels*/AUDIO_RENDER_CHANNELS, AUDIO_RENDER_SAMPLERATE);
    }
    
    m_afilter->process(frame, (short **)&pTmp, &pcmLen);
    uint8_t *pPCMData = pTmp;
    int len = 0;
    len = pcmLen;
    if(m_pitch != 0){
        m_psoundTouch->processData(pTmp, pcmLen, m_pSoundTouchBuffer, &len);
        pPCMData = m_pSoundTouchBuffer;
    }
    memcpy(m_pPCMBuffer + m_pcmBufPos, pPCMData, len);
    m_pcmBufPos += len;
    if(m_pcmBufPos >= m_pcmBufSize){
        PRODUCER_LOG_ERROR("%s pos > size !!! \n");
    }
    
    return 0;
}

ve_filter * AudioProducer::findFadeFilter(int64_t ts)
{
    if(m_curClip && m_curClip->m_filters.size()){
        std::map<int,VEFilterData>::iterator it = m_curClip->m_filters.begin();
        for(it;it!=m_curClip->m_filters.end();it++){
            ve_filter &filter = it->second.m_filter;
            if(filter.type == VE_FILTER_AUDIO && (filter.start_time <= ts && filter.end_time >= ts)){
                if(filter.af_type == VE_AUDIO_FILTER_FADE_IN || filter.af_type == VE_AUDIO_FILTER_FADE_OUT){
                    return &filter;
                }
            }
        }
    }
    return NULL;
}

int AudioProducer::reportStatus(int status)
{
    if(m_statusCallback){
        m_statusCallback(this,status,m_userExt);
    }
    return 0;
}


