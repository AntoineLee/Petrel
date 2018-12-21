#include <stdio.h>
#include <stdlib.h> 
#include "ve_webm_wrap.h"
#include "VELog.h"

#define VE_MODULE_TAG "[VEWebmWrapInterface]"
typedef struct{
	HANDLE handle;
	ve_webm_config *config;
	ve_webm_frame* frame;
}ve_webm_wrap;

void freeMemory(ve_webm_wrap* o){
	if(o){
		if(o->config){
			if(o->config->output_filename){
				free((void*)o->config->output_filename);
				o->config->output_filename = NULL;
			}
			free(o->config);
			o->config = NULL;
		}

		if(o->frame){
			for (int i = 0; i < 8; ++i){
				if(o->frame->data[i]){
					free(o->frame->data[i]);
					o->frame->data[i] = NULL;
				}
			}
			free(o->frame);
			o->frame = NULL;
		}

		free(o);
		o = NULL;
	}
}

HANDLE ve_webm_wrap_new(ve_webm_config *config){
	HANDLE handle = ve_webm_new(config);
	if(handle != 0){
		ve_webm_wrap *o = (ve_webm_wrap *)malloc(sizeof(ve_webm_wrap));
		o->handle = handle;
		o->config = config;
		o->frame = NULL;
		return (HANDLE)o;
	}else {
		return NULL;
	}
}

/**
* 释放webm视频合成对象
* @param handle ve_webm_wrap_new创建的实例句柄
*/
void ve_webm_wrap_free(HANDLE handle){
	if(handle){
		ve_webm_wrap *o = (ve_webm_wrap *)handle;
		if(o->handle){
			ve_webm_free(o->handle);
			o->handle = NULL;
		}
		freeMemory(o);
	}
}


/**
* 开始合成，启动内部线程
* @param handle ve_webm_wrap_new创建的实例句柄
*/
VE_WEBM_ERR ve_webm_wrap_start(HANDLE handle){
	if(handle){
		ve_webm_wrap *o = (ve_webm_wrap *)handle;
		if(o->handle){
			return ve_webm_start(o->handle);
		}
	}
	return VE_WEBM_ERR_INPUT_PARAM;
}


VE_WEBM_ERR ve_webm_wrap_send_frame(HANDLE handle,ve_webm_frame* frame){
	if(handle){
		ve_webm_wrap *o = (ve_webm_wrap *)handle;
		if(o->handle){
			return ve_webm_send_frame(o->handle, frame);
		}
	}
	return VE_WEBM_ERR_INPUT_PARAM;
}
/**
* 结束合成，停止内部线程
* @param handle ve_webm_wrap_new创建的实例句柄
*/
VE_WEBM_ERR ve_webm_wrap_stop(HANDLE handle){
	if(handle){
		ve_webm_wrap *o = (ve_webm_wrap *)handle;
		if(o->handle){
			return ve_webm_stop(o->handle);
		}
	}
	return VE_WEBM_ERR_INPUT_PARAM;
}

void ve_webm_wrap_read_frame_for_test(const char* source_filename,ve_webm_frame* frame){
	//ve_webm_read_frame_for_test(source_filename, frame);
}
//-------------
// int linesize[8];//AV_PIX_FMT_YUVA420P格式数据
// int width;//源视频帧宽
// int height;//源视频帧高
ve_webm_frame* ve_webm_wrap_get_frame_cache(HANDLE handle, int* linesize, int linesizeLen, int width, int height, int pts){
	const int DATA_LENGTH = 8;
	if(!handle || !linesize || linesizeLen <= 0 || linesizeLen > DATA_LENGTH || width <= 0 || height <= 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"param exception! h=%0x linesize=%0x linesizeLen=%d w=%d h=%d", handle, linesize, linesizeLen, width, height);
		return NULL;
	}

	ve_webm_wrap *o = (ve_webm_wrap *)handle;
	if(o->frame){

		ve_webm_frame* frame = o->frame;

		int oldDataLen[DATA_LENGTH] = {0};
		for (int i = 0; i < DATA_LENGTH; ++i){
			const int oldLen = frame->height * frame->linesize[i];

			if(oldLen > 0 && frame->data[i]){
				// cache old data len
				oldDataLen[i] = oldLen;

				// memset old data
				memset(frame->data[i], 0, oldLen * sizeof(uint8_t));	
			}
		}

		// new frame
		frame->width = width;
		frame->height = height;
		frame->pts = pts;

		memcpy(frame->linesize, linesize, linesizeLen*sizeof(int));


		for (int i = 0; i < DATA_LENGTH; ++i){
			if(i < linesizeLen){
				const int curLen = height * linesize[i];
				if(curLen > oldDataLen[i]){

					frame->data[i] = (uint8_t *)realloc(frame->data[i], curLen);
					if(frame->data[i]){
						memset(frame->data[i], 0, curLen);	
					} else {
						VE_LOG_TAG_ERROR(VE_MODULE_TAG,"realloc failed!\n");
						break;
					}
				}
			}
		}

		return o->frame;

	} else {

		// init  ve_webm_wrap->frame
		ve_webm_frame* frame = (ve_webm_frame*)calloc(1, sizeof(ve_webm_frame));
		if(frame){

			frame->width = width;
			frame->height = height;
			frame->pts = pts;

			memcpy(frame->linesize, linesize, linesizeLen*sizeof(int));

			for (int i = 0; i < DATA_LENGTH; ++i){

				frame->data[i] = NULL;
				// maybe linesizeLen < DATA_LENGTH
				if(i < linesizeLen){
					const int len = height * linesize[i];
					if(len > 0){
						frame->data[i] = (uint8_t *) calloc(1, len * sizeof(uint8_t));
						if(!frame->data[i])	{
							VE_LOG_TAG_ERROR(VE_MODULE_TAG,"calloc data failed!\n");
							break;
						}
					}
				}
			}

			o->frame = frame;
		} else {
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"calloc frame failed!\n");
		}

		return o->frame;
	}

}
