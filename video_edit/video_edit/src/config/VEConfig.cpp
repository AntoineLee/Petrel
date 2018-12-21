#include "VEConfig.h"
#include "VESource.h"

int VEConfig::m_veLogEnable = 1;
extern int veLogEnable;

#define VE_MODULE_TAG "[VEConfig]"

VEConfig::VEConfig(ve_timeline *timeline){
	m_id = timeline->timeline_id;
	m_width = timeline->output_width;
	m_height = timeline->output_height;
	m_outputFps = timeline->output_fps;
	m_bitrate = m_videoBitrate = timeline->video_bitrate;
	m_audioBitrate = timeline->audio_bitrate;
	m_filename = timeline->filename?timeline->filename:"";
	m_context = timeline->context;
	m_volume = timeline->volume;
	m_speed = timeline->speed;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"timeline->timeline_id=%d,timeline->output_width=%d,timeline->output_height=%d,timeline->output_fps=%d,timeline->video_bitrate=%d,timeline->audio_bitrate=%d",timeline->timeline_id,timeline->output_width,timeline->output_height,timeline->output_fps,timeline->video_bitrate,timeline->audio_bitrate);

	VEConfig::initialize();
}
int VEConfig::m_inited = 0;

void VEConfig::initialize(){

	if(!m_inited){
		avcodec_register_all();
		avfilter_register_all();
		av_register_all();
		avformat_network_init();

#ifdef __ANDROID__
		avcodec_register(&ff_libx264_encoder);
#endif
		/*
		avcodec_register(&ff_libfdk_aac_encoder);
		avcodec_register(&ff_libqy265_decoder);
		avcodec_register(&ff_libqy265_encoder);
		*/

#ifdef __APPLE__
		avcodec_register(&ff_h264_videotoolbox_encoder);
		//av_register_hwaccel(&my_ff_h264_videotoolbox_hwaccel);
        /*
		AVHWAccel* hw_h264 = &my_ff_h264_videotoolbox_hwaccel;
		AVHWAccel** hw_head = av_hwaccel_head();
		hw_h264->next = (*hw_head);
		(*hw_head) = hw_h264;
         */
#endif
		m_inited = true;
	}
}
VEConfig::~VEConfig(){

}

