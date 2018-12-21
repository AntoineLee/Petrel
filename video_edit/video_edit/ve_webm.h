#ifndef __VE_WEBM_INTERFACE__
#define __VE_WEBM_INTERFACE__

#include "ve_type.h"

#ifdef __cplusplus
extern "C" {
#endif


#define BITRATE_720P	4500000
#define BITRATE_720P_Y_SIZE	777600

typedef enum{
	VE_WEBM_ERR_OK = 0,

	VE_WEBM_ERR_INPUT_PARAM,
	VE_WEBM_ERR_MALLOC_FAILED,
	VE_WEBM_ERR_CREATE_THREAD_FAILED,
	VE_WEBM_ERR_OPEN_FILE_FAILED,
	VE_WEBM_ERR_WRITE_FILE_FAILED,
	VE_WEBM_ERR_NEW_STREAM_FAILED,
	VE_WEBM_ERR_OPEN_ENCODER_FAILED,

	//...	...


}VE_WEBM_ERR;


/**
 * 调用顺序
 *
 *ve_webm_new
 *ve_webm_start
 *
 *ve_webm_send_frame		loop
 *	...	...
 *
 *ve_webm_stop	返回值为 VE_WEBM_ERR_OK 表示成功
 *ve_webm_free
 */


enum{
	//VE_WEBM_COLOR_YUVA420P,
	VE_WEBM_COLOR_ARGB,//ARGB_8888,packet
    VE_WEBM_COLOR_RGBA,//for ios
};
typedef struct{
	const char* output_filename{NULL};
	int output_width{1280};
	int output_height{720};
	int output_video_bitrate{BITRATE_720P};//byte单位
	int output_fps{30};//保留先暂时不用
	int frame_format{VE_WEBM_COLOR_ARGB};//AV_PIX_FMT_ARGB
	//... ...
}ve_webm_config;

/**
 * 创建webm视频合成对象
 * @param config 参考ve_webm_config
 * @return: 生成的webm视频合成对象HANDLE，NULL 失败
 *
 */
    
EXPORT_API HANDLE ve_webm_new(ve_webm_config *config);

/**
 * 释放webm视频合成对象
 * @param handle ve_webm_new创建的实例句柄
 */
EXPORT_API void ve_webm_free(HANDLE handle);


/**
 * 开始合成，启动内部线程
 * @param handle ve_webm_new创建的实例句柄
 */
EXPORT_API VE_WEBM_ERR ve_webm_start(HANDLE handle);


typedef struct{
	uint8_t *data[8];//AV_PIX_FMT_ARGB格式数据
	int linesize[8];//AV_PIX_FMT_ARGB格式数据
	int width;//源视频帧宽
	int height;//源视频帧高
	int pts;//时间戳，单位毫秒，从0开始第一帧为0）
	//... ...
}ve_webm_frame;

EXPORT_API VE_WEBM_ERR ve_webm_send_frame(HANDLE handle,ve_webm_frame* frame);
/**
 * 结束合成，停止内部线程
 * @param handle ve_webm_new创建的实例句柄
 */
EXPORT_API VE_WEBM_ERR ve_webm_stop(HANDLE handle);



//EXPORT_API void ve_webm_read_frame_for_test(const char* source_filename,ve_webm_frame* frame);

#ifdef __cplusplus
}
#endif

#endif
