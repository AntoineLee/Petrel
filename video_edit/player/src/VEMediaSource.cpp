#include "VEMediaSource.h"
#include "VEPlayerLogs.h"
#include "string.h"
#include "stdio.h"
#include "VEPlayerConfig.h"
#include "libyuv/scale.h"
#include "ios+ver.h"
#include "VEPlayerUtils.h"
#include "VEGraphManager.h"
#ifdef __APPLE__
#include <VideoToolbox/VideoToolbox.h>
#endif
#include "VESource.h"
#include "string.h"

using namespace VEPlayerUtils;

#ifdef __APPLE__
extern "C" AVCodec h264_videotoolbox_decoder ;
extern "C" AVCodec hevc_videotoolbox_decoder ;
#endif

#ifdef __ANDROID__
#define INT64_MAX        9223372036854775807LL
extern "C" AVCodec ff_h264_mediacodec_decoder;

//#include "mediacodec2.h"
//extern "C" AVCodec ff_h264_mediacodec_async_decoder;
//extern "C" void mediacodec2_get_frame_data(void* data, int* outTex, float** outMatrix);
#endif

extern "C" AVCodec ff_libvpx_vp8_decoder;
extern "C" AVCodec ff_libvpx_vp9_decoder;

MediaPacketQueue::MediaPacketQueue()
{
    m_queue.clear();
}

MediaPacketQueue::~MediaPacketQueue()
{
    flush();
}

bool MediaPacketQueue::getPacket(MediaPacket &pack)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_queue.size() == 0)
        return false;
    pack = m_queue.front();
    m_queue.pop_front();
    return true;
}

bool MediaPacketQueue::putPacketBack(const MediaPacket &packet,int force) //read线程做边界检查
{
    if(m_queue.size() > MaxPacketNum && !force)
        return false;
    std::unique_lock<std::mutex> lock(m_mtx);
    m_queue.push_back(packet);
    return true;
}

bool MediaPacketQueue::putPacketFront(const MediaPacket &packet)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    
    m_queue.push_front(packet);
    return true;
}

int MediaPacketQueue::getFirstId()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_queue.size())
        return m_queue.front().segmentId;
    return -1;
}

int MediaPacketQueue::size()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    return m_queue.size();
}

void MediaPacketQueue::flush()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    std::list<MediaPacket>::iterator it = m_queue.begin();
    for(;it != m_queue.end();it ++){
        av_packet_free(&it->packet);
    }
    m_queue.clear();
    return;
}



VEMediaSource::VEMediaSource()
{
    PREVIEW_LOG_INFO("build a souce for video edit %d \n",this);
    m_numSegments = 0;
    m_curSegmentId = 0;
    m_curFmtCtx = NULL;
    m_curVidCtx = NULL;
    m_curAudCtx = NULL;
    m_sinka = NULL;
    m_sinkv = NULL;
    m_hasExit = 0;
    m_seekFlags = m_seekTS = 0;
    m_lenSilentBuf = 1024 * 2 * AUDIO_RENDER_CHANNELS;
    m_pSilentBuffer = new uint8_t[m_lenSilentBuf];
    memset(m_pSilentBuffer, 0, m_lenSilentBuf);
    m_pAudioFrameBuffer = new uint8_t[AUDIO_FRAME_BUFFER_SIZE];
    m_posAudioFrameBuffer = 0;
    m_audioSrcId = 0;
    m_videoSrcId = 0;
    m_tidReader = m_tidDecA = m_tidDecV = 0;
    m_swrCtx = NULL;
    m_swsCtx = NULL;
    m_curBsf = NULL;
    m_curAfVol  = NULL;
    m_pSoundTouch = NULL;
    m_minVideoPts = -1;
    m_pSoundTouchBuffer = NULL;
    m_duration = 0;
    m_useHW = 1;
    memset(m_lastAudioFilter, 0, 255);
    m_vtMode.m_mode = VE_VT_MODE_PLAYER;
}

VEMediaSource::~VEMediaSource()
{
    stop();
    if(m_pSilentBuffer){
        delete m_pSilentBuffer;
        m_pSilentBuffer = NULL;
    }
    if(m_pAudioFrameBuffer){
        delete m_pAudioFrameBuffer;
        m_pAudioFrameBuffer = NULL;
    }
    
    if(m_curAudCtx){
        avcodec_free_context(&m_curAudCtx);
        m_curAudCtx = NULL;
    }
    
    if(m_curVidCtx){
        avcodec_free_context(&m_curVidCtx);
        m_curVidCtx = NULL;
    }
    
    if(m_curBsf){
        delete m_curBsf;
        m_curBsf = NULL;
    }
    
    if(m_curAfVol){
        delete m_curAfVol;
        m_curAfVol = NULL;
    }
    
    if(m_pSoundTouch){
        delete m_pSoundTouch;
        m_pSoundTouch = NULL;
    }
    
    if(m_pSoundTouchBuffer){
        delete m_pSoundTouchBuffer;
        m_pSoundTouchBuffer = NULL;
    }
    //释放segment list
    int i;
    for (i = 0; i < m_numSegments; i++){
        if(m_segments[i]->ctx){
            avformat_close_input(&m_segments[i]->ctx);
        }
        if(m_segments[i]->ctx_audio){
            avcodec_free_context(&m_segments[i]->ctx_audio);
        }
        if(m_segments[i]->ctx_video){
            avcodec_free_context(&m_segments[i]->ctx_video);
        }
        delete m_segments[i];
    }
    
    m_numSegments = 0;
    PREVIEW_LOG_INFO("free a souce for video edit %d \n",this);
}

int VEMediaSource::addTrack(VEConfig *config,VETrackData *track, int duration, int &hasMain)
{
    int clip_index = 0;
    
    for(clip_index;clip_index < track->m_clips.size();clip_index++){
        segment *seg = addNewSegment(config, track, clip_index, 0, m_duration);
        m_segments[m_numSegments] = seg;
        m_numSegments ++;
        m_duration += getSegRealDuration(seg);
    }
    
    //add test segment
    if(m_duration < duration){
        segment *seg = addTestSegment(m_duration, duration - m_duration);
        m_segments[m_numSegments] = seg;
        m_duration = duration;
        m_numSegments ++;
    }
    
    if(track->m_track.type == VE_TRACK_VIDEO && track->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_SEQUENCE && !hasMain){
        m_type = SOURCE_TYPE_VIDEO_MAIN;
        hasMain = 1;
    }
    else if (track->m_track.type == VE_TRACK_AUDIO){
        m_type = SOURCE_TYPE_AUDIO;
    }
    else{
        m_type = SOURCE_TYPE_VIDEO_OVERLAY;
    }

    return 0;
}


void VEMediaSource::setAudioSink(VEAudioManager *am)
{
    m_sinka = am;
    m_audioSrcId = m_sinka->addSource();
    //m_sinka->setQueueTime(m_audioSrcId, m_segments[0]->insert_start_time,m_segments[0]->insert_end_time,m_segments[0]->volume);
}

void VEMediaSource::setGraphSink(VEGraphManager *gm)
{
    m_sinkg = gm;
}

void VEMediaSource::setVideoSink(VEVideoManager *vm)
{
    m_sinkv = vm;
    if(m_type <= SOURCE_TYPE_VIDEO_OVERLAY)
        m_videoSrcId = m_sinkv->addSource();
}

int VEMediaSource::start()
{
    if(m_numSegments <= 0){
        PREVIEW_LOG_ERROR("mediasource can not start!");
        return -1;
    }
    
    if(0 != pthread_create(&m_tidReader, NULL, reader_worker, (void *)this)){
        PREVIEW_LOG_ERROR("mediasource reader_worker can not start! \n");
        return -1;
    }
    
    if(0 != pthread_create(&m_tidDecV, NULL, decv_worker, (void *)this)){
        PREVIEW_LOG_ERROR("mediasource decv_worker can not start! \n");
        return -1;
    }
    
    if(0 != pthread_create(&m_tidDecA, NULL, deca_worker, (void *)this)){
        PREVIEW_LOG_ERROR("mediasource deca_worker can not start! \n");
        return -1;
    }
    
    return 0;
}

int VEMediaSource::stop()
{
    m_hasExit = 1;
    
    if(m_tidReader){
        pthread_join(m_tidReader, NULL);
        m_tidReader = NULL;
    }
    
    if(m_tidDecV){
        pthread_join(m_tidDecV, NULL);
        m_tidDecV = NULL;
    }
    
    if(m_tidDecA){
        pthread_join(m_tidDecA, NULL);
        m_tidDecA = NULL;
    }
    PREVIEW_LOG_INFO("mediasource has stopped \n");
    
    return 0;
}

void* VEMediaSource::reader_worker(void *param)
{
    VEMediaSource *pThis = (VEMediaSource *)param;
    if(pThis){
        pThis->read_thread();
    }
    return 0;
}

void* VEMediaSource::decv_worker(void *param)
{
    VEMediaSource *pThis = (VEMediaSource *)param;
    if(pThis){
        pThis->decv_thread();
    }
    return 0;
}