VEConfig & VEConfig::operator =(VEConfig & other){

	comn::AutoCritSec lock(m_cs);
	comn::AutoCritSec lock2(other.m_cs);

	m_context = other.m_context;
	m_id = other.m_id;
	m_filename = other.m_filename;
	m_width = other.m_width;
	m_height = other.m_height;
	m_bitrate = other.m_bitrate;
	m_outputFps = other.m_outputFps;
	m_videoBitrate = other.m_videoBitrate;
	m_audioBitrate = other.m_audioBitrate;
	m_volume = other.m_volume;
	m_speed = other.m_speed;


	m_filters = other.m_filters;

	m_tracks = other.m_tracks;
	m_multitrackNum = other.m_multitrackNum;

	reAssignFiltersStrPtr(m_filters);


	std::map<int,VETrackData>::iterator trackIt = m_tracks.begin();

	if(m_tracks.size()){
		for(;trackIt != m_tracks.end();trackIt++){
			VETrackData & trackData = trackIt->second;
			int size = trackData.m_clips.size();
			int i = 0;
			for(;i<size;i++){
				VEClipData & clipData = trackData.m_clips[i];
				reAssignFiltersStrPtr(clipData.m_filters);
				clipData.m_clip.filename = clipData.m_filename.c_str();
				clipData.m_clip.original_filename = clipData.m_originalFilename.c_str();

			}
			reAssignFiltersStrPtr(trackData.m_filters);

			std::map<int,VETransitionData>::iterator transitionIt = trackData.m_transitions.begin();
			if(trackData.m_transitions.size()){
				for(;transitionIt != trackData.m_transitions.end();transitionIt++){
					VETransitionData & transitionData = transitionIt->second;
					transitionData.m_transition.action = transitionData.m_action.c_str();
				}
			}
		}
	}
    
    return *this;
}
VE_ERR VEConfig::getConfig(ve_timeline *timeline){

	comn::AutoCritSec lock(m_cs);

	timeline->output_width = m_width;
	timeline->output_height = m_height;
	timeline->output_fps = m_outputFps;
	timeline->video_bitrate = m_videoBitrate;
	timeline->audio_bitrate = m_audioBitrate;
    timeline->filename = m_filename.size()?m_filename.c_str():"";
	timeline->context = m_context;
    timeline->speed = m_speed;

	return VE_ERR_OK;

}
VE_ERR VEConfig::reconfig(ve_timeline *timeline){

	comn::AutoCritSec lock(m_cs);



	float oldSpeed = m_speed;
	m_speed = timeline->speed;

	int size = m_tracks.size();
	if(timeline->speed > oldSpeed && size){
		std::map<int,VETrackData>::iterator it = m_tracks.begin();
		for(;it != m_tracks.end();it++){
			VETrackData* trackData = &it->second;

			if(invalidSpeedForTrack(trackData,trackData->m_track.speed)){
				m_speed = oldSpeed;
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalidSpeedForTrack(trackData,trackData->m_track.speed)");
				return VE_ERR_INPUT_PARAM;
			}
		}

	}

	m_id = timeline->timeline_id;
	m_width = timeline->output_width;
	m_height = timeline->output_height;
	m_outputFps = timeline->output_fps;
	m_bitrate = m_videoBitrate = timeline->video_bitrate;
	m_audioBitrate = timeline->audio_bitrate;
	m_context = timeline->context;
	m_volume = timeline->volume;

    m_filename = timeline->filename?timeline->filename:"";
    
    VE_LOG_TAG_INFO(VE_MODULE_TAG,"timeline->filename=%s,timeline->output_width=%d,timeline->output_height=%d,timeline->output_fps=%d,timeline->video_bitrate=%d,timeline->audio_bitrate=%d",timeline->filename?timeline->filename:"",timeline->output_width,timeline->output_height,timeline->output_fps,timeline->video_bitrate,timeline->audio_bitrate);
    
    return VE_ERR_OK;
}
VE_ERR VEConfig::getTimelineDurationInternal(int actual,int *duration){
	int maxDuration = 0;
	int trackDuration = 0;
	VE_ERR ret;

	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	if(0 == m_tracks.size()){
		*duration = maxDuration;
		return VE_ERR_OK;
	}

	for(;it != m_tracks.end();it++){

		VETrackData* trackData = &it->second;

		ret = getTrackDuration(trackData->m_track.track_id,actual,&trackDuration);

		if(trackDuration > maxDuration){
			maxDuration = trackDuration;
		}
	}

	*duration = maxDuration;

	return VE_ERR_OK;
}
VE_ERR VEConfig::getTimelineDuration(int actual,int *duration){


	comn::AutoCritSec lock(m_cs);

	return getTimelineDurationInternal(actual,duration);
}
void VEConfig::enableVELog(bool enable){

	veLogEnable = m_veLogEnable = enable?1:0;

	if(m_veLogEnable){
#ifdef __ANDROID__
		av_log_set_callback(veLogCallbackFfmpeg);
#endif
	}else{
		av_log_set_callback(NULL);
	}

}
VE_ERR VEConfig::addTrack(ve_track *track){
	comn::AutoCritSec lock(m_cs);

	int i,j;
	
	if(track->track_id < 0 || m_tracks.find(track->track_id) != m_tracks.end() && m_tracks.size()){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"track->track_id < 0 || m_tracks.find(track->track_id) != m_tracks.end() && m_tracks.size()");
		return VE_ERR_INPUT_PARAM;
	}
    
	if(m_tracks.size() >= VE_MAX_TRACK_NUM){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_tracks.size() >= VE_MAX_TRACK_NUM");
		return VE_ERR_INPUT_PARAM;
	}

	VETrackData trackData;


	trackData.m_track = *track;

	m_tracks[track->track_id] = trackData;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"track->track_id=%d,track->type=%d,track->speed=%f,track->volume=%d",track->track_id,track->type,track->speed,track->volume);

	return VE_ERR_OK;
}
VE_ERR VEConfig::delTrack(int trackId){
	comn::AutoCritSec lock(m_cs);


	VETrackData* trackData;
	VEFilterData* filterData;
	VEClipData* clipData;

	std::map<int,VETrackData>::iterator it = m_tracks.find(trackId);

	if(it == m_tracks.end() && m_tracks.size()){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it == m_tracks.end() && m_tracks.size()");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;

	m_tracks.erase(trackId);

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"trackId=%d",trackId);

	return VE_ERR_OK;
}
VE_ERR VEConfig::modTrack(ve_track *track){
	comn::AutoCritSec lock(m_cs);

	int i,j;

	VETrackData* trackData;

	if(track->track_id < 0 || m_tracks.find(track->track_id) == m_tracks.end() && m_tracks.size()){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"track->track_id < 0 || m_tracks.find(track->track_id) == m_tracks.end() && m_tracks.size()");
		return VE_ERR_INPUT_PARAM;
	}
    if(m_tracks[track->track_id].m_track.type != track->type || m_tracks[track->track_id].m_track.clip_arrangement != track->clip_arrangement){//} || track->type != VE_MULTITRACK){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_tracks[track->track_id].m_track.type != track->type || m_tracks[track->track_id].m_track.clip_arrangement != track->clip_arrangement");
		return VE_ERR_INPUT_PARAM;
	}
    VE_LOG_TAG_INFO(VE_MODULE_TAG,"track->track_id=%d,track->type=%d,track->speed=%f,track->volume=%d",track->track_id,track->type,track->speed,track->volume);

	trackData = &m_tracks[track->track_id];

	if(track->speed > trackData->m_track.speed && invalidSpeedForTrack(trackData,track->speed)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"track->speed > trackData->m_track.speed && invalidSpeedForTrack(trackData,track->speed)");
		return VE_ERR_INPUT_PARAM;
	}

	trackData->m_track = *track;
	return VE_ERR_OK;
}
VE_ERR VEConfig::getTrack(int trackId,ve_track *track){

	comn::AutoCritSec lock(m_cs);

	VETrackData* trackData;

	if(m_tracks.find(trackId) == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_tracks.find(track_id) == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &m_tracks[trackId];

	*track = trackData->m_track;

	return VE_ERR_OK;
}

VE_ERR VEConfig::insertClip(ve_clip *clip,int index){

	comn::AutoCritSec lock(m_cs);

	int i;
	int size;
	int actualVolume;
	float actualSpeed;
	int fromIndex,toIndex;

	VETrackData* trackData;
	VEClipData* clipData;


	if(m_tracks.find(clip->track_id) == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0 || clip->clip_id < 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_tracks.find(clip->track_id) == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0 || clip->clip_id < 0");
		return VE_ERR_INPUT_PARAM;
	}

	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	for(;it != m_tracks.end();it++){
		trackData = &it->second;

		size = trackData->m_clips.size();

		for(i=0;i<size;i++){
			clipData = &trackData->m_clips[i];
			if(clipData->m_clip.clip_id == clip->clip_id){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clipData->m_clip.clip_id == clip->clip_id");
				return VE_ERR_INPUT_PARAM;
			}
		}
	}

	trackData = &m_tracks[clip->track_id];
	size = trackData->m_clips.size();

	if(trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_SEQUENCE){
		if(index < 0){
			index = 0;
		}
		if(index >= size){
			index = size;
		}
	}else{
		if(index >= 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"index >= 0 && trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY");
			return VE_ERR_INPUT_PARAM;
		}
		index = size;

		//判断是否时间重叠
		if(clipOverlay(trackData,clip)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clipOverlay(trackData,clip) == true");
			return VE_ERR_INPUT_PARAM;
		}
	}


	if(!clip->filename){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!clip->filename");
		return VE_ERR_INPUT_PARAM;
	}

	ve_clip_info clipInfo;

	if(clip->slv.active){
		VESource::getSourceInfo(clip->filename,&clipInfo,&clip->slv);
	}else{
		VESource::getSourceInfo(clip->filename,&clipInfo);
	}


	//audio
	if(trackData->m_track.type == VE_TRACK_AUDIO && (!clipInfo.a_codec_id || clip->type != VE_CLIP_AUDIO)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.type == VE_TRACK_AUDIO && (!clipInfo.a_codec_id || clip->type != VE_CLIP_AUDIO)");
        return VE_ERR_INPUT_PARAM;
	}

	int totalDuration;
	this->getTimelineDurationInternal(1,&totalDuration);

	if(trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY && clip->insert_time < 0){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY && clip->insert_index < 0");
        return VE_ERR_INPUT_PARAM;
	}

	if(clip->type == VE_CLIP_AUDIO  && ((int)(clip->duration / clip->speed)) > totalDuration){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clip->type == VE_CLIP_AUDIO  && ((int)(clip->duration / clip->speed)) > totalDuration");
        return VE_ERR_INPUT_PARAM;
	}
	//picture && video
	if(trackData->m_track.type == VE_TRACK_VIDEO && (!clipInfo.v_codec_id || (clip->type != VE_CLIP_VIDEO && clip->type != VE_CLIP_PICTURE))){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.type == VE_TRACK_VIDEO && (!clipInfo.v_codec_id || (clip->type != VE_CLIP_VIDEO && clip->type != VE_CLIP_PICTURE))");
        return VE_ERR_INPUT_PARAM;
	}
	//picture
	if(clip->type == VE_CLIP_PICTURE && (!clipInfo.picture || !clip->duration)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clip->type == VE_CLIP_PICTURE && (!clipInfo.picture || !clip->duration)");
        return VE_ERR_INPUT_PARAM;
	}

	//start_time && end_time
	if((clip->type == VE_CLIP_AUDIO || clip->type == VE_CLIP_VIDEO) && (clip->start_time < 0 || clip->start_time >= clipInfo.duration || clip->end_time > clipInfo.duration || clip->end_time <= clip->start_time || clip->duration <= 0)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"(clip->type == VE_CLIP_AUDIO || clip->type == VE_CLIP_VIDEO) && (clip->start_time(%d) < 0 || clip->start_time(%d) >= clipInfo.duration(%d) || clip->end_time(%d) > clipInfo.duration(%d) || clip->end_time(%d) <= clip->start_time(%d) || clip->duration(%d) <= 0)",clip->start_time,clip->start_time,clipInfo.duration,clip->end_time,clipInfo.duration,clip->end_time,clip->start_time,clip->duration);
        return VE_ERR_INPUT_PARAM;
	}

	//volume
	if(invalidVolume(trackData,clip,&actualVolume)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalidVolume(trackData,clip),m_volume=%d,trackData->m_track.volume=%d,clip->volume=%d",m_volume,trackData->m_track.volume,clip->volume);
        return VE_ERR_INPUT_PARAM;
	}

	//slv
	if(invalidSlvInfo(&clip->slv)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalidSlvInfo(&clip->slv)");
        return VE_ERR_INPUT_PARAM;
	}

	if((clip->type == VE_CLIP_VIDEO || clip->type == VE_CLIP_AUDIO) && clip->duration != (clip->end_time - clip->start_time)){
		clip->duration = clip->end_time - clip->start_time;
	}


	VEClipData tempClipData;
	clipData = &tempClipData;



	clipData->m_clip = *clip;
    clipData->m_clip.info = clipInfo;
	clipData->m_filename = clip->filename;
	clipData->m_originalFilename = clip->original_filename?clip->original_filename:"";
	clipData->m_clip.info.filename = clipData->m_clip.filename = clipData->m_filename.c_str();



	clipData->m_actualVolume = actualVolume;

	if(clipData->m_clip.slv.active){
		clipData->m_clip.slv.clip_start_time = getSlvOriginalStartTime(&clipData->m_clip.slv,clipData->m_clip.start_time);
		clipData->m_clip.slv.clip_end_time = getSlvOriginalStartTime(&clipData->m_clip.slv,clipData->m_clip.end_time);
		clipData->m_clip.slv.clip_speed = actualSpeed;
	}


	clipData->m_clipArrangement = trackData->m_track.clip_arrangement;

	if(clipData->m_clip.type == VE_CLIP_PICTURE && clipData->m_clip.duration && clipData->m_clip.picture_rotate){
		clipData->m_clip.info.rotate = clipData->m_clip.picture_rotate;
	}

	//speed
	if(invalidSpeed(trackData,clipData,-1,&actualSpeed)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalidSpeed(trackData,clip),m_speed=%f,trackData->m_track.speed=%f,clip->speed=%f",m_speed,trackData->m_track.speed,clip->speed);
        return VE_ERR_INPUT_PARAM;
	}
	clipData->m_actualSpeed = actualSpeed;

	insertClip(trackData->m_clips,clipData,index);

	fromIndex = index;
	toIndex = trackData->m_clips.size() - 1;

	for(;fromIndex <= toIndex;fromIndex++){
		delTransition(trackData,fromIndex);
	}



	VE_LOG_TAG_INFO(VE_MODULE_TAG,"clip->clip_id=%d,clip->track_id=%d,clip->type=%d,clip->duration=%d,clip->start_time=%d,clip->end_time=%d,clip->picture_rotate=%d,clip->volume=%d,clip->speed=%f,clipData->m_filename.c_str()=%s,clipData->m_originalFilename.c_str()=%s",clip->clip_id,clip->track_id,clip->type,clip->duration,clip->start_time,clip->end_time,clip->picture_rotate,clip->volume,clip->speed,clipData->m_filename.c_str(),clipData->m_originalFilename.c_str());


	return VE_ERR_OK;
}

