#include "ve_interface.h"
#include "VEConfig.h"
#include "VEExport.h"

#define VE_MODULE_TAG "[VEInterface]"

extern char ve_error_info[VE_ERROR_INFO_LEN];
extern log_printer veLogPrinter;
void ve_enable_log(	int enable ){
	VEConfig::enableVELog(enable);
}

void ve_set_log_printer(	log_printer printer){
	veLogPrinter = printer;
}
void ve_get_err_info(char* info,int info_size){
#ifdef __APPLE__
	snprintf(info,info_size,ve_error_info);
#endif
}

HANDLE ve_timeline_create(ve_timeline *timeline){


	VEConfig* handle = NULL;

	if(!timeline){
		handle = new VEConfig();
		return handle;
	}

	if(!timeline->output_width || !timeline->output_height || !timeline->filename){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"falied! error param");
		return NULL;
	}

	handle = new VEConfig(timeline);

	/*
	if(!handle){
		VE_LOG_TAG_ERROR("ve_timeline_create falied!");
	}
	*/

	return handle;
}

VE_ERR ve_timeline_reconfig(HANDLE timeline_handle,ve_timeline *timeline){
	VEConfig* config = (VEConfig*)timeline_handle;

	if(config && timeline){
		return config->reconfig(timeline);
	}else{
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
        return VE_ERR_INPUT_PARAM;
	}
}

VE_ERR ve_timeline_get_config(HANDLE timeline_handle,ve_timeline *timeline){

	VEConfig* config = (VEConfig*)timeline_handle;

	if(config && timeline){
		return config->getConfig(timeline);
	}else{
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
        return VE_ERR_INPUT_PARAM;
	}
}

void ve_timeline_free(HANDLE handle){

	VEConfig* config = (VEConfig*)handle;

	if(config){
		delete config;
	}else{
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
	}
}

VE_ERR ve_timeline_get_duration(HANDLE handle,int actual,int * duration){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getTimelineDuration(actual,duration);
}

VE_ERR ve_track_add(HANDLE handle,ve_track *track){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->addTrack(track);

}
VE_ERR ve_track_mod(HANDLE handle,ve_track *track){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->modTrack(track);

}
VE_ERR ve_track_del(HANDLE handle,int track_id){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->delTrack(track_id);

}
VE_ERR ve_track_get(HANDLE handle,int track_id,ve_track *track){


	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getTrack(track_id,track);

}
VE_ERR ve_get_file_info(const char* filename,ve_clip_info* info){

	if(!filename || !info){
		return VE_ERR_INPUT_PARAM;
	}

	VEConfig::initialize();

	return VESource::getSourceInfo(filename,info);

}

VE_ERR ve_clip_insert(HANDLE handle,ve_clip *clip,int index){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->insertClip(clip,index);


}
VE_ERR ve_clip_mod(HANDLE handle,ve_clip *clip){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->modClip(clip);

}

VE_ERR ve_clip_move(HANDLE timeline_handle,int track_id,int from,int to){

	VEConfig* config = (VEConfig*)timeline_handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->moveClip(track_id,from,to);

}

VE_ERR ve_clip_del(HANDLE handle,int  clip_id){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->delClip(clip_id);

}
VE_ERR ve_clip_del_by_index(HANDLE handle,int  track_id,int index){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->delClip(track_id,index);

}

VE_ERR ve_clip_get(HANDLE handle,int  clip_id,ve_clip* clip){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getClip(clip_id,clip);

}
VE_ERR ve_clip_get_by_index(HANDLE handle,int  track_id,int index,ve_clip* clip){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getClip(track_id,index,clip);

}
VE_ERR ve_clip_get_duration(HANDLE timeline_handle,int  clip_id,int* duration){

	VEConfig* config = (VEConfig*)timeline_handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getClipDuration(clip_id,duration);
}
VE_ERR ve_track_get_clips_count(HANDLE handle,int  track_id,int *count){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getClipsCount(track_id,count);

}
VE_ERR ve_track_get_clips(HANDLE handle,int track_id,ve_clip*  clips,int len){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getClips(track_id,clips,len);
}

VE_ERR ve_track_get_overall_duration(HANDLE handle,int track_id,int actual,int * duration){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getTrackDuration(track_id,actual,duration);

}


VE_ERR ve_track_get_duration(HANDLE handle,int index,int track_id,int actual,int * duration){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getTrackDuration(track_id,index,actual,duration);

}

VE_ERR ve_transition_add(HANDLE handle,ve_transition *transition){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->addTransition(transition);

}
VE_ERR ve_transition_mod(HANDLE handle,ve_transition *transition){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->modTransition(transition);

}
VE_ERR ve_transition_del(HANDLE handle,int transition_id){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->delTransition(transition_id);

}
VE_ERR ve_transition_get(HANDLE handle,int transition_id,ve_transition *transition){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getTransition(transition_id,transition);

}

VE_ERR ve_filter_add(HANDLE handle,ve_filter *filter){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->addFilter(filter);

}
VE_ERR ve_filter_mod(HANDLE handle,ve_filter *filter){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->modFilter(filter);
    
}
VE_ERR ve_filter_del(HANDLE handle,int filter_id){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->delFilter(filter_id);

}
VE_ERR ve_filter_get(HANDLE handle,int filter_id,ve_filter *filter){

	VEConfig* config = (VEConfig*)handle;

	if(!config){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"handle == NULL");
		return VE_ERR_INPUT_PARAM;
	}

	return config->getFilter(filter_id,filter);

}


HANDLE ve_export_create(HANDLE timeline_handle,ve_filter_callback  filter_callback,ve_export_status_callback status_callback,ve_export_param &param){

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	VEExport* handle  = new VEExport((VEConfig*)timeline_handle,filter_callback,status_callback,param);

	return handle;
}

void ve_export_free(HANDLE export_handle){

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	VEExport* handle = (VEExport*)export_handle;

	delete handle;

}
void ve_export_send_filtered_data_back(HANDLE export_handle,ve_filtered_data* filtered_data){

	VEExport* handle = (VEExport*)export_handle;

	if(handle){
		handle->sendData(filtered_data);
	}

}

void ve_export_start(HANDLE export_handle){

	VEExport* handle = (VEExport*)export_handle;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	if(handle){
		handle->start();
	}

}

void ve_export_cancel(HANDLE export_handle){

	VEExport* handle = (VEExport*)export_handle;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	if(handle){
		handle->cancel();
	}

}
