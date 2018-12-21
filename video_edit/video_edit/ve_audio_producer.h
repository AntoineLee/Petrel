#ifndef __VE_AUDIO_PRODUCER__
#define __VE_AUDIO_PRODUCER__

#include "ve_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AP_STATUS_FILE_OPEN_FAILED = 0,
    AP_STATUS_FILE_NO_AUDIO,
    AP_STATUS_FILE_DECODE_FAILED,
    AP_STATUS_FILE_COMPLETE
}AudioProducerStatus;

/**
 * @brief 音频数据回调
 * @param client_handle producer指针
 * @param pPCM 返回pcm数据buffer，内存由producer分配和释放
 * @param size pPCM长度
 * @param userExt ve_audio_producer_create设置的自定义数据
 */
typedef void (*ve_audio_producer_callback)(HANDLE client_handle,unsigned char *pPCM,int size, void* userExt);

/**
 * @brief 音频状态回调
 * @param client_handle 同数据回调
 * @param status 同AudioProducerStatus
 * @param userExt 同数据回调
 */
typedef void (*ve_audio_producer_status_callback)(HANDLE client_handle,int status,void* userExt);
    

/**
 * @brief 创建一个audio producer
 * @param edit_config_param_handle 参数配置句柄
 * @param callback 数据回调
 * @param status_callback 状态回调
 * @param userExt 用户私有数据
 * @return producer handle
 */
EXPORT_API HANDLE ve_audio_producer_create(HANDLE edit_config_param_handle, ve_audio_producer_callback callback,ve_audio_producer_status_callback status_callback,void* userExt);
    
/**
 * @brief 开始从audio producer获取数据
 * @param handle ve_audio_producer_create返回的handle
 * @param track_id track id
 * @param clip_id  clip id
 * @start_time 音频数据开始时间
 * @end_time   音频数据结束时间
 * @return success 0 else -1
 */
EXPORT_API int ve_audio_producer_start(HANDLE handle,int track_id,int clip_id,int64_t start_time,int64_t end_time);
    
/**
 * @brief 释放audio producer
 * @return success 0 else -1
 */
EXPORT_API int ve_audio_producer_free(HANDLE handle);


#ifdef __cplusplus
}
#endif

#endif