VE_ERR VEConfig::modClip(ve_clip *clip){
	comn::AutoCritSec lock(m_cs);

	int i;
	int size;
	int actualVolume;
	float actualSpeed;
	int clipIndex = -1;

	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL,clipDataTemp;
	int found = 0;

	if(m_tracks.find(clip->track_id) == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0 || clip->clip_id < 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_tracks.find(clip->track_id) == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0  || clip->clip_id < 0");
		return VE_ERR_INPUT_PARAM;
	}


	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	for(;it != m_tracks.end();it++){
		trackData = &it->second;

		size = trackData->m_clips.size();

		for(i=0;i<size;i++){
			clipData = &trackData->m_clips[i];
			if(clipData->m_clip.clip_id == clip->clip_id){
				found = 1;
				clipIndex = i;
				break;
			}
			clipData = NULL;
		}
		if(found)break;
	}
	if(!clipData){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!clipData");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &m_tracks[clip->track_id];
	size = trackData->m_clips.size();



	if(trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_SEQUENCE){
		//判断是否时间重叠
		if(clipOverlay(trackData,clip)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clipOverlay(trackData,clip) == true");
			return VE_ERR_INPUT_PARAM;
		}
	}

	if(!clip->filename){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!clip->filename");
		return VE_ERR_INPUT_PARAM;
	}

	ve_clip_info clipInfo;
	if(clipData->m_filename.compare(clip->filename)){

		if(clip->slv.active){
			VESource::getSourceInfo(clip->filename,&clipInfo,&clip->slv);
		}else{
			VESource::getSourceInfo(clip->filename,&clipInfo);
		}
	}else{
		clipInfo = clipData->m_clip.info;
	}

	//audio
	if(trackData->m_track.type == VE_TRACK_AUDIO && (!clipInfo.a_codec_id || clip->type != VE_CLIP_AUDIO)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.type == VE_TRACK_AUDIO && (!clipInfo.a_codec_id || clip->type != VE_CLIP_AUDIO)");
        return VE_ERR_INPUT_PARAM;
	}

	int totalDuration;
	this->getTimelineDurationInternal(1,&totalDuration);


	if(trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY && clip->insert_time < 0){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY && clip->insert_index < 0");
        return VE_ERR_INPUT_PARAM;
	}

	if(clip->type == VE_CLIP_AUDIO  && ((int)(clip->duration / clip->speed)) > totalDuration){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clip->type == VE_CLIP_AUDIO  && ((int)(clip->duration / clip->speed)) > totalDuration");
        return VE_ERR_INPUT_PARAM;
	}

	//picture && video
	if(trackData->m_track.type == VE_TRACK_VIDEO && (!clipInfo.v_codec_id || (clip->type != VE_CLIP_VIDEO && clip->type != VE_CLIP_PICTURE))){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.type == VE_TRACK_VIDEO && (!clipInfo.v_codec_id || (clip->type != VE_CLIP_VIDEO && clip->type != VE_CLIP_PICTURE))");
        return VE_ERR_INPUT_PARAM;
	}
	//picture
	if(clip->type == VE_CLIP_PICTURE && (!clipInfo.picture || !clip->duration)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clip->type == VE_CLIP_PICTURE && (!clipInfo.picture || !clip->duration)");
        return VE_ERR_INPUT_PARAM;
	}
	if((clip->type == VE_CLIP_PICTURE) && (clip->start_time || clip->end_time)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"(clip->type == VE_CLIP_PICTURE) && (clip->start_time || clip->end_time)");
        return VE_ERR_INPUT_PARAM;
	}
	//start_time && end_time
	if((clip->type == VE_CLIP_AUDIO || clip->type == VE_CLIP_VIDEO) && (clip->start_time < 0 || clip->start_time >= clipInfo.duration || clip->end_time > clipInfo.duration || clip->end_time <= clip->start_time || clip->duration <= 0)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"(clip->type == VE_CLIP_AUDIO || clip->type == VE_CLIP_VIDEO) && (clip->start_time(%d) < 0 || clip->start_time(%d) >= clipInfo.duration(%d) || clip->end_time(%d) > clipInfo.duration(%d) || clip->end_time(%d) <= clip->start_time(%d) || clip->duration(%d) <= 0)",clip->start_time,clip->start_time,clipInfo.duration,clip->end_time,clipInfo.duration,clip->end_time,clip->start_time,clip->duration);
        return VE_ERR_INPUT_PARAM;
	}

	//volume
	if(invalidVolume(trackData,clip,&actualVolume)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalidVolume(trackData,clip),m_volume=%d,trackData->m_track.volume=%d,clip->volume=%d",m_volume,trackData->m_track.volume,clip->volume);
        return VE_ERR_INPUT_PARAM;
	}
	clipDataTemp = *clipData;
	clipDataTemp.m_clip.speed = clip->speed;
	//speed
	if(invalidSpeed(trackData,&clipDataTemp,clipIndex,&actualSpeed)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalidSpeed(trackData,clip),m_speed=%f,trackData->m_track.speed=%f,clip->speed=%f",m_speed,trackData->m_track.speed,clip->speed);
        return VE_ERR_INPUT_PARAM;
	}
	//slv
	if(invalidSlvInfo(&clip->slv)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalidSlvInfo(&clip->slv)");
        return VE_ERR_INPUT_PARAM;
	}

	if((clip->type == VE_CLIP_VIDEO || clip->type == VE_CLIP_AUDIO) && clip->duration && clip->duration < (clip->end_time - clip->start_time)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"(clip->type == VE_CLIP_VIDEO || clip->type == VE_CLIP_AUDIO) && clip->duration && clip->duration < (clip->end_time - clip->start_time)");
        return VE_ERR_INPUT_PARAM;
	}
	if((clip->type == VE_CLIP_VIDEO || clip->type == VE_CLIP_AUDIO) && clip->duration != (clip->end_time - clip->start_time)){
		clip->duration = clip->end_time - clip->start_time;
	}


	clipData->m_clip = *clip;
    clipData->m_clip.info = clipInfo;
	clipData->m_filename = clip->filename;
	clipData->m_originalFilename = clip->original_filename?clip->original_filename:"";
	clipData->m_clip.info.filename = clipData->m_clip.filename = clipData->m_filename.c_str();
	clipData->m_actualSpeed = actualSpeed;
	clipData->m_actualVolume = actualVolume;

	if(clipData->m_clip.slv.active){
		clipData->m_clip.slv.clip_start_time = getSlvOriginalStartTime(&clipData->m_clip.slv,clipData->m_clip.start_time);
		clipData->m_clip.slv.clip_end_time = getSlvOriginalStartTime(&clipData->m_clip.slv,clipData->m_clip.end_time);
		clipData->m_clip.slv.clip_speed = actualSpeed;
	}

	clipData->m_clipArrangement = trackData->m_track.clip_arrangement;

	if(clipData->m_clip.type == VE_CLIP_PICTURE && clipData->m_clip.picture_rotate){
		clipData->m_clip.info.rotate = clipData->m_clip.picture_rotate;
	}

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"clip->clip_id=%d,clip->track_id=%d,clip->type=%d,clip->duration=%d,clip->start_time=%d,clip->end_time=%d,clip->picture_rotate=%d,clip->volume=%d,clip->speed=%f,clipData->m_filename.c_str()=%s,clipData->m_originalFilename.c_str()=%s",clip->clip_id,clip->track_id,clip->type,clip->duration,clip->start_time,clip->end_time,clip->picture_rotate,clip->volume,clip->speed,clipData->m_filename.c_str(),clipData->m_originalFilename.c_str());

	return VE_ERR_OK;
}
VE_ERR VEConfig::moveClip(int trackId,int from,int to){

	comn::AutoCritSec lock(m_cs);

	int size;
	int fromIndex,toIndex;
	VETrackData* trackData = NULL;
	VEClipData clipData;

	std::map<int,VETrackData>::iterator it = m_tracks.find(trackId);

	if(m_tracks.size() && it != m_tracks.end()){
		trackData = &it->second;

		if(trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY){
			return VE_ERR_INPUT_PARAM;
		}
		size = trackData->m_clips.size();

		if(from <= 0){
			from = 0;
		}
		if(from >= size){
			from = size - 1;
		}
		if(to <= 0){
			to = 0;
		}
		if(to >= size){
			to = size - 1;
		}

		if( from <= to){
			fromIndex = from;
			toIndex = to;
		}else{
			fromIndex = to;
			toIndex = from;
		}

		clipData = trackData->m_clips[from];

		trackData->m_clips.erase(trackData->m_clips.begin() + from);
		trackData->m_clips.insert(trackData->m_clips.begin() + to,clipData);

		for(;fromIndex <= toIndex;fromIndex++){
			delTransition(trackData,fromIndex);
		}

		VE_LOG_TAG_INFO(VE_MODULE_TAG,"trackId=%d,from=%d,to=%d",trackId,from,to);
		return VE_ERR_OK;
	}else{
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"invalid trackId=%d",trackId);
        return VE_ERR_INPUT_PARAM;
	}

}
VE_ERR VEConfig::delClip(int clipId){

	comn::AutoCritSec lock(m_cs);

	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;
	int i,size;
	int found = 0;
	int fromIndex,toIndex;

	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"clipId=%d",clipId);

	for(;it != m_tracks.end();it++){
		trackData = &it->second;
		size = trackData->m_clips.size();

		for(i=0;i<size;i++){
			clipData = &trackData->m_clips[i];

			if(clipData->m_clip.clip_id == clipId){
				found = 1;
				break;
			}
		}
		if(found)break;
	}

	if(found){

		trackData->m_clips.erase(trackData->m_clips.begin() + i);

		fromIndex = i;
		toIndex = size - 1;
		for(;fromIndex <= toIndex;fromIndex++){
			delTransition(trackData,fromIndex);
		}
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"clipId=%d",clipId);
		return VE_ERR_OK;
	}

	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"clipId not found");

	return VE_ERR_INPUT_PARAM;
}
VE_ERR VEConfig::delClip(int trackId,int index){

	comn::AutoCritSec lock(m_cs);

	int size;
	int fromIndex,toIndex;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.find(trackId);

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"trackId=%d,index=%d",trackId,index);

	if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_tracks.find(trackId) == m_tracks.end()");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;

	size = trackData->m_clips.size();

	if(index < 0 || index > (size -1)){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"index < 0 || index > (size -1)");
		return VE_ERR_INPUT_PARAM;
	}

	if(trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_SEQUENCE){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_SEQUENCE");
		return VE_ERR_INPUT_PARAM;
	}
	clipData = &trackData->m_clips[index];

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"clip_id=%d,track_id=%d,index=%d",clipData->m_clip.clip_id,trackId,index);

	trackData->m_clips.erase(trackData->m_clips.begin() + index);

	fromIndex = index;
	toIndex = size - 1;
	for(;fromIndex <= toIndex;fromIndex++){
		delTransition(trackData,fromIndex);
	}

	return VE_ERR_OK;
}
VE_ERR VEConfig::getClip(int clipId,ve_clip *clip){

	comn::AutoCritSec lock(m_cs);

	int size,i;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	if(m_tracks.size()){
		for(;it != m_tracks.end();it++){

			trackData = &it->second;

			size = trackData->m_clips.size();

			for(i=0;i<size;i++){
				clipData = &trackData->m_clips[i];
				if(clipData->m_clip.clip_id == clipId){
                    clipData->m_clip.filename = clipData->m_filename.c_str();
                    clipData->m_clip.original_filename = clipData->m_originalFilename.c_str();
					*clip = clipData->m_clip;
					return VE_ERR_OK;
				}
			}
		}
	}
	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"not found clipId=%d",clipId);
	return VE_ERR_INPUT_PARAM;
}

