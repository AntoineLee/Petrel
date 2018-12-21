#include "ve_interface.h"
#include "VEThumbnail.h"

#define VE_MODULE_TAG "[VEThumbanilInterface]"

HANDLE ve_thumbnail_new(int max_mem_cache_num){

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"max_mem_cache_num=%d", max_mem_cache_num);

	VEThumbnail * thumbnail = new VEThumbnail(max_mem_cache_num);
	/*
    if (!thumbnail) {
    	VE_LOG_ERROR("ve_thumbnail_new new  VEThumbnail failed");
    }
    */
	return thumbnail;
}

/**
 * 释放缩略图对象
 * @param handle 实例句柄
 */
void ve_thumbnail_free(HANDLE handle){

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");
	VEThumbnail * thumbnail = (VEThumbnail *)handle;
	if(thumbnail)delete thumbnail;
}

int ve_thumbnail_get(HANDLE handle, ve_thumbnail_param *param){


	VEThumbnail * thumbnail = (VEThumbnail *)handle;
    if(thumbnail){
        VE_LOG_TAG_INFO(VE_MODULE_TAG,"filename=%s,width=%d,height=%d,start_time=%d,end_time=%d,count=%d,rotate=%d,path=%s,userExt=%p",param->filename,param->width,param->height,param->start_time,param->end_time,param->count,param->rotate,param->path,param->userExt);
        return thumbnail->getThumbnails(param);
    }
	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle = NULL");
	return VE_ERR_INPUT_PARAM;
}


void ve_thumbnail_cancel(HANDLE handle){

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");
	VEThumbnail * thumbnail = (VEThumbnail *)handle;
	if(thumbnail)thumbnail->cancel();
	else{
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle = NULL");
	}
}


