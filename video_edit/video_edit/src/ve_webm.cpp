#include "ve_webm.h"

#include "VELog.h"
#include "VEWebm.h"

#define VE_MODULE_TAG "[VEWebmInterface]"


HANDLE ve_webm_new(ve_webm_config *config){
    
	if(!config || config->output_width <= 0 || config->output_height <= 0 || !config->output_filename){
        
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!config || config->output_width <= 0 || config->output_height <= 0 || !config->output_filename");
		return NULL;
	}

    VE_LOG_TAG_INFO(VE_MODULE_TAG,"config->output_filename=%s,config->output_fps=%d,config->output_width=%d,config->output_height=%d,config->output_video_bitrate=%d", config->output_filename?config->output_filename:"",config->output_fps,config->output_width,config->output_height,config->output_video_bitrate);

	VEWebm * webm = new VEWebm(config);
	/*
    if (!webm) {
    	VE_LOG_ERROR("ve_webm_new failed");
    }
    */
	return webm;
}

void ve_webm_free(HANDLE handle){

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	VEWebm *webm  = (VEWebm*)handle;
	delete webm;
}

VE_WEBM_ERR ve_webm_start(HANDLE handle){
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	VEWebm *webm  = (VEWebm*)handle;
	if(webm) return webm->start();
	return VE_WEBM_ERR_INPUT_PARAM;
}


VE_WEBM_ERR ve_webm_send_frame(HANDLE handle,ve_webm_frame* frame){
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	VEWebm *webm  = (VEWebm*)handle;
	if(webm) return webm->sendFrame(frame);
	return VE_WEBM_ERR_INPUT_PARAM;
}

VE_WEBM_ERR ve_webm_stop(HANDLE handle){
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	VEWebm *webm  = (VEWebm*)handle;
	if(webm) return webm->stop();
	return VE_WEBM_ERR_INPUT_PARAM;
}