VE_ERR VEConfig::getClip(int trackId,int index,ve_clip *clip){

	comn::AutoCritSec lock(m_cs);

	int size,i;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.find(trackId);

	if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it == m_tracks.end()");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;

	size = trackData->m_clips.size();

	if(index < 0 || index > size -1){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"index < 0 || index > size -1");
		return VE_ERR_INPUT_PARAM;
	}
	if(trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_SEQUENCE){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_SEQUENCE");
		return VE_ERR_INPUT_PARAM;
	}

	clipData = &trackData->m_clips[index];

    clipData->m_clip.filename = clipData->m_filename.c_str();
    clipData->m_clip.original_filename = clipData->m_originalFilename.c_str();
	*clip = clipData->m_clip;

	return VE_ERR_OK;
}

VE_ERR VEConfig::getClipDuration(int clipId,int* duration){

	comn::AutoCritSec lock(m_cs);

	int size,i;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	if(m_tracks.size()){
		for(;it != m_tracks.end();it++){

			trackData = &it->second;

			size = trackData->m_clips.size();

			for(i=0;i<size;i++){
				clipData = &trackData->m_clips[i];
				if(clipData->m_clip.clip_id == clipId){
					*duration = getClipDuration(clipData);
					return VE_ERR_OK;
				}
			}
		}
	}
	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"not found clipId=%d",clipId);
	return VE_ERR_INPUT_PARAM;
}
VE_ERR VEConfig::getClipsCount(int trackId,int *count){

	comn::AutoCritSec lock(m_cs);

	int size,i;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.find(trackId);

	if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		*count = 0;
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;

	*count = trackData->m_clips.size();

	return VE_ERR_OK;
}
VE_ERR VEConfig::getClips(int trackId,ve_clip*  clips,int len){

	comn::AutoCritSec lock(m_cs);

	int size,i;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.find(trackId);

	if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_tracks.size() || m_tracks.size() == 0");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;

	size = trackData->m_clips.size();

	if(len != size){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"len != size");
		return VE_ERR_INPUT_PARAM;
	}
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"track_id=%d",trackData->m_track.track_id);
	for(i=0;i<len;i++){
		clipData = &trackData->m_clips[i];
		clips[i] = clipData->m_clip;
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"clips[%d]->clip_id=%d,clips[%d]->end_time=%d",i,clips[i].clip_id,i,clips[i].end_time);
	}
	return VE_ERR_OK;
}
VE_ERR VEConfig::getTrackDuration(int trackId,int index,int actual,int *duration){
	comn::AutoCritSec lock(m_cs);
	return getTrackDurationInternal(trackId,index,actual,duration);
}
VE_ERR VEConfig::getTrackDurationInternal(int trackId,int index,int actual,int *duration){



	int size,i;
	int totalDuration = 0;
	int maxDuration = 0;
	int curDuration = 0;
	int transitionDuraion = 0,transitionDuraion1 = 0,transitionDuraion2;

	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;

	float factor = 1;

	std::map<int,VETrackData>::iterator it = m_tracks.find(trackId);

	*duration = 0;

	if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;

	size = trackData->m_clips.size();

	if(trackData->m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_SEQUENCE){

		if(index > size){
			index = size;
		}else if(index <= size - 1 && index >= 0){
			size = index + 1;
		}

		for(i=0;i<size;i++){
				clipData = &trackData->m_clips[i];


				factor = 1.0;
				if(actual){
					factor = clipData->m_clip.speed * m_speed * trackData->m_track.speed;

					transitionDuraion += this->transitionDuration(trackData,i);

				}

				if(clipData->m_clip.type == VE_CLIP_PICTURE){
					totalDuration += (clipData->m_clip.duration)  /  factor;
				}else{
					totalDuration += (clipData->m_clip.end_time - clipData->m_clip.start_time) / factor;
				}

		}
		totalDuration -= transitionDuraion;
	}else{
		for(i=0;i<size;i++){
			clipData = &trackData->m_clips[i];

			if(actual){
				factor = clipData->m_clip.speed * m_speed * trackData->m_track.speed;
			}
			if(clipData->m_clip.type == VE_CLIP_PICTURE){
				curDuration = clipData->m_clip.duration  /  factor;
			}else{
				curDuration = (clipData->m_clip.end_time - clipData->m_clip.start_time) / factor;
			}
            curDuration += clipData->m_clip.insert_time;
            
			if(curDuration > maxDuration){
				maxDuration = curDuration;
			}
		}
		totalDuration = maxDuration;
	}

	*duration = totalDuration;

	//TODO
	return VE_ERR_OK;
}
VE_ERR VEConfig::getTrackDuration(int trackId,int actual,int *duration){

	return getTrackDurationInternal(trackId,256,actual,duration);

}


