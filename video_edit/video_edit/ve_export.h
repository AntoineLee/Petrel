#ifndef __VE_EXPORT__
#define __VE_EXPORT__

#include "ve_type.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
	VE_EXPORT_PROCESSING = 0,
	VE_EXPORT_END,
	VE_EXPORT_CANCEL,

	VE_EXPORT_ERR = -100,
}VE_EXPORT_STATUS;



typedef struct{
	HANDLE handle;
	int status;
	float progress;
	int err_no;
	void* userExt;
}ve_export_status;


typedef void (*ve_export_status_callback)(ve_export_status* status);

typedef struct{
	void* userExt{NULL};
	int render{0};//for android
	int hw_encode{0};//for android
	int hw_decode{0};//for android
}ve_export_param;

/**
 * 创建视频编辑合成对象
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param filter_callback 视频特效处理回调
 * @return: 生成的视频编辑合成对象HANDLE，NULL 失败
 *
 */
EXPORT_API HANDLE ve_export_create(HANDLE timeline_handle, ve_filter_callback filter_callback, ve_export_status_callback status_callback, ve_export_param &param);

/**
 * 释放视频编辑合成对象
 * @param export_handle 实例句柄
 */
EXPORT_API void ve_export_free(HANDLE export_handle);

enum{
	ANDROID_MEDIACODEC_FRAME = 0,
	ANDROID_MEDIACODEC_KEY_FRAME,
	ANDROID_MEDIACODEC_PPS_SPS,
	ANDROID_RGBA,
};
typedef struct{
	int encoded{0};
	void* data{0};
	int len{0};
	int width{0};
	int height{0};
	int cur_time{0};

	//for android
	int frame_type{ANDROID_MEDIACODEC_FRAME};
	int end_frame{0};//保留，不再使用
}ve_filtered_data;


EXPORT_API void ve_export_send_filtered_data_back(HANDLE export_handle,ve_filtered_data* filtered_data);

EXPORT_API void ve_export_start(HANDLE export_handle);

EXPORT_API void ve_export_cancel(HANDLE export_handle);

#ifdef __cplusplus
}
#endif

#endif /* __VE_EXPORT__ */
