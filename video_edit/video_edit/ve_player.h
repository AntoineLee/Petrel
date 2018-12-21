#ifndef __VE_PLAYER__
#define __VE_PLAYER__
#include "ve_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	VE_PLAYER_ON_FRAME = 90, // GP ADDED.
    VE_PLAYER_SEEK_COMPLETE = 100,
    VE_PLAYER_PLAYBACK_COMPLETE
}VE_PLAYER_STATUS;


/**
 * @brief 视频帧数据回调
 * @param player_handle player句柄
 * @param param player返回的当前需要渲染的视频帧信息
 * @param userExt ve_player_create设置的自定义数据
 */
typedef void (*ve_player_callback)(HANDLE player_handle,ve_filter_callback_param* param,void* userExt);
    
/**
 * @brief player状态回调
 * @param player_handle 同数据回调
 * @param status player状态,同VE_PLAYER_STATUS
 * @param userExt 同数据回调
 */
typedef void(*ve_player_status_callback)(HANDLE player_handle,int status,void* userExt);

    
/**
 * @brief 创建player
 * @param edit_config_param_handle 参数配置句柄
 * @param callback 视频帧数据回调
 * @param status_callback player状态回调
 * @param userExt 用户数据
 * @param video_edit_render 安卓硬解支持
 * @return player句柄
 */
#ifdef __ANDROID__
EXPORT_API HANDLE ve_player_create(HANDLE edit_config_param_handle,HANDLE video_edit_render, ve_player_callback callback,ve_player_status_callback status_callback,void* userExt = NULL);
#else
EXPORT_API HANDLE ve_player_create(HANDLE edit_config_param_handle, ve_player_callback callback,ve_player_status_callback status_callback,void* userExt);
#endif

/**
 * @brief 设置硬解(安卓)
 * @param handle player句柄 由ve_player_create
 * @param usehw 1：开启硬解
 */
EXPORT_API void ve_player_usehw(HANDLE handle,int usehw);

/**
 * @brief player预览开始
 * @param handle player句柄
 */
EXPORT_API void ve_player_start(HANDLE handle);

/**
 * @brief 释放视频编辑player对象
 * @param handle 实例句柄
 */
EXPORT_API void ve_player_free(HANDLE handle);


/**
 * @brief  预览播放seek
 * @param  handle player句柄
 * @param  seekto seek的位置(ms)
 * @param  force  强制seek
 */
EXPORT_API void ve_player_seek(HANDLE handle,int seekto, int force = 0);

/**
 * @brief   预览播放seek到片段
 * @param   handle player句柄
 * @param   idx    片段索引
 */
//EXPORT_API void video_edit_preview_player_seek_idx(HANDLE handle,int idx);

/**
 * @brief   预览播放开始
 * @param   handle player句柄
 */
EXPORT_API void ve_player_play(HANDLE handle);

/**
 * @brief    预览播放暂停
 * @param    handle player句柄
 */
EXPORT_API void ve_player_pause(HANDLE handle);

/**
 * @brief 获取当前播放时间
 * @param handle player句柄
 * @return 当前流时间(ms)
 */
EXPORT_API int64_t ve_player_get_current_time(HANDLE handle);
    
/**
 *  @brief 刷新所有参数配置
 *  @param edit_config_param_handle 参数配置句柄
 *  @param player_handle 播放器句柄
 *  @return 0:sucess
 */
EXPORT_API int ve_player_refresh_config(HANDLE edit_config_param_handle,HANDLE player_handle);

#ifdef __cplusplus
}
#endif

#endif