VE_ERR VEConfig::addTransition(ve_transition *transition){
	comn::AutoCritSec lock(m_cs);

	VEClipData* clipData = NULL;

	VETrackData* trackData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.find(transition->track_id);

	

	std::map<int,VETransitionData>::iterator it2;

	int durationA,durationB;

	if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;

	if(trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_SEQUENCE){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_SEQUENCE");
		return VE_ERR_INPUT_PARAM;
	}
    
    if(transition->clip_index_b >= trackData->m_clips.size()){
    	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transition->clip_index_b >= trackData->m_clips.size()");
        return VE_ERR_INPUT_PARAM;
    }

    if(transitionHasIndexB(trackData,transition->clip_index_b)){
    	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transitionHasIndexB(trackData,transition->clip_index_b)");
        return VE_ERR_INPUT_PARAM;
    }

    
	it2 = trackData->m_transitions.find(transition->transition_id);

	if(it2 != trackData->m_transitions.end() && trackData->m_transitions.size() || transition->transition_id < 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it2 != trackData->m_transitions.end() || trackData->m_transitions.size() == 0 || transition->transition_id < 0");
		return VE_ERR_INPUT_PARAM;
	}
	if(!transition->action){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!transition->action");
		return VE_ERR_INPUT_PARAM;
	}

	if(transition->clip_index_b <= 0 || transition->clip_index_b >= trackData->m_clips.size()){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transition->clip_index_b <= 0 || transition->clip_index_b >= trackData->m_clips.size()");
		return VE_ERR_INPUT_PARAM;
	}

	clipData = &trackData->m_clips[transition->clip_index_b - 1];

	durationA = getClipDuration(clipData);

	clipData = &trackData->m_clips[transition->clip_index_b];

	durationB = getClipDuration(clipData);


	float transitionSpeed = m_speed * trackData->m_track.speed * clipData->m_clip.speed;
	int transitionDuration = transition->duration / transitionSpeed;

	if(transitionDuration > durationA || transitionDuration > durationB){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transitionDuration > durationA || transitionDuration > durationB");
		return VE_ERR_INPUT_PARAM;
	}

    VETransitionData transitionData;



	transitionData.m_action = transition->action;
	transitionData.m_transition = *transition;
	transitionData.m_transition.action = transitionData.m_action.c_str();
	trackData->m_transitions[transition->transition_id] = transitionData;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"transition->track_id=%d,transition->transition_id=%d,transition->clip_index_b=%d,transition->duration=%d",transition->track_id,transition->transition_id,transition->clip_index_b,transition->duration);

	return VE_ERR_OK;
}
VE_ERR VEConfig::modTransition(ve_transition *transition){

	comn::AutoCritSec lock(m_cs);

	VEClipData* clipData = NULL;

	VETrackData* trackData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.find(transition->track_id);

	VETransitionData* transitionData = NULL;

	std::map<int,VETransitionData>::iterator it2;

	int durationA,durationB;

	if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
		return VE_ERR_INPUT_PARAM;
	}

	trackData = &it->second;


	it2 = trackData->m_transitions.find(transition->transition_id);

	if(it2 == trackData->m_transitions.end() && trackData->m_transitions.size() || trackData->m_transitions.size() == 0 || transition->transition_id < 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"it2 == trackData->m_transitions.end() && trackData->m_transitions.size() || trackData->m_transitions.size() == 0 || transition->transition_id < 0");
		return VE_ERR_INPUT_PARAM;
	}
    
	transitionData = &it2->second;

	if(transitionData->m_transition.clip_index_b != transition->clip_index_b && transitionHasIndexB(trackData,transition->clip_index_b)){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transitionData->m_transition.clip_index_b != transition->clip_index_b && transitionHasIndexB(trackData,transition->clip_index_b)");
        return VE_ERR_INPUT_PARAM;
	}

    if(transition->clip_index_b >= trackData->m_clips.size()){
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transition->clip_index_b >= trackData->m_clips.size()");
        return VE_ERR_INPUT_PARAM;
    }
    
	if(!transition->action){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!transition->action");
		return VE_ERR_INPUT_PARAM;
	}

	if(transition->clip_index_b <= 0 || transition->clip_index_b >= trackData->m_clips.size()){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transition->clip_index_b <= 0 || transition->clip_index_b >= trackData->m_clips.size()");
		return VE_ERR_INPUT_PARAM;
	}

	clipData = &trackData->m_clips[transition->clip_index_b - 1];

	durationA = getClipDuration(clipData);

	clipData = &trackData->m_clips[transition->clip_index_b];

	durationB = getClipDuration(clipData);

	float transitionSpeed = m_speed * trackData->m_track.speed * clipData->m_clip.speed;
	int transitionDuration = transition->duration / transitionSpeed;

	if(transitionDuration > durationA || transitionDuration > durationB){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transitionDuration > durationA || transitionDuration > durationB");
		return VE_ERR_INPUT_PARAM;
	}

	transitionData->m_action = transition->action;
	transitionData->m_transition = *transition;
	transitionData->m_transition.action = transitionData->m_action.c_str();

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"transition->track_id=%d,transition->transition_id=%d,transition->clip_index_b=%d,transition->duration=%d",transition->track_id,transition->transition_id,transition->clip_index_b,transition->duration);

	return VE_ERR_OK;
}
VE_ERR VEConfig::delTransition(int transitionId){

	comn::AutoCritSec lock(m_cs);

	VEClipData* clipData = NULL;

	VETrackData* trackData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	VETransitionData* transitionData = NULL;

	std::map<int,VETransitionData>::iterator it2;

	int found = 0;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"transitionId=%d",transitionId);


	for(;it != m_tracks.end();it++){
		trackData = &it->second;
		it2 = trackData->m_transitions.find(transitionId);
		if(it2 == trackData->m_transitions.end() && trackData->m_transitions.size() || trackData->m_transitions.size() == 0){
			continue;
		}

		found = 1;
		trackData->m_transitions.erase(transitionId);
		break;

	}

	if(found){
		return VE_ERR_OK;
	}else{
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transitionId not found");
		return VE_ERR_INPUT_PARAM;
	}
}
VE_ERR VEConfig::getTransition(int transitionId,ve_transition *transition){

	comn::AutoCritSec lock(m_cs);

	VEClipData* clipData = NULL;

	VETrackData* trackData = NULL;

	std::map<int,VETrackData>::iterator it = m_tracks.begin();

	VETransitionData* transitionData = NULL;

	std::map<int,VETransitionData>::iterator it2;

	int found = 0;


	for(;it != m_tracks.end();it++){
		trackData = &it->second;
		it2 = trackData->m_transitions.find(transitionId);
		if(it2 == trackData->m_transitions.end() && trackData->m_transitions.size() || trackData->m_transitions.size() == 0){
			continue;
		}

		found = 1;
		transitionData = &it2->second;
		*transition = transitionData->m_transition;
		break;

	}
	if(found){
		return VE_ERR_OK;
	}else{
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"transitionId not found");
		return VE_ERR_INPUT_PARAM;
	}
}


