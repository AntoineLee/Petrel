#ifndef __VE_WEBM_INTERFACE_WRAP__
#define __VE_WEBM_INTERFACE_WRAP__


#include "ve_webm.h"

#ifdef __cplusplus
extern "C" {
#endif



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

/**
 * 创建webm视频合成对象
 * @param cofnig 参考ve_webm_config
 * @return: 生成的webm视频合成对象HANDLE，NULL 失败
 *
 */

EXPORT_API HANDLE ve_webm_wrap_new(ve_webm_config *config);

/**
 * 释放webm视频合成对象
 * @param handle ve_webm_wrap_new创建的实例句柄
 */
EXPORT_API void ve_webm_wrap_free(HANDLE handle);


/**
 * 开始合成，启动内部线程
 * @param handle ve_webm_wrap_new创建的实例句柄
 */
EXPORT_API VE_WEBM_ERR ve_webm_wrap_start(HANDLE handle);


EXPORT_API VE_WEBM_ERR ve_webm_wrap_send_frame(HANDLE handle,ve_webm_frame* frame);
/**
 * 结束合成，停止内部线程
 * @param handle ve_webm_wrap_new创建的实例句柄
 */
EXPORT_API VE_WEBM_ERR ve_webm_wrap_stop(HANDLE handle);

EXPORT_API ve_webm_frame* ve_webm_wrap_get_frame_cache(HANDLE handle, int* linesize, int linesizeLen, int width, int height, int pts);

//EXPORT_API void ve_webm_wrap_read_frame_for_test(const char* source_filename,ve_webm_frame* frame);

#ifdef __cplusplus
}
#endif

#endif
