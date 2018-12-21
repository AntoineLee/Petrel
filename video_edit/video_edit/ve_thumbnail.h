#ifndef __VE_THUMBNAIL__
#define __VE_THUMBNAIL__

#include "ve_type.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 *
 * 缩略图对象
 *
 * @param  max_mem_cache_num 设置最大内存缓存数
 * @return: 生成的缩略图对象HANDLE，NULL 失败
 *
 */
EXPORT_API HANDLE ve_thumbnail_new(int max_mem_cache_num = 300);

/**
 * 释放缩略图对象
 * @param handle 实例句柄
 */
EXPORT_API void ve_thumbnail_free(HANDLE handle);




typedef enum{
	VE_THUMBNAIL_PROCESSING = 0,
	VE_THUMBNAIL_END,
	VE_THUMBNAIL_CANCEL,

	VE_THUMBNAIL_ERR = -100,
}VE_THUMBNAIL_STATUS;



typedef struct{
	const char* mp4_file;
	int width;//缩略图宽，与video_edit_get_thumbnail中的一致
	int height;//缩略图高，与video_edit_get_thumbnail中的一致
	int rotate{VE_ROTATE_0};//旋转角度
	int index;//索引保留，？
	void* output_jpegs;//输出参数，jpeg文件的内存首地址
	int output_jpegs_len;//输出参数，jpeg文件的长度
}ve_thumbnail_callback_param;
typedef void (*ve_thumbnail_callback)(HANDLE handle,ve_thumbnail_callback_param* param,int status,void* userExt);

typedef struct{
	const char* filename{0};
	int width{0};
	int height{0};
	int start_time{0};
	int end_time{0};
	int count{0};
	int rotate{0};
	const char * path{0};
	ve_thumbnail_callback callback{0};
	int hw_decode{0};//for android
	void* userExt{0};
	bool single{false};
}ve_thumbnail_param;

/**
 * 获取录制后的mp4的缩略图
 * @param handle 实例句柄
 * @param param	参考ve_thumbnail_param

 * @return VE_ERR_OK,或错误码，请参考ve_interface.h
 */

EXPORT_API int ve_thumbnail_get(HANDLE handle, ve_thumbnail_param * param);

/**
 * cancel缩略图操作，缩略图线程退出，不再回调上层
 * @param handle 生成的视频编辑参数对象HANDLE，NULL 失败
 *
 */
EXPORT_API void ve_thumbnail_cancel(HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif /* __VE_THUMBNAIL__ */