VE_ERR VEConfig::addFilter(ve_filter *filter){

	comn::AutoCritSec lock(m_cs);

	std::map<int,VETrackData>::iterator it;

	VETrackData* trackData = NULL;

	std::map<int,VEFilterData>::iterator it2;

	
	VEClipData* clipData = NULL;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"filter->filter_id=%d,filter->track_id=%d,filter->loc_type=%d,filter->type=%d,filter->clip_id=%d,filter->clip_index=%d,filter->start_time=%d,filter->end_time=%d,filter->af_type=%d,filter->fade_curve=%d",filter->filter_id,filter->track_id,filter->loc_type,filter->type,filter->clip_id,filter->clip_index,filter->start_time,filter->end_time,filter->af_type,filter->fade_curve);

	if(filter->filter_id < 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->filter_id < 0");
		return VE_ERR_INPUT_PARAM;
	}

	if(filter->start_time > filter->end_time){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->start_time >= filter->end_time");
		return VE_ERR_INPUT_PARAM;
	}


	if(filter->loc_type == VE_FILTER_LOC_TIMELINE){
		if(this->filterExists(m_filters,filter->filter_id)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"this->filterExists(m_filters,filter->filter_id=%d)",filter->filter_id);
			return VE_ERR_INPUT_PARAM;
		}
	}
	if(filter->loc_type == VE_FILTER_LOC_TRACK){

		it = m_tracks.find(filter->track_id);

		if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"VE_FILTER_LOC_TRACK it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
			return VE_ERR_INPUT_PARAM;
		}

		trackData = &it->second;

		if(this->filterExists(trackData->m_filters,filter->filter_id)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"this->filterExists(trackData->m_filters,filter->filter_id)");
			return VE_ERR_INPUT_PARAM;
		}

	}


	if(filter->type == VE_FILTER_AUDIO && filter->loc_type != VE_FILTER_LOC_CLIP){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->type == VE_FILTER_AUDIO && filter->loc_type != VE_FILTER_LOC_CLIP");
		return VE_ERR_INPUT_PARAM;
	}
	if(filter->type == VE_FILTER_VIDEO && trackData  && trackData->m_track.type != VE_TRACK_VIDEO){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->type == VE_FILTER_VIDEO && trackData  && trackData->m_track.type != VE_TRACK_VIDEO");
		return VE_ERR_INPUT_PARAM;
	}

	if(filter->loc_type == VE_FILTER_LOC_CLIP){

		it = m_tracks.find(filter->track_id);

		if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"VE_FILTER_LOC_CLIP it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
			return VE_ERR_INPUT_PARAM;
		}

		trackData = &it->second;

		if(filter->clip_id < 0 && filter->clip_index < 0 || filter->clip_id >= 0 && filter->clip_index >= 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->clip_id < 0 && filter->clip_index < 0 || filter->clip_id >= 0 && filter->clip_index >= 0");
			return VE_ERR_INPUT_PARAM;
		}

		if(filter->clip_index >= 0 && filter->clip_index >= trackData->m_clips.size()){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->clip_index >= 0 && filter->clip_index >= trackData->m_clips.size()");
			return VE_ERR_INPUT_PARAM;
		}

		if( trackData->m_clips.size() == 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"trackData->m_clips.size() == 0 cannot find file filter");
			return VE_ERR_INPUT_PARAM;
		}


		if(filter->clip_id >= 0){

			int found = 0;

			std::deque<VEClipData>::iterator clip_it = trackData->m_clips.begin();

			for(;clip_it != trackData->m_clips.end();clip_it++){
				clipData = (VEClipData*)&(*clip_it);
				if(clipData->m_clip.clip_id == filter->clip_id){
					found = 1;
					break;
				}
			}
			if(!found){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"cannot find file filter");
				return VE_ERR_INPUT_PARAM;
			}
		}else if(filter->clip_index >= 0){
			clipData = &trackData->m_clips[filter->clip_index];
		}

		if(this->filterExists(clipData->m_filters,filter->filter_id)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"this->filterExists(clipData->m_filters,filter->filter_id)");
			return VE_ERR_INPUT_PARAM;
		}

	}

	if(!filter->action){
		filter->action = "";
	}



	if(filter->type == VE_FILTER_AUDIO && filter->af_type >= VE_AUDIO_FILTER_FADE_IN && filter->af_type <= VE_AUDIO_FILTER_FADE_OUT){

		if( filter->gain_min < 0 || filter->gain_min > 100 || filter->gain_max < 0 || filter->gain_max> 100){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->gain_start < 0 || filter->gain_start > 100 || filter->gain_end < 0 || filter->gain_end > 100");
			return VE_ERR_INPUT_PARAM;
		}

	}
    

    VEFilterData filterData;
    
	filterData.m_action = filter->action?filter->action:"";
	filterData.m_filter = *filter;
	filterData.m_filter.action = filterData.m_action.c_str();


	if(filter->loc_type == VE_FILTER_LOC_TRACK){
		trackData->m_filters[filter->filter_id] = filterData;
	}else if(filter->loc_type == VE_FILTER_LOC_TIMELINE){
		m_filters[filter->filter_id] = filterData;
	}else if(filter->loc_type == VE_FILTER_LOC_CLIP){
		clipData->m_filters[filter->filter_id] = filterData;
	}



	return VE_ERR_OK;
}
VE_ERR VEConfig::modFilter(ve_filter *filter){

	comn::AutoCritSec lock(m_cs);

	std::map<int,VETrackData>::iterator it;

	VETrackData* trackData = NULL;

	std::map<int,VEFilterData>::iterator it2;

	VEFilterData* filterData = NULL;
	VEClipData* clipData = NULL;

	if(filter->filter_id < 0){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->filter_id < 0");
		return VE_ERR_INPUT_PARAM;
	}

	if(filter->start_time >= filter->end_time){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->start_time >= filter->end_time");
		return VE_ERR_INPUT_PARAM;
	}


	if(filter->loc_type == VE_FILTER_LOC_TIMELINE){
		if(!this->filterExists(m_filters,filter->filter_id)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!this->filterExists(m_filters,filter->filter_id=%d)",filter->filter_id);
			return VE_ERR_INPUT_PARAM;
		}
	}
	if(filter->loc_type == VE_FILTER_LOC_TRACK){

		it = m_tracks.find(filter->track_id);


		if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"VE_FILTER_LOC_TRACK it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
			return VE_ERR_INPUT_PARAM;
		}

		trackData = &it->second;

		if(!this->filterExists(trackData->m_filters,filter->filter_id)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!this->filterExists(trackData->m_filters,filter->filter_id)");
			return VE_ERR_INPUT_PARAM;
		}

	}


	if(filter->type == VE_FILTER_AUDIO && filter->loc_type != VE_FILTER_LOC_CLIP){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->type == VE_FILTER_AUDIO && filter->loc_type != VE_FILTER_LOC_CLIP");
		return VE_ERR_INPUT_PARAM;
	}
	if(filter->type == VE_FILTER_VIDEO && trackData  && trackData->m_track.type != VE_TRACK_VIDEO){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->type == VE_FILTER_VIDEO && trackData  && trackData->m_track.type != VE_TRACK_VIDEO");
		return VE_ERR_INPUT_PARAM;
	}

	if(filter->loc_type == VE_FILTER_LOC_CLIP){

		it = m_tracks.find(filter->track_id);

		if(it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"VE_FILTER_LOC_CLIP it == m_tracks.end() && m_tracks.size() || m_tracks.size() == 0");
			return VE_ERR_INPUT_PARAM;
		}

		trackData = &it->second;

		if(filter->clip_id < 0 && filter->clip_index < 0 || filter->clip_id >= 0 && filter->clip_index >= 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->clip_id < 0 && filter->clip_index < 0 || filter->clip_id >= 0 && filter->clip_index >= 0");
			return VE_ERR_INPUT_PARAM;
		}

		if(filter->clip_index >= 0 && filter->clip_index >= trackData->m_clips.size()){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->clip_index >= 0 && filter->clip_index >= trackData->m_clips.size()");
			return VE_ERR_INPUT_PARAM;
		}


		if(filter->clip_id >= 0){

			int found = 0;

			std::deque<VEClipData>::iterator clip_it = trackData->m_clips.begin();

			for(;clip_it != trackData->m_clips.end();clip_it++){
				clipData = (VEClipData*)&(*clip_it);
				if(clipData->m_clip.clip_id == filter->clip_id){
					found = 1;
					break;
				}
			}
			if(!found){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"cannot find file filter");
				return VE_ERR_INPUT_PARAM;
			}
		}else if(filter->clip_index >= 0){
			clipData = &trackData->m_clips[filter->clip_index];
		}

		if(!this->filterExists(clipData->m_filters,filter->filter_id)){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!this->filterExists(clipData->m_filters,filter->filter_id)");
			return VE_ERR_INPUT_PARAM;
		}

	}

	if(!filter->action){
		filter->action = "";
	}

	if(filter->type == VE_FILTER_AUDIO && filter->af_type >= VE_AUDIO_FILTER_FADE_IN && filter->af_type <= VE_AUDIO_FILTER_FADE_OUT) {
		if(filter->gain_min < 0 || filter->gain_min > 100 || filter->gain_max < 0 || filter->gain_max > 100) {
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"filter->gain_start < 0 || filter->gain_start > 100 || filter->gain_end < 0 || filter->gain_end > 100");
			return VE_ERR_INPUT_PARAM;
		}
	}

	if(filter->loc_type == VE_FILTER_LOC_TRACK){
		filterData = &trackData->m_filters[filter->filter_id];
	}else if(filter->loc_type == VE_FILTER_LOC_TIMELINE){
		filterData = &m_filters[filter->filter_id];
	}else if(filter->loc_type == VE_FILTER_LOC_CLIP){

		filterData = &clipData->m_filters[filter->filter_id];
	}
	filterData->m_action = filter->action?filter->action:"";
	filterData->m_filter = *filter;
	filterData->m_filter.action = filterData->m_action.c_str();

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"filter->filter_id=%d,filter->track_id=%d,filter->loc_type=%d,filter->type=%d,filter->clip_id=%d,filter->clip_index=%d,filter->start_time=%d,filter->end_time=%d,filter->af_type=%d,filter->fade_curve=%d",filter->filter_id,filter->track_id,filter->loc_type,filter->type,filter->clip_id,filter->clip_index,filter->start_time,filter->end_time,filter->af_type,filter->fade_curve);
	return VE_ERR_OK;
}
VE_ERR VEConfig::delFilter(int filterId){

	comn::AutoCritSec lock(m_cs);

	VEFilterData* filterData = NULL;

	std::map<int,VETrackData>::iterator it;

	VETrackData* trackData = NULL;

	VEClipData* clipData = NULL;

	int index;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"filterId=%d",filterId);

	if(this->filterExists(m_filters,filterId)){
		m_filters.erase(filterId);
		return VE_ERR_OK;
	}


	it = m_tracks.begin();

	for(;it != m_tracks.end();it++){

		trackData = &it->second;

		if(this->filterExists(trackData->m_filters,filterId)){
			trackData->m_filters.erase(filterId);
			return VE_ERR_OK;
		}



		for(index = 0;index != trackData->m_clips.size();index++){
			clipData = &trackData->m_clips[index];

			if(this->filterExists(clipData->m_filters,filterId)){
				clipData->m_filters.erase(filterId);
				return VE_ERR_OK;
			}
		}


	}

	return VE_ERR_INPUT_PARAM;
}
VE_ERR VEConfig::getFilter(int filterId,ve_filter *filter){

	comn::AutoCritSec lock(m_cs);

	VEFilterData* filterData = NULL;

	std::map<int,VETrackData>::iterator it;

	VETrackData* trackData = NULL;

	std::map<int,VEClipData>::iterator it2;

	VEClipData* clipData = NULL;

	int index;


	if(this->filterExists(m_filters,filterId)){
		filterData = &m_filters[filterId];
		*filter = filterData->m_filter;
		return VE_ERR_OK;
	}


	it = m_tracks.begin();

	for(;it != m_tracks.end();it++){

		trackData = &it->second;

		if(this->filterExists(trackData->m_filters,filterId)){
			filterData = &trackData->m_filters[filterId];
			*filter = filterData->m_filter;
			return VE_ERR_OK;
		}


		for(index = 0;index != trackData->m_clips.size();index++){
			clipData = &trackData->m_clips[index];

			if(this->filterExists(clipData->m_filters,filterId)){
				filterData = &clipData->m_filters[filterId];
				*filter = filterData->m_filter;
				return VE_ERR_OK;
			}
		}


	}

	return VE_ERR_INPUT_PARAM;
}