void* VEMediaSource::deca_worker(void *param)
{
    VEMediaSource *pThis = (VEMediaSource *)param;
    if(pThis){
        pThis->deca_thread();
    }
    return 0;
}

int VEMediaSource::seekTo(int64_t ts)
{
	PREVIEW_LOG_INFO("mediasource seekto %lld  begin\n",ts);
    
    std::unique_lock<std::mutex> lock(m_mtx_seek);
    m_seekFlags = 1;
    if(ts < 0) ts = 0;
    m_seekTS =  ts;

    PREVIEW_LOG_INFO("mediasource seekto %lld end\n",ts);
    return 0;
}

void VEMediaSource::read_thread()
{
    int fileShouldSeek = 0;
    int fileAudioEnd = 0;
    int fileVideoEnd = 0;
    int fileAudioDuration = 0;
    int fileVideoDuration = 0;
    
    while (!m_hasExit){
        if(m_curSegmentId >= m_numSegments && !m_seekFlags){
            usleep(20 * 1000);
            continue;
        }
        
        {
            std::unique_lock<std::mutex> lock(m_mtx_seek);
            if(m_seekFlags){
                PREVIEW_LOG_INFO("mediasource read_thread seek start %lld \n",m_seekTS);
                int idx = m_numSegments - 1;
                int seekSegIdx = -1;
                fileAudioDuration = fileVideoDuration = 0;
                
                for(idx;idx>=0;idx--){
                    if(m_segments[idx]->insert_start_time <= m_seekTS){
                        seekSegIdx = idx;
                        break;
                    }
                }

                m_curFmtCtx = NULL;
                m_curSegmentId = seekSegIdx;
                fileAudioDuration = m_segments[m_curSegmentId]->duration_audio;
                fileVideoDuration = m_segments[m_curSegmentId]->duration_video;
                if(m_segments[seekSegIdx]->ctx && m_segments[seekSegIdx]->last_time <= 0 && !m_segments[seekSegIdx]->test_segment){
                    m_curFmtCtx = m_segments[seekSegIdx]->ctx;
                    m_curFmtCtx->pb->eof_reached = 0;
                    int ret = av_seek_frame(m_curFmtCtx, -1,0, AVSEEK_FLAG_BACKWARD);
                    
                    if(m_segments[m_curSegmentId]->video_idx < 0) fileVideoEnd = 1;
                    else fileVideoEnd = 0;
                    
                    if(m_segments[m_curSegmentId]->audio_idx < 0){
                        fileAudioEnd = 1;
                        m_segments[m_curSegmentId]->audio_idx = -1;
                    }else
                        fileAudioEnd = 0;
                    PREVIEW_LOG_INFO("seek a old file  \n");
                }
                
            }
            
            if(m_curFmtCtx == NULL && !m_segments[m_curSegmentId]->test_segment){
                PREVIEW_LOG_INFO("open video start index = %d %s \n",m_curSegmentId,m_segments[m_curSegmentId]->url);
                if(m_segments[m_curSegmentId]->ctx){
                    avformat_close_input(&m_segments[m_curSegmentId]->ctx);
                    m_segments[m_curSegmentId]->ctx = NULL;
                }
                m_curFmtCtx = avformat_alloc_context();
                AVDictionary *dict = NULL;
                av_dict_set_int(&dict, "reconnect", 1, 0);
                av_dict_set_int(&dict, "timeout", 3000000, 0);
                int ret = avformat_open_input(&m_curFmtCtx, m_segments[m_curSegmentId]->url, NULL, &dict);
                av_dict_free(&dict);
                if(ret < 0){
                    PREVIEW_LOG_ERROR("read_thread can not open %s \n",m_segments[m_curSegmentId]->url);
                    return ;
                }
                
                ret = avformat_find_stream_info(m_curFmtCtx, NULL);
                if(ret < 0){
                    PREVIEW_LOG_ERROR("read_thread %s find stream info failed \n",m_segments[m_curSegmentId]->url);
                    return;
                }
                
                if(m_type <= SOURCE_TYPE_VIDEO_OVERLAY)
                    m_segments[m_curSegmentId]->video_idx = av_find_best_stream(m_curFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
                m_segments[m_curSegmentId]->audio_idx = av_find_best_stream(m_curFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
                if(m_segments[m_curSegmentId]->start_time > 0){
                    fileShouldSeek = 1;
                }
                
                m_segments[m_curSegmentId]->ctx = m_curFmtCtx;
                if(m_segments[m_curSegmentId]->video_idx >= 0 && m_segments[m_curSegmentId]->ctx_video == NULL){
                    m_segments[m_curSegmentId]->ctx_video = avcodec_alloc_context3(NULL);
                    avcodec_copy_context(m_segments[m_curSegmentId]->ctx_video, m_curFmtCtx->streams[m_segments[m_curSegmentId]->video_idx]->codec);
                }
                if(m_segments[m_curSegmentId]->audio_idx >= 0 && m_segments[m_curSegmentId]->ctx_audio == NULL){
                    m_segments[m_curSegmentId]->ctx_audio = avcodec_alloc_context3(NULL);
                    avcodec_copy_context(m_segments[m_curSegmentId]->ctx_audio, m_curFmtCtx->streams[m_segments[m_curSegmentId]->audio_idx]->codec);
                }
                if(m_segments[m_curSegmentId]->video_idx < 0){
                    fileVideoEnd = 1;
                }else{
                    fileVideoEnd = 0;
                    m_segments[m_curSegmentId]->duration_video = av_rescale_q(m_curFmtCtx->streams[m_segments[m_curSegmentId]->video_idx]->duration, m_curFmtCtx->streams[m_segments[m_curSegmentId]->video_idx]->time_base, AV_TIME_BASE_Q) / 1000;
                    fileVideoDuration = m_segments[m_curSegmentId]->duration_video;
                }
                
                if(m_segments[m_curSegmentId]->audio_idx < 0){
                    fileAudioEnd = 1;
                }else{
                    fileAudioEnd = 0;
                    m_segments[m_curSegmentId]->duration_audio = av_rescale_q(m_curFmtCtx->streams[m_segments[m_curSegmentId]->audio_idx]->duration, m_curFmtCtx->streams[m_segments[m_curSegmentId]->audio_idx]->time_base, AV_TIME_BASE_Q) / 1000;
                    fileAudioDuration = m_segments[m_curSegmentId]->duration_audio;
                }
                
                m_segments[m_curSegmentId]->pts_min = m_segments[m_curSegmentId]->insert_start_time;
                m_segments[m_curSegmentId]->pts_max = m_segments[m_curSegmentId]->insert_end_time - 1;
                PREVIEW_LOG_INFO("open video finish %s videoduration = %lld audioduration = %lld \n",m_segments[m_curSegmentId]->url,m_segments[m_curSegmentId]->duration_video,m_segments[m_curSegmentId]->duration_audio);
            }
            
            
            if(m_seekFlags || fileShouldSeek){
                PREVIEW_LOG_INFO("mediasource video seek start %s \n",m_segments[m_curSegmentId]->url);
                if(m_seekFlags){
                    flush();
                }
                int64_t seekTo = m_segments[m_curSegmentId]->start_time;
                if(m_seekFlags){
                    seekTo = GetSegOriginTime(m_segments[m_curSegmentId], m_seekTS);
                }else{
                    m_seekTS = m_segments[m_curSegmentId]->insert_start_time;
                }
                
                m_segments[m_curSegmentId]->pts_min = m_seekTS;
                seekTo = (seekTo /*+ m_segments[m_curSegmentId]->start_time*/) * 1000;
                if(m_segments[m_curSegmentId]->last_time < 0 && !m_segments[m_curSegmentId]->test_segment){
                    int ret = av_seek_frame(m_curFmtCtx, -1,seekTo, AVSEEK_FLAG_BACKWARD);
                    if(ret < 0){
                        PREVIEW_LOG_ERROR("file seek failed");
                        //return ;
                    }
                }
                
                if(m_type == SOURCE_TYPE_VIDEO_MAIN && m_seekFlags) //主视频step,叠加视频跟着走
                    m_sinkv->step(1);
                m_seekFlags = 0;
                fileShouldSeek = 0;
                PREVIEW_LOG_INFO("mediasource seek finish  \n");
            }
        }

       
        AVPacket *packet = av_packet_alloc();
        int ret = 0;
        if(m_segments[m_curSegmentId]->test_segment){
            addTestSegment(m_segments[m_curSegmentId],0);
            ret = -1;
        }else{
            ret = av_read_frame(m_curFmtCtx,packet);
        }
        
        if(ret < 0){
            bool isNetFile = 0;
            if(m_curFmtCtx && m_curFmtCtx->filename)
                isNetFile = strstr(m_curFmtCtx->filename, "http") ? 1 : 0;
            
            if(ret != AVERROR_EOF && isNetFile)
                m_sinkg->statusCallback(VE_ERR_PREVIEW_CONNECT_FAILED);
            
            goto segment_finish;
        }
        else
        {
            int64_t segment_pts = av_rescale_q(packet->pts, m_curFmtCtx->streams[packet->stream_index]->time_base, AV_TIME_BASE_Q) / 1000;
            int64_t segment_dts = av_rescale_q(packet->dts, m_curFmtCtx->streams[packet->stream_index]->time_base, AV_TIME_BASE_Q) /1000;
            int64_t virtual_pts = m_segments[m_curSegmentId]->insert_start_time + GetSegRealTime(m_segments[m_curSegmentId], segment_pts - m_segments[m_curSegmentId]->start_time) ;
            int64_t virtual_dts = m_segments[m_curSegmentId]->insert_start_time + GetSegRealTime(m_segments[m_curSegmentId], segment_dts - m_segments[m_curSegmentId]->start_time) ;
            int64_t packet_duration = 0;
            if(packet->duration > 0)
                packet_duration = av_rescale_q(packet->duration,m_curFmtCtx->streams[packet->stream_index]->time_base, AV_TIME_BASE_Q) / 1000;
            
            if(m_segments[m_curSegmentId]->last_time > 0){
                virtual_pts = m_segments[m_curSegmentId]->pts_min;
                virtual_dts = m_segments[m_curSegmentId]->pts_min;
            }
            packet->pts = virtual_pts;
            packet->dts = virtual_dts;
            
            if(!fileVideoEnd || !fileAudioEnd){
                MediaPacket pkt;
                pkt.packet = packet;
                pkt.segmentId = m_curSegmentId;
                pkt.reuseUntilMs = 0;
                pkt.pts = pkt.dts = virtual_pts;
                if(virtual_pts >= m_segments[m_curSegmentId]->insert_end_time && m_segments[m_curSegmentId]->last_time <= 0){
                    if(packet->stream_index == m_segments[m_curSegmentId]->video_idx && m_segments[m_curSegmentId]->videoCounter)
                        fileVideoEnd = 1;
                    if(packet->stream_index == m_segments[m_curSegmentId]->audio_idx)
                        fileAudioEnd = 1;
                }
                if(packet->stream_index == m_segments[m_curSegmentId]->video_idx && !fileVideoEnd){
                    m_segments[m_curSegmentId]->videoCounter = 1;
                    PREVIEW_LOG_TRACE("segmentid = %d coming video packet stream_idx = %d pts = %lld dts = %lld segment pts = %lld size = %d isKey = %d \n",m_curSegmentId,packet->stream_index,packet->pts,packet->dts,segment_pts,packet->size,packet->flags);
                    if(m_segments[m_curSegmentId]->last_time > 0 || (m_segments[m_curSegmentId]->audio_idx < 0 && m_segments[m_curSegmentId]->audio_idx != -999) || (fileVideoDuration  && fileAudioDuration && (fileVideoDuration - fileAudioDuration > 2000) && segment_pts > fileAudioDuration)){
                        
                        pkt.packet = NULL;
                        pkt.reuseUntilMs = m_segments[m_curSegmentId]->insert_end_time - 1;
                        if(pkt.reuseUntilMs < pkt.pts)
                            pkt.pts = pkt.reuseUntilMs;
                        
                        PREVIEW_LOG_INFO("put a slient audio packet reusetime = %lld pkt.pts = %lld id = %d pts_min=%lld \n",pkt.reuseUntilMs,pkt.pts,pkt.segmentId,m_segments[pkt.segmentId]->pts_min);
                        
                        BEGIN_LOOP_QUEUE_PACKET(m_audioQ, pkt)
                        {
                            if(m_videoQ.size() == 0 && m_type < SOURCE_TYPE_AUDIO){
                                m_audioQ.putPacketBack(pkt,1);
                                break;
                            }
                        }
                        END_LOOP_QUEUE_PACKET(m_hasExit, m_seekFlags, pkt)
                        
                        pkt.packet = packet;
                        
                        if(m_segments[m_curSegmentId]->last_time > 0){
                            fileVideoEnd = 1;
                        }else if(m_segments[m_curSegmentId]->audio_idx < 0){
                            m_segments[m_curSegmentId]->audio_idx = -999;
                            if(packet_duration < 1000) pkt.reuseUntilMs = 0;
                        }else if ((fileVideoDuration  && fileAudioDuration && (fileVideoDuration - fileAudioDuration > 2000) && segment_pts > fileAudioDuration)){
                            fileAudioEnd = 1;
                            fileAudioDuration = 0;
                            pkt.reuseUntilMs = 0;
                        }
                    }
                    
                    BEGIN_LOOP_QUEUE_PACKET(m_videoQ, pkt)
                    {
                        if(m_segments[pkt.segmentId]->audio_idx >= 0 && m_audioQ.size() == 0 && m_type < SOURCE_TYPE_AUDIO){
                            m_videoQ.putPacketBack(pkt,1);
                            break;
                        }
                    }
                    END_LOOP_QUEUE_PACKET(m_hasExit, m_seekFlags, pkt)
                    
                }else if(packet->stream_index == m_segments[m_curSegmentId]->audio_idx && !fileAudioEnd){
                    PREVIEW_LOG_TRACE("segmentid = %d coming audio packet stream_idx = %d pts = %lld dts = %lld segment pts = %lld size = %d \n",m_curSegmentId,packet->stream_index,packet->pts,packet->dts,segment_pts,packet->size);
                    
                    BEGIN_LOOP_QUEUE_PACKET(m_audioQ, pkt)
                    {
                        if(m_videoQ.size() == 0 && m_type < SOURCE_TYPE_AUDIO){
                            m_audioQ.putPacketBack(pkt,1);
                            break;
                        }
                    }
                    END_LOOP_QUEUE_PACKET(m_hasExit, m_seekFlags, pkt)
                }else{
                    av_packet_free(&packet);
                }
            }else{
                goto segment_finish;
            }
            continue;
        }
        
segment_finish:
        av_packet_free(&packet);
        fileVideoDuration = fileAudioDuration = 0;
        m_curFmtCtx = NULL;
        m_curSegmentId ++;
        addFlushPkt();
        continue;
    }
}

void VEMediaSource::decv_thread()
{
    int lastSegmentId = 0;
    int rotate = 0;
    while (!m_hasExit){
        if(m_seekFlags){
            usleep(20 * 1000);
            continue;
        }
        std::unique_lock<std::mutex> lock(m_mtx_video);
        MediaPacket pkt;
        if(!m_videoQ.getPacket(pkt)){
            lock.unlock();
            usleep(20 * 1000);
            continue;
        }
        if(pkt.segmentId == FLUSH_PKT_ID){
            PREVIEW_LOG_INFO("flush video pkt \n");
        	changeVideoDecoder(m_curVidCtx,lastSegmentId);
        	continue;
        }
        
        if(lastSegmentId != pkt.segmentId || m_curVidCtx == NULL){
            m_minVideoPts = -1;
            if(m_curVidCtx && m_segments[lastSegmentId]->last_time <= 0)
                changeVideoDecoder(m_curVidCtx, lastSegmentId);
            if(!m_segments[pkt.segmentId]->test_segment)
                openCodec(m_segments[pkt.segmentId]->ctx_video);
            else{
                if(m_curVidCtx){
                    avcodec_free_context(&m_curVidCtx);
                    m_curVidCtx = NULL;
                }
            }
        
            lastSegmentId = pkt.segmentId;
            rotate = m_segments[pkt.segmentId]->rotate;
        }
        if(pkt.reuseUntilMs && pkt.packet == NULL){
            addTestFrame(pkt);
            continue;
        }
        
        if(m_curBsf){
            av_packet_split_side_data(pkt.packet);
            m_curBsf->applyBitstreamFilter(m_curVidCtx, pkt.packet);
        }
        
        AVFrame *frame = av_frame_alloc();
        VideoFrame2 videoFrame;
        VideoFrame2 *videoFrameOne = NULL;
        videoFrame.rotate = rotate;
        videoFrame.test = 0;
        videoFrame.overlayId = 0;
        videoFrame.transitionId = m_segments[pkt.segmentId]->transition_id;
        videoFrame.trackId = m_segments[pkt.segmentId]->track_id;
        videoFrame.clipId = m_segments[pkt.segmentId]->clip_id;
        if(m_videoQ.getFirstId() == FLUSH_PKT_ID && ((m_segments[pkt.segmentId]->pts_min - pkt.packet->pts) > 0)){
            if((m_segments[pkt.segmentId]->pts_min - pkt.packet->pts) < 300){
                m_segments[pkt.segmentId]->pts_min = pkt.packet->pts;
            }
        }
#ifdef __APPLE__
        if((m_curVidCtx->codec_id == AV_CODEC_ID_H265 || m_curVidCtx->codec_id == AV_CODEC_ID_H264) && isIOSVerAbove(10.0)){
            if(pkt.packet->pts < m_segments[pkt.segmentId]->pts_min){
                pkt.packet->pos = -360;
            }
        }
#endif
        int got = 0;
        PREVIEW_LOG_DEBUG("seek push a video packet to decoder %lld key=%d pos = %d\n",pkt.packet->pts,pkt.packet->flags,pkt.packet->pos);
        int ret = avcodec_decode_video2(m_curVidCtx, frame, &got,pkt.packet);
#ifdef __APPLE__
        if(m_curVidCtx->pix_fmt == AV_PIX_FMT_VIDEOTOOLBOX){
            frame->best_effort_timestamp = frame->pts;
        }
#endif
        if(got){
        	PREVIEW_LOG_INFO("decode frametimestamp %lld  \n",frame->best_effort_timestamp);
        }
        if(got && m_sinkv &&(frame->best_effort_timestamp >= m_segments[pkt.segmentId]->pts_min || (m_curSegmentId >= m_numSegments && m_videoQ.size() == 0)) && (m_minVideoPts == -1 || frame->best_effort_timestamp >= m_minVideoPts)){
            if(m_minVideoPts == -1){
                m_minVideoPts = frame->best_effort_timestamp;
                PREVIEW_LOG_INFO("segment %d min video pts %lld \n",pkt.segmentId,m_minVideoPts);
            }
            int loop = pkt.reuseUntilMs ? 1 : 0;
            do
            {
                if(videoFrameOne == NULL){
                    int width = 0,height = 0;
                    uint8_t *pData = NULL;
                    int size = 0;
#ifndef __APPLE__
                    pData = storeVideoFrame(frame,&width,&height,&size);
#else
                    pData = storeVideoFrameNative(frame, &width, &height);
#endif
                    videoFrame.size = size;
                    videoFrame.pData = pData;
                    videoFrame.timestamp = frame->best_effort_timestamp;
                    videoFrame.clipTimestamp = GetSegOriginTime(m_segments[pkt.segmentId], videoFrame.timestamp);
                    videoFrame.width = width;
                    videoFrame.height = height;
                    videoFrame.rotate = m_segments[pkt.segmentId]->rotate;
                    videoFrame.test = 0;
                    videoFrame.overlayId = 0;//m_segments[segId]->overlayId;
                    videoFrame.clipId = m_segments[pkt.segmentId]->clip_id;
                    videoFrame.trackId = m_segments[pkt.segmentId]->track_id;
                    videoFrame.transitionId = m_segments[pkt.segmentId]->transition_id;
                    videoFrame.startTime = m_segments[pkt.segmentId]->insert_start_time;
                    videoFrame.endTime = m_segments[pkt.segmentId]->insert_end_time;
                    if(width && height){
                        videoFrame.native = 0;
                    }else{
                        videoFrame.native = 1;
                    }
                    if(loop && videoFrame.timestamp > pkt.reuseUntilMs){
                        videoFrame.timestamp = pkt.reuseUntilMs;
                    }
                }
//#ifdef __APPLE__
                if(videoFrameOne){
                    videoFrame = CopyVideoFrame(*videoFrameOne);
                    videoFrame.timestamp = frame->best_effort_timestamp;
                }
                
                if(loop && videoFrameOne == NULL /*&& videoFrame.native*/){
                    videoFrameOne = new VideoFrame2();
                    *videoFrameOne = CopyVideoFrame(videoFrame);
                }
//#endif
                BEGIN_LOOP_QUEUE_FRAME(m_sinkv,videoFrame,m_videoSrcId)
                {
                    if(m_hasExit || m_seekFlags){
                        ReleaseVideoFrame(&videoFrame);
                        loop = 0;
                        break;
                    }
                }
                END_LOOP_QUEUE_FRAME()
                
                if(loop){
                    if(pkt.reuseUntilMs > (frame->best_effort_timestamp)){
                        frame->best_effort_timestamp += VIDEO_FRAME_INTERVAL;
                        if(frame->best_effort_timestamp > pkt.reuseUntilMs)
                            frame->best_effort_timestamp = pkt.reuseUntilMs;
                    }else{
                        loop = 0;
                    }
                }
            }while (loop);

        }else{
#ifdef __APPLE__
            if(frame->linesize[7] == AV_PIX_FMT_VIDEOTOOLBOX){
                CVPixelBufferRelease((CVPixelBufferRef)frame->data[7]);
            }
#endif

// #ifdef __ANDROID__
			// if (frame->format == AV_PIX_FMT_MEDIACODEC && frame->linesize[7] == AV_PIX_FMT_MEDIACODEC)
   //          {
			// 	PREVIEW_LOG_INFO("%s decv_thread statusCallback(VIDEO_EDIT_PREVIEW_FRAME_DROP) 2 got=%d fmt=%d linesize=%d\n",TAGVEPLAYER, got, frame->format, frame->linesize[7]);
   //          	m_sinkg->statusCallback(VIDEO_EDIT_PREVIEW_FRAME_DROP);
   //          }
// #endif
        }

        av_packet_free(&pkt.packet);
        av_frame_free(&frame);
        if(videoFrameOne){
            ReleaseVideoFrame(videoFrameOne);
            delete videoFrameOne;
        }
    }
}

void VEMediaSource::deca_thread()
{
    SetThreadHighPriority();
    int lastSegmentId = 0;
    while (!m_hasExit){
        if(m_seekFlags){
            usleep(20 * 1000);
            continue;
        }
        
        std::unique_lock<std::mutex> lock(m_mtx_audio);
        AudioFrame2 audioFrame;
        MediaPacket pkt;
        if(!m_audioQ.getPacket(pkt)){
            lock.unlock();
            usleep(50 *1000);
            continue;
        }
        
        if(pkt.segmentId == FLUSH_PKT_ID)
            continue;
        
        if(pkt.reuseUntilMs){
            if((pkt.reuseUntilMs /*+ m_segments[pkt.segmentId]->pts_min*/) >= pkt.pts){
                if(m_sinka && pkt.pts >= m_segments[pkt.segmentId]->pts_min){
//                    float speed;
//                    speed = m_segments[pkt.segmentId]->speed;//getSegSpeed(m_type, m_segments[pkt.segmentId]->srcSeq);
//                    if(speed != m_segments[pkt.segmentId]->speed)
//                        m_segments[pkt.segmentId]->speed = speed;
//                    int tmpSilentBufLen = m_lenSilentBuf;
//                    if(!VEPlayerUtils::IsFloatEqual(speed,1.0))
//                        tmpSilentBufLen = (int)(m_lenSilentBuf * 1.0 / speed);
//                    m_posAudioFrameBuffer += tmpSilentBufLen;
                    m_posAudioFrameBuffer = m_lenSilentBuf;
                    
                    int64_t tmpPTS = pkt.pts;// calTranslateTimestamp(pkt.pts);
                    while (m_posAudioFrameBuffer >= m_lenSilentBuf){
                        memset(m_pAudioFrameBuffer, 0, m_posAudioFrameBuffer);
                        uint8_t *pData = new uint8_t[m_lenSilentBuf];
                        memcpy(pData, m_pAudioFrameBuffer, m_lenSilentBuf);
                        m_posAudioFrameBuffer -= m_lenSilentBuf;
                        audioFrame.pData = pData;
                        audioFrame.len = m_lenSilentBuf;
                        audioFrame.timestamp = tmpPTS;
                        audioFrame.volume = 0;
                        
                        BEGIN_LOOP_QUEUE_FRAME(m_sinka, audioFrame, m_audioSrcId)
                        {
                            if(m_hasExit || m_seekFlags){
                                ReleaseAudioFrame(&audioFrame);
                                break;
                            }
                        }
                        END_LOOP_QUEUE_FRAME()
                    }

                }
                pkt.pts += SILENT_FRAME_INTERVAL;
                pkt.dts += SILENT_FRAME_INTERVAL;
                m_audioQ.putPacketFront(pkt);
            }
        }else{
            if(lastSegmentId != pkt.segmentId || m_curAudCtx == NULL){
                openCodec(m_segments[pkt.segmentId]->ctx_audio);
                lastSegmentId = pkt.segmentId;
            }
            
            if(pkt.packet ==  NULL)
                continue;
            
            int got = 0;
            AVFrame *frame = av_frame_alloc();
            int ret = avcodec_decode_audio4(m_curAudCtx, frame, &got,pkt.packet);
            
            if(ret < 0 || !got){
                PREVIEW_LOG_WARN("audio frame decode failed \n");
            }else{
                int pitch = m_segments[pkt.segmentId]->pitch;
                audioFrame.volume = getSegmentVolume(m_segments[pkt.segmentId]->track_id, m_segments[pkt.segmentId]->clip_id);
                float speed = getFrameSpeed(m_type, pkt.segmentId, pkt.pts);
                storeAudioFrame(frame,audioFrame.volume,speed,pitch,pkt.segmentId);
                audioFrame.volume = 100;
                int64_t timestamp = frame->best_effort_timestamp;
                
                while (m_posAudioFrameBuffer >= m_lenSilentBuf){
                    uint8_t *pData = new uint8_t[m_lenSilentBuf];
                    memcpy(pData, m_pAudioFrameBuffer, m_lenSilentBuf);
                    m_posAudioFrameBuffer -= m_lenSilentBuf;
                    memmove(m_pAudioFrameBuffer, m_pAudioFrameBuffer + m_lenSilentBuf, m_posAudioFrameBuffer);
                    audioFrame.len = m_lenSilentBuf;
                    audioFrame.pData = pData;
                    audioFrame.timestamp = timestamp;
                    
                    if(frame->best_effort_timestamp < m_segments[pkt.segmentId]->pts_min){
                        if((m_curSegmentId >= m_numSegments && m_audioQ.size() == 0)){
                            //last frame
                        }
                        else{
                            ReleaseAudioFrame(&audioFrame);
                            m_posAudioFrameBuffer = 0;
                            break;
                        }
                    }
                    
                    BEGIN_LOOP_QUEUE_FRAME(m_sinka, audioFrame, m_audioSrcId)
                    {
                        if(m_hasExit || m_seekFlags)
                        {
                            ReleaseAudioFrame(&audioFrame);
                            break;
                        }
                    }
                    END_LOOP_QUEUE_FRAME()
                    
                    timestamp += SILENT_FRAME_INTERVAL;
                }
            }
            av_packet_free(&pkt.packet);
            av_frame_free(&frame);
        }
    }
}

void VEMediaSource::openCodec(AVCodecContext *ctx)
{
    if(ctx == NULL)
        return ;
    AVCodecContext * _ctx = NULL;
    if(ctx->codec_type == AVMEDIA_TYPE_AUDIO){
        if(m_curAudCtx){
            avcodec_free_context(&m_curAudCtx);
            m_curAudCtx = NULL;
        }
        
        m_curAudCtx = avcodec_alloc_context3(NULL);
        avcodec_copy_context(m_curAudCtx, ctx);
        _ctx = m_curAudCtx;
        
        if(m_curAfVol){
            delete m_curAfVol;
            m_curAfVol = NULL;
        }
        if(m_pSoundTouch){
            delete m_pSoundTouch;
            m_pSoundTouch = NULL;
        }
    }else{
        if(m_curBsf){
            delete m_curBsf;
            m_curBsf = NULL;
        }
        if(m_curVidCtx){
            avcodec_free_context(&m_curVidCtx);
            m_curVidCtx = NULL;
        }
        if(m_swsCtx){
            sws_freeContext(m_swsCtx);
            m_swsCtx = NULL;
        }
        m_curVidCtx = avcodec_alloc_context3(NULL);
        avcodec_copy_context(m_curVidCtx, ctx);
        m_curVidCtx->thread_type = 0;
        _ctx = m_curVidCtx;
    }
    AVCodec *_codec = NULL;
    if(ctx->codec_id == AV_CODEC_ID_H264){
#ifdef __APPLE__
        if(VE_HW_DECODER && isIOSVerAbove(9.0)){
            _codec = &h264_videotoolbox_decoder;
            _ctx->pix_fmt = AV_PIX_FMT_VIDEOTOOLBOX;
            _ctx->opaque = _codec;
            m_curBsf = new VEBitstream("h264_mp4toannexb");
        }
#endif
#ifdef __ANDROID__
        /*if(GetAndroidVersion() >= 21)
		{
        	if(enableHwDecodeGPMediaCodecDecoderEx() && m_type == SOURCE_TYPE_VIDEO_MAIN)
			{
				Mediacodec2Param* param = (Mediacodec2Param*) malloc(sizeof(Mediacodec2Param));
				if(param){
					PREVIEW_LOG_ERROR("%s api >= 21 use ff_h264_mediacodec_async_decoder\n", TAGVEPLAYER);

					param->filepath = NULL;
					param->width = m_curVidCtx->width;
					param->height = m_curVidCtx->height;
					param->timeBase = (AVRational ) { 1, 1000 };
					param->render = m_sinkg->getRender();

					_codec = &ff_h264_mediacodec_async_decoder;
					_ctx->pix_fmt = AV_PIX_FMT_MEDIACODEC;
					_ctx->opaque = param;
					m_curBsf = new VEBitstream("h264_mp4toannexb");
				}
			}

        	if(_codec == NULL)
        	{
        		_codec = &ff_h264_mediacodec_decoder;
        	}
        }else*/ 
        if(0 && GetAndroidVersion() >= 19){
        	_codec = &ff_h264_mediacodec_decoder;
        }
#endif
    }
    
    if(ctx->codec_id == AV_CODEC_ID_H265){
#ifdef __APPLE__
        if(VE_HW_DECODER && isIOSVerAbove(11.0)){
            _codec = &hevc_videotoolbox_decoder;
            _ctx->pix_fmt = AV_PIX_FMT_VIDEOTOOLBOX;
            _ctx->opaque = &m_vtMode;
            m_curBsf = new VEBitstream("hevc_mp4toannexb");
        }
#endif
#ifdef __ANDROID__
        _codec = NULL;
#endif
    }
    
    if(ctx->codec_id == AV_CODEC_ID_VP8){
        _codec = &ff_libvpx_vp8_decoder;
    }
    
//    if(ctx->codec_id == AV_CODEC_ID_VP9 )
//    {
//        _codec = &ff_libvpx_vp9_decoder;
//    }
    if(_codec == NULL)
        _codec = avcodec_find_decoder(ctx->codec_id);
    int ret = avcodec_open2(_ctx, _codec, NULL);
    if (ret < 0){
#ifdef __ANDROID__
    	/*
    	if(ctx->codec_id != AV_CODEC_ID_H265 && _codec == &ff_h264_mediacodec_async_decoder && GetAndroidVersion() >= 19)
    	{
    		avcodec_close(_ctx);

    		_codec = &ff_h264_decoder;
    		int ret0 = avcodec_open2(_ctx, _codec, NULL);
    		if(ret0 < 0)
    		{
    			return;
    		}
    	}
    	else
    	*/
    	{
#endif
        PREVIEW_LOG_ERROR("avcodec_open failed\n");
        return;
#ifdef __ANDROID__
    	}
#endif
    }

}

void VEMediaSource::storeAudioFrame(AVFrame *frame,int vol,float speed,int pitch,int segId)
{
    int bufLenLeft = AUDIO_FRAME_BUFFER_SIZE - m_posAudioFrameBuffer;
    uint8_t *pBuf = m_pAudioFrameBuffer + m_posAudioFrameBuffer;
    uint8_t *pTmp = NULL;
    int pcmLen = 0;
    char expr[256];
    ve_audio_filter_param audioParam;
    audioParam.m_samplerateForSpeed = m_curAudCtx->sample_rate * speed;
    audioParam.m_volume = vol;
    
    ve_filter *fadeFilter = findFadeFilter(m_segments[segId]->track_id, m_segments[segId]->clip_id , GetSegOriginTime(m_segments[segId], frame->best_effort_timestamp));
    
    if(fadeFilter){
        audioParam.m_fade = fadeFilter->af_type;
        audioParam.m_gainMax = fadeFilter->gain_max;
        audioParam.m_gainMin = fadeFilter->gain_min;
        audioParam.m_nbSamples =  ((int64_t)(fadeFilter->end_time - fadeFilter->start_time) / speed) * frame->sample_rate / 1000;
        audioParam.m_startSamples = (GetSegRealTime(m_segments[segId], fadeFilter->start_time) + m_segments[segId]->insert_start_time) * frame->sample_rate / 1000;
    }
    
    VESource::getAudioFilterString(&audioParam,expr);
    if(strcmp(m_lastAudioFilter, expr) || m_curAfVol == NULL){
        if(m_curAfVol){
            delete m_curAfVol;
        }
        m_curAfVol = new VEAudioFilter();
        m_curAudCtx->time_base.num = 1;
        m_curAudCtx->time_base.den = 1000;
        m_curAfVol->addFilters(m_curAudCtx,expr);
        
        strcpy(m_lastAudioFilter, expr);
        m_segments[segId]->volume = vol;
    }
    
    if(pitch != m_segments[segId]->pitch ||(m_pSoundTouch == NULL && pitch != 0)){
        if(m_pSoundTouch == NULL){
            m_pSoundTouch = new VESoundTouch();
        }
        if(m_pSoundTouchBuffer == NULL){
            m_pSoundTouchBuffer = new uint8_t[1024*1024];
        }
        m_pSoundTouch->configSoundTouchPitch(pitch, /*frame->channels*/AUDIO_RENDER_CHANNELS, AUDIO_RENDER_SAMPLERATE);
        m_segments[segId]->pitch = pitch;
    }
    
    int len = 0;
    
    m_curAfVol->process(frame, (short **)&pTmp, &pcmLen);
    uint8_t *pPCMData = pTmp;
    len = pcmLen;
    if(pitch != 0){
        m_pSoundTouch->processData(pTmp, pcmLen, m_pSoundTouchBuffer, &len);
        pPCMData = m_pSoundTouchBuffer;
    }
    if(bufLenLeft >= len){
        memcpy(pBuf, pPCMData, len);
        m_posAudioFrameBuffer += len;
    }
    PREVIEW_LOG_TRACE("storeAudioFrame a audio frame pts = %lld len = %d \n",frame->best_effort_timestamp,len);
    
}

#ifdef __ANDROID__
uint8_t* VEMediaSource::storeVideoFrame(AVFrame *frame,int *width,int *height,int *size)
{
    int w = frame->width;
    int h = frame->height;
    
    if(frame->width > 4096 && frame->width >= frame->height){
        w = 4096;
        h = w * frame->height / frame->width;
    }
    else if(frame->height > 4096 && frame->height >= frame->width){
        h = 4096;
        w = h * frame->width /frame->height;
    }
    if(w % 8)
        w = ((w + 7) / 8) * 8;
    if(h % 2)
        h = ((h + 1)/ 2) * 2;

    if (frame->format == AV_PIX_FMT_MEDIACODEC && frame->linesize[7] == AV_PIX_FMT_MEDIACODEC) {
    	*width = w;
    	*height = h;
		return NULL;
	}
    uint8_t *pData = NULL;//new uint8_t[w * h * 3 /2];
    
    int y_length = w * h;
    int len = y_length * 3 / 2;
    *width = w;
    *height = h;
    *size = len;
    
    /*if(frame->format == AV_PIX_FMT_RGB24)
    {
        libyuv::RAWToI420((uint8_t*)frame->data[0],frame->linesize[0],pData, frame->width,
                          pData + y_length, frame->width / 2,
                          pData + y_length * 5 / 4, frame->width / 2,
                          frame->width, frame->height);
    }
    else if(frame->format == AV_PIX_FMT_RGBA)
    {
        libyuv::ABGRToI420((uint8_t*)frame->data[0],frame->linesize[0],pData, frame->width,
                           pData + y_length, frame->width / 2,
                           pData + y_length * 5 / 4, frame->width / 2,
                           frame->width, frame->height);
    }
    else */if(frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P){
        //if(frame.pFrame->format == AV_PIX_FMT_YUV420P)
//        {
//            libyuv::I420Copy((uint8_t*)frame->data[0],frame->linesize[0],(uint8_t*)frame->data[1],frame->linesize[1],(uint8_t*)frame->data[2],frame->linesize[2],pData, w,
//                             pData + y_length, w / 2,
//                             pData + y_length * 5 / 4, w / 2,
//                             frame->width, frame->height);
//        }
        pData = new uint8_t[*size];
        libyuv::I420Scale((uint8_t *)frame->data[0], frame->linesize[0],(uint8_t *)frame->data[1],  frame->linesize[1],(uint8_t *)frame->data[2], frame->linesize[2], frame->width, frame->height, pData, w,pData + y_length, w/2 ,pData + y_length * 5 / 4, w/2, w, h, libyuv::kFilterNone);
    }else{
        //ffmpeg
        AVFrame *tmp;
        int tmp_w = frame->width;
        int tmp_h = frame->height;
        AVPixelFormat dst_fmt = AV_PIX_FMT_YUV420P;
        if(frame->format == AV_PIX_FMT_YUVA420P || frame->format >= AV_PIX_FMT_YUVA422P && frame->format <=AV_PIX_FMT_YUVA444P16LE ||
           frame->format >= AV_PIX_FMT_RGBA64BE && frame->format <= AV_PIX_FMT_BGRA64LE ||
           frame->format >= AV_PIX_FMT_GBRAP && frame->format <= AV_PIX_FMT_GBRAP16LE ||
           frame->format >= AV_PIX_FMT_AYUV64LE && frame->format <= AV_PIX_FMT_AYUV64BE ||
           frame->format >= AV_PIX_FMT_GBRAP12BE && frame->format <= AV_PIX_FMT_GBRAP10LE ||
           frame->format >= AV_PIX_FMT_GBRAPF32BE && frame->format <= AV_PIX_FMT_GBRAPF32LE){
            dst_fmt = AV_PIX_FMT_BGRA; //mov透明通道
            tmp_w = w;
            tmp_h = h;
            *size = w * h * 4;
        }
        
        pData = new uint8_t[*size];
        
        if(m_swsCtx == NULL){
            m_swsCtx = sws_getContext(frame->width, frame->height,(AVPixelFormat)frame->format,tmp_w, tmp_h, dst_fmt, SWS_BILINEAR, 0, 0, 0);
        }
        tmp = av_frame_alloc();
        tmp->format = dst_fmt;
        tmp->width = tmp_w;
        tmp->height = tmp_h;
        av_frame_get_buffer(tmp, 1);
        
        if(tmp && m_swsCtx)
            sws_scale(m_swsCtx, frame->data, frame->linesize, 0, frame->height,tmp->data, tmp->linesize);
        
        if(dst_fmt == AV_PIX_FMT_YUV420P){
            libyuv::I420Scale((uint8_t*)tmp->data[0],tmp->linesize[0],(uint8_t*)tmp->data[1],tmp->linesize[1],(uint8_t*)tmp->data[2],tmp->linesize[2],tmp->width,tmp->height,pData, w,
                         pData + y_length, w / 2,
                         pData + y_length * 5 / 4, w / 2,
                         w, h,libyuv::kFilterNone);
        }
        else{
            if(tmp->linesize[0] != tmp_w * 4){
                PREVIEW_LOG_ERROR("tmp frame linesieze %d is not tmp_w %d \n",tmp->linesize[0],tmp_w);
            }
            memcpy(pData, tmp->data[0], *size);
        }
        av_frame_free(&tmp);
    }
    return pData;
}
#endif

#ifdef  __APPLE__
uint8_t* VEMediaSource::storeVideoFrameNative(AVFrame *frame, int *width, int *height)
{
    *width = 0;
    *height = 0;
    if(frame->linesize[7] == AV_PIX_FMT_VIDEOTOOLBOX){
        return frame->data[7];
    }
    int w = frame->width;
    int h = frame->height;
    int max_res = 2048;
#ifdef __APPLE__
    max_res = 4096;
#endif
    if(frame->width > max_res && frame->width >= frame->height){
        w = max_res;
        h = w * frame->height / frame->width;
    }
    else if(frame->height > max_res && frame->height >= frame->width){
        h = max_res;
        w = h * frame->width /frame->height;
    }
    
    if(w % 8)
        w = ((w + 7) / 8) * 8;
    if(h % 2)
        h = ((h + 1) / 2) * 2;
    
    AVPixelFormat dst_fmt = AV_PIX_FMT_NV12;
    if(frame->format == AV_PIX_FMT_YUVA420P || frame->format >= AV_PIX_FMT_YUVA422P && frame->format <=AV_PIX_FMT_YUVA444P16LE ||
       frame->format >= AV_PIX_FMT_RGBA64BE && frame->format <= AV_PIX_FMT_BGRA64LE ||
       frame->format >= AV_PIX_FMT_GBRAP && frame->format <= AV_PIX_FMT_GBRAP16LE ||
       frame->format >= AV_PIX_FMT_AYUV64LE && frame->format <= AV_PIX_FMT_AYUV64BE ||
       frame->format >= AV_PIX_FMT_GBRAP12BE && frame->format <= AV_PIX_FMT_GBRAP10LE ||
       frame->format >= AV_PIX_FMT_GBRAPF32BE && frame->format <= AV_PIX_FMT_GBRAPF32LE){
        dst_fmt = AV_PIX_FMT_BGRA; //mov透明通道
    }
    //ffmpeg
    AVFrame *tmp;
    if(m_swsCtx == NULL)
    {
        m_swsCtx = sws_getContext(frame->width, frame->height,(AVPixelFormat)frame->format,w, h, dst_fmt, SWS_BILINEAR, 0, 0, 0);
    }
    tmp = av_frame_alloc();
    tmp->format = dst_fmt;
    tmp->width = w;
    tmp->height = h;
    av_frame_get_buffer(tmp, 16);
    
    if(tmp && m_swsCtx)
        sws_scale(m_swsCtx, frame->data, frame->linesize, 0, frame->height,tmp->data, tmp->linesize);
    
    void *pixelbuf = NULL;
    if(dst_fmt == AV_PIX_FMT_NV12)
        GetPixelBufferFromNV12(w, h, tmp->data[0], tmp->data[1], tmp->linesize[0], &pixelbuf);
    else if(dst_fmt == AV_PIX_FMT_BGRA)
        GetPixelBufferFromRGBA(dst_fmt, w, h, tmp->data, tmp->linesize, &pixelbuf);
    else
        PREVIEW_LOG_ERROR("dst pixel fmt not support \n");
    av_frame_free(&tmp);

    return (uint8_t *)pixelbuf;
}
#endif

void VEMediaSource::flush()
{
    PREVIEW_LOG_INFO("mediasource flush\n");
    {
        std::unique_lock<std::mutex> lock(m_mtx_video);
        m_videoQ.flush();
        if(m_curVidCtx){
        	avcodec_flush_buffers(m_curVidCtx);
        }
        m_sinkv->flush(m_videoSrcId);
    }
    
    {
        std::unique_lock<std::mutex> lock (m_mtx_audio);
        m_audioQ.flush();
        if(m_curAudCtx)
            avcodec_flush_buffers(m_curAudCtx);
        m_sinka->flush();
    }
    m_minVideoPts = -1;
    
    PREVIEW_LOG_INFO("mediasource flush finish \n");
}

void VEMediaSource::changeVideoDecoder(AVCodecContext *ctx,int segId)
{
    PREVIEW_LOG_INFO("change video decoder start \n");
    if(ctx){
        int got = 0;
        do
        {
            AVFrame *frame = av_frame_alloc();
            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.data = NULL;
            pkt.size = 0;
            
            int ret = avcodec_decode_video2(ctx, frame, &got,&pkt);
            if(ret < 0 || got == 0){
                PREVIEW_LOG_INFO("change video finish2  ret = %d \n",ret);
                av_frame_free(&frame);
                break;
            }
            
            VideoFrame2 videoFrame;
            int width,height;
            uint8_t *pData = NULL;
            int size = 0;
#ifndef __APPLE__
            pData = storeVideoFrame(frame,&width,&height,&size);
#else
            pData = storeVideoFrameNative(frame, &width, &height);
#endif
            videoFrame.size = size;
            videoFrame.pData = pData;
            videoFrame.timestamp = frame->best_effort_timestamp;
            videoFrame.clipTimestamp = GetSegOriginTime(m_segments[segId], videoFrame.timestamp);
            videoFrame.width = width;
            videoFrame.height = height;
            videoFrame.rotate = m_segments[segId]->rotate;
            videoFrame.test = 0;
            videoFrame.overlayId = 0;//m_segments[segId]->overlayId;
            videoFrame.clipId = m_segments[segId]->clip_id;
            videoFrame.trackId = m_segments[segId]->track_id;
            videoFrame.transitionId = m_segments[segId]->transition_id;
            videoFrame.startTime = m_segments[segId]->insert_start_time;
            videoFrame.endTime = m_segments[segId]->insert_end_time;
            if(width && height){
                videoFrame.native = 0;
            }else{
                videoFrame.native = 1;
            }
                
            PREVIEW_LOG_INFO("change video decoder pdata=%x native = %d size = %d  w=%d h=%d\n",videoFrame.pData,videoFrame.native,videoFrame.size,width,height);

            m_sinkv->writeFrame(videoFrame,m_videoSrcId,1);
            av_frame_free(&frame);
            
        }while(got);
    }
    PREVIEW_LOG_INFO("change video decoder finish \n");
}


int VEMediaSource::getSegId(int64_t ts)
{
    int i = m_numSegments - 1;
    int segIdx = -1;
    for(i;i>=0;i--){
        if(m_segments[i]->insert_end_time <= ts){
            segIdx = i;
            break;
        }
    }
    return segIdx;
}

int VEMediaSource::getSegmentVolume(int type, int segmentid)
{
    return m_sinkg->getSegVolume(type, segmentid);
}

struct segment* VEMediaSource::addTestSegment(int offset, int duration){
    segment *seg = new segment();
    if(!seg){
        PREVIEW_LOG_ERROR("%s failed \n",__FUNCTION__);
        m_sinkg->statusCallback(VE_ERR_PREVIEW_PARAM);
        return NULL;
    }
    seg->offset = offset;
    seg->start_time = 0;
    seg->end_time = duration;
    seg->duration = duration;
    seg->test_segment = 1;
    int i = 0;
    for(i;i < m_numSegments;i++){
        seg->offset += m_segments[i]->duration;
    }
    seg->insert_start_time = offset;
    seg->insert_end_time = offset + duration;
    seg->pts_min = seg->insert_start_time;
    seg->pts_max = seg->insert_end_time;
    seg->src_seq = m_numSegments;
    return seg;
}

int VEMediaSource::insertSegment(VEConfig *config,VETrackData *track, int index, int transition)
{
    int insert_time = 0;
    int insert_time_duration = 0;
    VEClipData *clip = &(track->m_clips[index]);
    float speed = config->m_speed * track->m_track.speed * clip->m_clip.speed;
    
    VETransitionData *transition_data = NULL;
    if(transition){
        std::map<int,VETransitionData>::iterator it_transiton = track->m_transitions.begin();
        for(it_transiton;it_transiton!=track->m_transitions.end();it_transiton++){
            if(it_transiton->second.m_transition.clip_index_b == index){
                transition_data = &(it_transiton->second);
                break;
            }
        }
    }
    
    if(transition){
        insert_time = clip->m_insertTime;
        insert_time_duration = transition_data->m_transition.duration / speed;
        if(clip->m_clip.slv.active){
            int slv_dur = GetSlvDuration(&(clip->m_clip.slv), clip->m_clip.start_time, clip->m_clip.start_time + transition_data->m_transition.duration);
            insert_time_duration = slv_dur / speed;
        }
    }else{
        insert_time = clip->m_clip.insert_time;
        insert_time_duration = clip->m_clip.duration / speed;
        if(clip->m_clip.slv.active){
            int slv_dur = GetSlvDuration(&(clip->m_clip.slv), clip->m_clip.start_time, clip->m_clip.end_time);
            insert_time_duration = slv_dur / speed;
        }
    }
    
    int segIdx = 0;
    int insert_end_time = insert_time + insert_time_duration;
    for (segIdx;segIdx<m_numSegments;segIdx++){
        segment * seg = m_segments[segIdx];
        if(seg->test_segment && seg->insert_start_time <= insert_time && insert_end_time <= seg->insert_end_time){
            break;
        }
    }
    if(segIdx >= m_numSegments)
        return -1;
    segment * segL = addNewTestSegment(m_segments[segIdx]->insert_start_time, insert_time);
    segment * segM = addNewSegment(config, track, index, transition, insert_time);
    segment * segH = addNewTestSegment(insert_end_time, m_segments[segIdx]->insert_end_time);
    
    replaceSegment(segL, segM, segH, segIdx);

    return 0;
}

int VEMediaSource::newSource(VEConfig *config,VETrackData *track, int index, int transitoin,int duration)
{
    VEClipData *clip = &(track->m_clips[index]);
    int insert_time = 0;
    if(transitoin){
        insert_time = clip->m_insertTime;
    }else{
        insert_time = clip->m_clip.insert_time;
    }
    
    if(insert_time < 0){
        PREVIEW_LOG_ERROR("%s insert time wrong \n",__FUNCTION__);
        return -1;
    }
    
    if(insert_time > 0){
        segment *seg = addTestSegment(0, insert_time);
        m_segments[m_numSegments] = seg;
        m_numSegments ++;
    }
    
    m_duration += insert_time;
    segment *seg = addNewSegment(config, track,index, transitoin, insert_time);
    m_duration += getSegRealDuration(seg);
    m_segments[m_numSegments] = seg;
    m_numSegments ++;
    
    if(m_duration < duration){
        segment *seg = addTestSegment(m_duration, duration - m_duration);
        m_duration = duration;
        m_segments[m_numSegments] = seg;
        m_numSegments ++;
    }
    
    if(track->m_track.type == VE_TRACK_AUDIO){
        m_type = SOURCE_TYPE_AUDIO;
    }
    else{
        m_type = SOURCE_TYPE_VIDEO_OVERLAY;
    }
    return 0;
}

struct segment* VEMediaSource::addNewSegment(VEConfig *config, VETrackData *track, int index, int transition,int insert_time)
{
    VEClipData *clip = &(track->m_clips[index]);
    segment *seg  = new segment();
    seg->track_id = track->m_track.track_id;
    seg->clip_id = clip->m_clip.clip_id;
    seg->last_time  = clip->m_clip.type == VE_CLIP_PICTURE ? clip->m_clip.duration : -1;
    seg->start_time = clip->m_clip.start_time;
    seg->end_time   = clip->m_clip.end_time;
    seg->speed      = clip->m_clip.speed * config->m_speed * track->m_track.speed;
    seg->transition_id = -1;
    
    VETransitionData *transition_data = NULL;
    if(transition){
        std::map<int,VETransitionData>::iterator it_transiton = track->m_transitions.begin();
        for(it_transiton;it_transiton!=track->m_transitions.end();it_transiton++){
            if(it_transiton->second.m_transition.clip_index_b == index){
                transition_data = &(it_transiton->second);
                if(seg->last_time > 0){
                    seg->last_time = it_transiton->second.m_transition.duration;
                }else{
                    seg->end_time = seg->start_time + it_transiton->second.m_transition.duration;
                }
                int trans_duration = it_transiton->second.m_transition.duration / seg->speed;
                clip->m_insertTime = m_duration - trans_duration;
                break;
            }
        }
        seg->transition_id = transition_data->m_transition.transition_id;
    }else{
        if(track->m_transitions.size() > 0){ //只增加非transition区域
            std::map<int,VETransitionData>::iterator it_transiton = track->m_transitions.begin();
            for(it_transiton;it_transiton!=track->m_transitions.end();it_transiton++){
                if(it_transiton->second.m_transition.clip_index_b == index){
                    if(seg->last_time > 0){
                        seg->last_time -= it_transiton->second.m_transition.duration;
                    }else{
                        seg->start_time += it_transiton->second.m_transition.duration;
                    }
                    int trans_duration = it_transiton->second.m_transition.duration / seg->speed;
                    clip->m_insertTime = m_duration - trans_duration;
                }
            }
        }
    }
    
    seg->offset     = 0;
    seg->volume     = 0;//clip->m_clip.volume + config->m_volume + track->m_track.volume;
    VEConfig::ajustVolume(config, track, &(clip->m_clip), &seg->volume);
    seg->pitch      = clip->m_clip.pitch;
    seg->ctx        = NULL;
    seg->ctx_video = seg->ctx_audio = NULL;
    seg->audio_idx = seg->video_idx = -1;
    seg->duration_audio = seg->duration_video = 0;
    seg->videoCounter = 0;
    seg->slv = clip->m_clip.slv;
    if(seg->slv.active){
        seg->start_time = clip->m_clip.slv.clip_start_time;
        seg->end_time = clip->m_clip.slv.clip_end_time;
        if(transition && transition_data){
            seg->end_time = seg->start_time + transition_data->m_transition.duration;
            seg->slv.clip_end_time = seg->end_time;
        }
        int c = 0;
        for(c;c<seg->slv.len;c++){
            seg->slv.speed[c] = seg->slv.speed[c] * seg->speed;
        }
        seg->speed = 1.0;
    }
    
    strcpy(seg->url, clip->m_clip.filename);
    if(clip->m_clip.type == VE_CLIP_PICTURE)
        seg->rotate = clip->m_clip.picture_rotate;
    else
        seg->rotate = clip->m_clip.info.rotate;
    
    int i;
    for(i=0 ; i < m_numSegments ; i++){
        seg->offset += m_segments[i]->duration;
    }
    
    if(seg->last_time > 0){
        //seg->last_time = (seg->last_time * 1.0) / seg->speed;
        seg->duration = seg->last_time;
    }
    else
        seg->duration   = seg->end_time - seg->start_time;
    
    seg->src_seq = m_numSegments;
    seg->test_segment = 0;
    seg->insert_start_time = insert_time;
    seg->insert_end_time = insert_time + getSegRealDuration(seg);
    
    PREVIEW_LOG_INFO("add track segment[%d:%d]: last_time[%lld] start_time[%lld] end_time[%lld] filename[%s] duration [%lld]\n",seg->track_id,seg->clip_id,seg->last_time,seg->start_time,seg->end_time,seg->url,seg->duration);
    return seg;
}

struct segment *VEMediaSource::addNewTestSegment(int insert_time, int insert_end_time)
{
    segment *seg = new segment();
    if(!seg){
        PREVIEW_LOG_ERROR("%s failed \n",__FUNCTION__);
        m_sinkg->statusCallback(VE_ERR_PREVIEW_PARAM);
        return NULL;
    }
    seg->offset = 0;
    seg->start_time = 0;
    seg->end_time = insert_end_time - insert_time;
    seg->duration = insert_end_time - insert_time;
    seg->test_segment = 1;
    int i = 0;
    for(i;i < m_numSegments;i++){
        seg->offset += m_segments[i]->duration;
    }
    seg->insert_start_time = insert_time;
    seg->insert_end_time = insert_end_time;
    seg->src_seq = m_numSegments;
    seg->pts_min = seg->insert_start_time;
    seg->pts_max = seg->insert_end_time;
    return seg;
}

int VEMediaSource::addTestSegment(segment *seg,int time)
{
    if(seg->test_segment){
        MediaPacket packet;
        packet.packet = NULL;
        int h_buf = 0;
        if(m_curSegmentId != 0)
            h_buf = 1;
        packet.pts = packet.dts = seg->pts_min;//seg->insert_end_time - time + h_buf;
        packet.reuseUntilMs = seg->insert_end_time;
        packet.segmentId = m_curSegmentId;
        //seg->pts_min =  seg->insert_end_time - time + h_buf;//seg->offset;
        //seg->pts_max = packet.reuseUntilMs;
        
        m_audioQ.putPacketBack(packet,1);
        if(m_type <= SOURCE_TYPE_VIDEO_OVERLAY)
            m_videoQ.putPacketBack(packet,1);
        
    }
    return 0;
}

int VEMediaSource::addTestFrame(MediaPacket packet)
{
    VideoFrame2 frame;
    int64_t ptsStart = packet.pts;
    int64_t ptsEnd   = packet.reuseUntilMs;
    while (ptsStart < ptsEnd){
        frame.test = 1;
        frame.timestamp = ptsStart;
        ptsStart += 50; //20fps
        
        BEGIN_LOOP_QUEUE_FRAME(m_sinkv,frame,m_videoSrcId)
        {
            if(m_hasExit || m_seekFlags){
                ptsStart = ptsEnd;
                break;
            }
        }
        END_LOOP_QUEUE_FRAME()
        
    }
    
    return 0;
}

int VEMediaSource::addFlushPkt()
{
    MediaPacket pkt;
    pkt.packet = NULL;
    pkt.segmentId = FLUSH_PKT_ID; //flush pkt
    m_videoQ.putPacketBack(pkt,1);
    m_audioQ.putPacketBack(pkt,1);
    return 0;
}


int VEMediaSource::getSourceType()
{
    return m_type;
}


ve_filter *VEMediaSource::findFadeFilter(int trackId,int clipId,int64_t ts)
{
    return m_sinkg->findFadeFilter(trackId, clipId, ts);
}

float VEMediaSource::getFrameSpeed(int type, int segmentid, int64_t timestamp)
{
    float speed = 1.0;
    if(type == SOURCE_TYPE_VIDEO_MAIN || type == SOURCE_TYPE_VIDEO_OVERLAY){
        if(m_segments[segmentid]->slv.active){
            int64_t mediaTime = timestamp;
            int i = 0;
             timestamp = GetSegOriginTime(m_segments[segmentid], timestamp);
            for(i;i<m_segments[segmentid]->slv.len;i++){
                speed = m_segments[segmentid]->speed;
                if(timestamp >= m_segments[segmentid]->slv.start_time[i] && timestamp <= m_segments[segmentid]->slv.end_time[i]){
                    speed = speed * m_segments[segmentid]->slv.speed[i];
                    break;
                }
            }
        }else{
            speed = m_segments[segmentid]->speed;
        }
    }else{
        speed = m_segments[segmentid]->speed;
    }
    return speed;
}



int VEMediaSource::getSegRealDuration(segment *seg)
{
    int duration = 0;
    duration = seg->duration;
    if(seg->slv.active){
        duration = GetSlvDuration(&(seg->slv), seg->slv.clip_start_time, seg->slv.clip_end_time);
    }
    
    duration = (duration * 1.0)/seg->speed;
        
    return duration;
}

int VEMediaSource::getVideoQueueId()
{
    return m_videoSrcId;
}

int VEMediaSource::replaceSegment(segment *segL, segment *segM, segment *segH, int index)
{
    if(m_numSegments <= 0 || segL == NULL){
        PREVIEW_LOG_ERROR("insert segment failed \n");
        return -1;
    }
    
    segment *segments_tmp[MAX_SEG_CNT];
    int i =0;
    int count = 0;
    for(i;i<m_numSegments;i++){
        if(m_segments[i]->insert_end_time > m_segments[i]->insert_start_time){
            segments_tmp[i] = m_segments[i];
            count ++;
        }else{
            PREVIEW_LOG_ERROR("insert segment failed as find a none segment \n");
            if(i < index)
                index --;
        }
    }
    i = 0;
    int base = 0;
    m_numSegments = 0;
    for(i;i<count;i++){
        if(i == index){
            delete segments_tmp[i];
            if(segL->insert_end_time - segL->insert_start_time > 0){
                m_segments[m_numSegments] = segL;
                m_segments[m_numSegments]->src_seq =  m_numSegments;
                base ++;
                m_numSegments ++;
            }
            
            if(segM->insert_end_time - segM->insert_start_time > 0){
                m_segments[m_numSegments] = segM;
                m_segments[m_numSegments]->src_seq =  m_numSegments;
                base ++;
                m_numSegments ++;
            }
            
            if(segH->insert_end_time - segH->insert_start_time > 0){
                m_segments[m_numSegments] = segH;
                m_segments[m_numSegments]->src_seq =  m_numSegments;
                base ++;
                m_numSegments ++;
            }
        }else{
            m_segments[m_numSegments] = segments_tmp[i];
            m_segments[m_numSegments]->src_seq =  m_numSegments;
            m_numSegments ++;
        }
    }
    
    return 0;
}

void VEMediaSource::setHW(int usehw)
{
    m_useHW = usehw;
}

bool VEMediaSource::hasOverlay(int64_t ts)
{
    int idx = m_numSegments - 1;
    
    for(idx;idx>=0;idx--){
        if(m_segments[idx]->insert_start_time <= ts){
            break;
        }
    }
    return m_segments[idx]->test_segment ? false : true;
}
