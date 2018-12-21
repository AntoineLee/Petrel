#include "VEPlayer.h"
#include "ve_interface.h"
#include "VEGraphManager.h"
#include "VEPlayerLogs.h"

#ifdef __ANDROID__
EXPORT_API HANDLE ve_player_create(HANDLE edit_config_param_handle,HANDLE video_edit_render, ve_player_callback callback,ve_player_status_callback status_callback,void* userExt)
#else
EXPORT_API HANDLE ve_player_create(HANDLE edit_config_param_handle, ve_player_callback callback,ve_player_status_callback status_callback,void* userExt)
#endif
{
    VEGraphManager *preview = new VEGraphManager(edit_config_param_handle,callback,status_callback,userExt);
#ifdef __ANDROID__
    preview->setRender((void*)video_edit_render);
#endif
#ifdef __ANDROID__
	av_log_set_callback(veLogCallbackFfmpeg);

#endif
    return preview;
}

EXPORT_API void ve_player_start(HANDLE handle)
{
    VEGraphManager *preview = (VEGraphManager *)handle;
    if(preview)
        preview->start();
}

/**
 * 释放视频编辑预览播放器对象
 * @param handle 实例句柄
 */
EXPORT_API void ve_player_free(HANDLE handle)
{
    VEGraphManager *preview = (VEGraphManager *)handle;
    if(preview)
        delete preview;
    return;
}


/**
 * 预览播放seek
 * @return: 生成的预览播放器对象HANDLE，NULL 失败
 *
 */
EXPORT_API void ve_player_seek(HANDLE handle,int seekto,int force)
{
    VEGraphManager *preview = (VEGraphManager *)handle;
    if(preview)
        preview->seekTo(seekto,force);
    else
        PREVIEW_LOG_INFO("invalid seek %d \n",seekto);
    return;
}

//EXPORT_API void video_edit_preview_player_seek_idx(HANDLE handle,int idx)
//{
//    GraphManager *preview = (GraphManager *)handle;
//    if(preview)
//        preview->seekToIdx(idx);
//    return;
//}

/**
 * 预览播放
 * @return: 生成的预览播放器对象HANDLE，NULL 失败
 *
 */
EXPORT_API void ve_player_play(HANDLE handle)
{
    VEGraphManager *preview = (VEGraphManager *)handle;
    if(preview)
        preview->play();
    return;
}

/**
 * 预览暂停
 * @return: 生成的预览播放器对象HANDLE，NULL 失败
 *
 */
EXPORT_API void ve_player_pause(HANDLE handle)
{
    VEGraphManager *preview = (VEGraphManager *)handle;
    if(preview)
        preview->pause();
    return ;
}

//int64_t video_edit_preview_player_duration(HANDLE handle)
//{
//    GraphManager *preview = (GraphManager *)handle;
//    if(preview)
//        return preview->getDuration();
//    return 0;
//}

int64_t ve_player_get_current_time(HANDLE handle)
{
    int ret = 0;
    VEGraphManager *preview = (VEGraphManager *)handle;
    if(preview)
        ret = preview->getCurrentTime();
    return ret;
}


//int video_edit_preview_get_frame_params(HANDLE handle,int64_t ts,ve_param_video_filter** render_cmd,int *render_cmd_len)
//{
//    GraphManager *preview = (GraphManager *)handle;
//    if(preview)
//    {
//        return preview->getFrameParams(ts, render_cmd, render_cmd_len);
//    }
//
//    return 0;
//}

//int video_edit_preview_freeing(HANDLE handle)
//{
//    GraphManager *preview = (GraphManager *)handle;
//    if(preview)
//    {
//        return preview->stopping();
//    }
//
//    return 0;
//}

int ve_player_refresh_config(HANDLE edit_config_param_handle,HANDLE player_handle)
{
    VEGraphManager *preview = (VEGraphManager *)player_handle;
    if(preview){
        preview->refreshHandle(edit_config_param_handle);
        return  0;
    }
    return -1;
}

void ve_player_usehw(HANDLE handle,int usehw)
{
    VEGraphManager *preview = (VEGraphManager *)handle;
    if(preview)
    {
        preview->setHW(usehw);
    }
}

#include "VEAudioProducer.h"

HANDLE ve_audio_producer_create(HANDLE edit_config_param_handle, ve_audio_producer_callback callback,ve_audio_producer_status_callback status_callback,void* userExt)
{
    AudioProducer *producer = new AudioProducer(edit_config_param_handle,callback,status_callback,userExt);
    PRODUCER_LOG_INFO("new audio producer %x \n",producer);
    return producer;
}

int ve_audio_producer_start(HANDLE handle,int track_id,int clip_id,int64_t start_time,int64_t end_time)
{
    int ret = -1;
    AudioProducer *producer = (AudioProducer *)handle;
    if(producer){
        ret = producer->start(track_id, clip_id, start_time, end_time);
    }
    return ret;
}

int ve_audio_producer_free(HANDLE handle)
{
    int ret = -1;
    AudioProducer *producer = (AudioProducer *)handle;
    if(producer){
        PRODUCER_LOG_INFO("free audio producer %x \n",producer);
        delete producer;
        ret = 0;
    }
    return ret;
}