bool VEConfig::filterExists(std::map<int,VEFilterData> & filterMap,int filterId){

	if(filterMap.size() && filterMap.find(filterId) != filterMap.end()){
		return true;
	}else{
		return false;
	}
}
bool VEConfig::transitionHasIndexB(VETrackData* trackData,int indexB){

	VETransitionData* transitionData = NULL;
	std::map<int,VETransitionData>::iterator it = trackData->m_transitions.begin();
	int size = trackData->m_transitions.size();

	if(size){
		for(;it != trackData->m_transitions.end();it++){
			transitionData = &it->second;
			if(transitionData->m_transition.clip_index_b == indexB){
				return true;
			}

		}
	}
	return false;
}
int VEConfig::transitionDuration(VETrackData* trackData,int indexB){


	VETransitionData* transitionData = NULL;
	std::map<int,VETransitionData>::iterator it = trackData->m_transitions.begin();
	int size = trackData->m_transitions.size();

	VEClipData * clipData = &trackData->m_clips[indexB];
	float transitionSpeed = m_speed * trackData->m_track.speed * clipData->m_clip.speed;
	if(size){
		for(;it != trackData->m_transitions.end();it++){
			transitionData = &it->second;
			if(transitionData->m_transition.clip_index_b == indexB){
				return transitionData->m_transition.duration / transitionSpeed;
			}

		}
	}
	return 0;
}

void VEConfig::delTransition(VETrackData* trackData,int indexB){

	VETransitionData* transitionData = NULL;
	std::map<int,VETransitionData>::iterator it = trackData->m_transitions.begin();
	int size = trackData->m_transitions.size();

	if(size){
		for(;it != trackData->m_transitions.end();it++){
			transitionData = &it->second;
			if(transitionData->m_transition.clip_index_b == indexB){
				break;
			}
			transitionData = NULL;
		}
		if(transitionData){
			trackData->m_transitions.erase(it);
			
		}
	}
    return;
}
bool VEConfig::clipOverlay(VETrackData* trackData,ve_clip* clip){

	VEClipData* curClip;
	float baseSpeed = this->m_speed * trackData->m_track.speed;
	float curSpeed,speed;

	int curDuration,curStartTime,curEndTime,duration,startTime,endTime;
	int index = 0,size = trackData->m_clips.size();

	if(trackData->m_track.track_id != clip->track_id){
		return false;
	}

	if(trackData->m_track.clip_arrangement != VE_CLIP_ARRANGEMENT_OVERLAY){
		return false;
	}

	for(;index < size;index++){

		curSpeed = speed = baseSpeed;
		curClip = &trackData->m_clips[index];
		curSpeed *=  curClip->m_clip.speed;
		curDuration = curClip->m_clip.duration / curSpeed;
		curStartTime = curClip->m_clip.insert_time;
		curEndTime = curStartTime + curDuration;

		if(clip->clip_id == curClip->m_clip.clip_id){
			continue;
		}

		speed *= clip->speed;
		duration = clip->duration / speed;
		startTime = clip->insert_time;
		endTime = startTime + duration;

		if(startTime >= curStartTime && startTime < curEndTime ||
				curStartTime >= startTime && curStartTime < endTime){
			return true;
		}
	}

	return false;

}

bool VEConfig::invalidVolume(VETrackData* trackData,ve_clip* clip,int *actualVolume){

	float volumeFactor;
	int volume;

	volumeFactor = m_volume / 100.0;
	volumeFactor *= trackData->m_track.volume / 100.0;
	volumeFactor *= clip->volume / 100.0;
	volume = volumeFactor * 100;

	if(volume < 0 || volume > 200){
		return true;
	}
	*actualVolume = volume;

	return false;
}
bool VEConfig::invalidSpeedForTrack(VETrackData* trackData,float trackSpeed){
	VETrackData trackDataTemp = *trackData;
	int i,size;
	float actualSpeed;
	trackDataTemp.m_track.speed = trackSpeed;

	size = trackData->m_clips.size();
	if(size){
		for(i=0;i<size;i++){
			VEClipData* clipData = &trackData->m_clips[i];

			if(invalidSpeed(&trackDataTemp,clipData,i,&actualSpeed)){
				return true;
			}
		}
	}

	return false;
}
bool VEConfig::invalidSpeed(VETrackData* trackData,VEClipData* clipData,int index,float *actualSpeed){

	int clipDuration;
	float speed,transitionSpeed;
	int transitionDuration;

	speed = m_speed * trackData->m_track.speed * clipData->m_clip.speed;


	if(speed <= 1.0/64 || speed > 64){
		return true;
	}
	*actualSpeed = speed;

	if(trackData->m_transitions.size()){
		std::map<int,VETransitionData>::iterator it = trackData->m_transitions.begin();
		for(;it != trackData->m_transitions.end();it++){
			VETransitionData & transitionData = it->second;
			if(transitionData.m_transition.clip_index_b == index || transitionData.m_transition.clip_index_b == index + 1){
				clipDuration = this->getClipDuration(clipData);

				VEClipData *indexBClipData;
				if(transitionData.m_transition.clip_index_b == index){
					indexBClipData = clipData;
				}else if(transitionData.m_transition.clip_index_b == index + 1){
					indexBClipData = &trackData->m_clips[index];
				}
				transitionSpeed = m_speed * trackData->m_track.speed * indexBClipData->m_clip.speed;
				transitionDuration = transitionData.m_transition.duration / transitionSpeed;
				if(transitionDuration > clipDuration){
					return true;
				}
			}
		}
	}
	return false;
}
bool VEConfig::invalidSlvInfo(slv_info* slv){

	int slvIndex = 0;

	if(!slv->active){
		return false;
	}
	if(slv->start_time[0]){
		return true;
	}
	for(;slvIndex < slv->len;slvIndex++){
		if(slvIndex < slv->len -1 && slv->end_time[slvIndex] != slv->start_time[slvIndex + 1]){
			return true;
		}

		if(slv->end_time[slvIndex]  <= slv->start_time[slvIndex]){
			return true;
		}
	}

	return false;
}
void VEConfig::reAssignFiltersStrPtr(std::map<int,VEFilterData> & filterMap){

	std::map<int,VEFilterData>::iterator f_it = filterMap.begin();

	if(filterMap.size()){
		for(;f_it != filterMap.end();f_it++){
			VEFilterData & filterData = f_it->second;
			filterData.m_filter.action = filterData.m_action.c_str();
		}
	}
}
void VEConfig::insertClip(std::deque<VEClipData>& clips,VEClipData * clip,int moveIndex){

	if(moveIndex < 0)moveIndex = 0;
    int size = clips.size();
    if(moveIndex >= size){
    	moveIndex = size;
    }

    VEClipData* move_file = clip;

	if(moveIndex == 0){
		clips.push_front(*move_file);
	}else if(moveIndex == size){
		clips.push_back(*move_file);
	}else{
		std::deque<VEClipData>::iterator it = clips.begin() + moveIndex;
		std::deque<VEClipData> temp_deque;
		for(;it != clips.end();it++){
			temp_deque.push_back(*it);
		}
		it = clips.begin() + moveIndex;
		clips.erase(it,clips.end());
		clips.push_back(*move_file);
		it = temp_deque.begin();
		for(;it != temp_deque.end();it++){
			clips.push_back(*it);
		}
	}
}
void VEConfig::ajustVolume(VEConfig* config,VETrackData* trackData,ve_clip* clip,int *actualVolume){

	float volumeFactor;
	int volume;

	volumeFactor = config->m_volume / 100.0;
	volumeFactor *= trackData->m_track.volume / 100.0;
	volumeFactor *= clip->volume / 100.0;
	volume = volumeFactor * 100;

	if(volume < 0){
		volume = 0;
	}
	if(volume > 200){
		volume = 200;
	}
	*actualVolume = volume;

}
void VEConfig::ajustSpeed(VEConfig* config,VETrackData* trackData,ve_clip* clip,float *actualSpeed){

	float speed;

	speed = config->m_speed * trackData->m_track.speed * clip->speed;

	if(speed <= 1.0/64){
		speed = 1.0/64;
	}
	if(speed > 64){
		speed = 64;
	}
	*actualSpeed = speed;

}


int VEConfig::getClipDuration(VEClipData* clip){

    std::map<int,VETrackData>::iterator it = m_tracks.find(clip->m_clip.track_id);


    VETrackData* trackData = &it->second;

    ajustSpeed(this,trackData,&clip->m_clip,&clip->m_actualSpeed);

	return clip->m_clip.duration / clip->m_actualSpeed;

}


void VEConfig::getTimelineVFilters(VEConfig* config,int vCurTime,ve_multitrack_callback_param & param){


	std::map<int,VEFilterData>::iterator it = config->m_filters.begin();

	VEFilterData* filterData = NULL;

	int curTime = vCurTime;
	int index = 0;

	if(config->m_filters.size()){
		for(;it != config->m_filters.end();it++){
			filterData = &it->second;

			if(filterData->m_filter.type == VE_FILTER_VIDEO && filterData->m_filter.loc_type == VE_FILTER_LOC_TIMELINE &&
					curTime >= filterData->m_filter.start_time && curTime < filterData->m_filter.end_time){

					if(index >= VE_MAX_FILTER_NUM){
						break;
					}
					param.multitrack_filters[index].action = filterData->m_action.c_str();
					param.multitrack_filters[index].start_time = filterData->m_filter.start_time;
					param.multitrack_filters[index].end_time = filterData->m_filter.end_time;
					param.multitrack_filters[index].filter_id = filterData->m_filter.filter_id;

					index++;
			}

		}

	}
	param.multitrack_filters_len = index;


}
void VEConfig::getTrackVFilters(VEConfig* config,int trackId,int vCurTime,ve_track_callback_param & param){

	std::map<int,VETrackData>::iterator trackIt = config->m_tracks.find(trackId);
	std::map<int,VEFilterData>::iterator filterIt;

	VETrackData* trackData = NULL;
	VEFilterData* filterData = NULL;

	int curTime;
	int index = 0;

	if(config->m_tracks.size() && trackIt != config->m_tracks.end()){
		trackData = &trackIt->second;
		curTime = vCurTime;
		if(trackData->m_filters.size()){
			filterIt = trackData->m_filters.begin();
			for(;filterIt != trackData->m_filters.end();filterIt++){
				filterData = &filterIt->second;

				if(filterData->m_filter.type == VE_FILTER_VIDEO && filterData->m_filter.loc_type == VE_FILTER_LOC_TRACK &&
						curTime >= filterData->m_filter.start_time && curTime < filterData->m_filter.end_time){
						
						if(index >= VE_MAX_FILTER_NUM){
							break;
						}
						param.track_filters[index].action = filterData->m_action.c_str();
						param.track_filters[index].start_time = filterData->m_filter.start_time;
						param.track_filters[index].end_time = filterData->m_filter.end_time;
						param.track_filters[index].filter_id = filterData->m_filter.filter_id;
						param.track_id = trackData->m_track.track_id;
						index++;
				}
			}//for
		}//if
		// param.track_context = trackData->m_track.context;
	}//if

	param.track_filters_len = index;
}
void VEConfig::getClipVFilters(VEConfig* config,int trackId,int clipId,int clipCurTime,ve_v_frame_callback_param & param){


	std::map<int,VETrackData>::iterator trackIt = config->m_tracks.find(trackId);
	std::map<int,VEFilterData>::iterator filterIt;

	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;
	VEFilterData* filterData = NULL;


	int clipSize = 0;
	int index = 0;
	int i;

	if(config->m_tracks.size() && trackIt != config->m_tracks.end()){

		trackData = &trackIt->second;

		clipSize = trackData->m_clips.size();

		for(i=0;i<clipSize;i++){
			clipData = &trackData->m_clips[i];
			if(clipData->m_clip.clip_id == clipId){
				break;
			}
			clipData = NULL;
		}

		if(clipData && clipData->m_filters.size()){
			filterIt = clipData->m_filters.begin();
			for(;filterIt != clipData->m_filters.end();filterIt++){
				filterData = &filterIt->second;

				if(filterData->m_filter.type == VE_FILTER_VIDEO && filterData->m_filter.loc_type == VE_FILTER_LOC_CLIP &&
						clipCurTime >= filterData->m_filter.start_time && clipCurTime <= filterData->m_filter.end_time){

						if(index >= VE_MAX_FILTER_NUM){
							break;
						}
						param.clip_filters[index].action = filterData->m_action.c_str();
						param.clip_filters[index].start_time = clipData->m_insertTime + filterData->m_filter.start_time / clipData->m_actualSpeed;
						param.clip_filters[index].end_time = clipData->m_insertTime + filterData->m_filter.end_time / clipData->m_actualSpeed;
						param.clip_filters[index].filter_id = filterData->m_filter.filter_id;
						param.clip_id = clipData->m_clip.clip_id;
						index++;
				}
			}//for
		}//if
		// param.clip_context = clipData->m_clip.context;
	}//if

	param.clip_filters_len = index;
}

void VEConfig::getTransitionFilter(VETrackData* trackData,std::map<int,VETransitionData> & transitions,int clipIndexB,VEClipData * clipData){

	std::map<int,VETransitionData>::iterator it = transitions.begin();

	VETransitionData* transitionData = NULL;

	float transitionSpeed = m_speed * trackData->m_track.speed * clipData->m_clip.speed;

	if(transitions.size()){
		for(;it != transitions.end();it++){
			transitionData = &it->second;
			if(transitionData->m_transition.clip_index_b == clipIndexB){
				clipData->m_transitionDuration = transitionData->m_transition.duration / transitionSpeed;
				clipData->m_transitionAction = transitionData->m_action.c_str();
				clipData->m_transitionId = transitionData->m_transition.transition_id;
			}
		}
	}

}
int VEConfig::getSlvOriginalStartTime(slv_info* slv,int startTime){

	if(slv->active){
		int prevSlvDuration = 0,slvDuration = 0,curSlvDuration = 0;
		int originalStartTime,offset;
		int slvIndex = 0,slvMax = slv->len;

		if(startTime <= 0){
			return 0;
		}

		for(;slvIndex < slvMax;slvIndex++){

			curSlvDuration = (slv->end_time[slvIndex] -  slv->start_time[slvIndex]) /  slv->speed[slvIndex];

			slvDuration += curSlvDuration;

			if(startTime <= slvDuration){

				offset = (startTime - prevSlvDuration) * slv->speed[slvIndex];

				originalStartTime = slv->start_time[slvIndex] + offset;
				return originalStartTime;
			}
			prevSlvDuration += curSlvDuration;
		}

		return slv->end_time[slv->len - 1];

	}else{
		return startTime;
	}
}
int VEConfig::getSlvTime(slv_info* slv,int startTime){

	if(slv->active){
		int prevSlvDuration = 0,slvDuration = 0,curSlvDuration = 0;
		int slvIndex = 0,slvMax = slv->len;

		for(;slvIndex < slvMax;slvIndex++){

			curSlvDuration = (slv->end_time[slvIndex] -  slv->start_time[slvIndex]) /  (slv->speed[slvIndex]);
			if(startTime >= slv->end_time[slvIndex]){
				prevSlvDuration += curSlvDuration;
				continue;
			}
			if(startTime < slv->end_time[slvIndex]){
				curSlvDuration = (startTime -  slv->start_time[slvIndex]) /  (slv->speed[slvIndex]);

				slvDuration = prevSlvDuration + curSlvDuration;
				return slvDuration;
			}
		}
	}
	return 0;
}
int VEConfig::getSlvDuration(slv_info* slv,int startTime,int endTime){

	if(slv->active){
		int slvDuration = getSlvTime(slv,startTime);
		int slvDuration2 = getSlvTime(slv,endTime);

		return slvDuration2 - slvDuration;

	}
	return 0;
}
float VEConfig::getCurSlvSpeed(slv_info* slv,int curTime){

	 if(slv->active){
		int slvIndex = 0,slvMax = slv->len;
		for(;slvIndex < slvMax;slvIndex++){
			if(curTime >= slv->start_time[slvIndex] && curTime <= slv->end_time[slvIndex]){

				return slv->speed[slvIndex];
			}
		}
	}else{
		return 1.0;
	}
   return 1.0;

}
