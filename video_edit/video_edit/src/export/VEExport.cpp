#include "VEExport.h"
#include "ve_interface.h"
#include "VESource.h"
#include "VEMp4Writer.h"
//#include "mp4v2/mp4v2.h"
#include "VECommon.h"
#ifdef __APPLE__
#include "ios+ver.h"
#include "VTDecoder.h"
#endif

#ifdef __APPLE__
#define VE_EXPORT_GPU_QUEUE_LEN	3
#define VE_EXPORT_YUV_QUEUE_LEN 5
#else
#define VE_EXPORT_GPU_QUEUE_LEN	5
#define VE_EXPORT_YUV_QUEUE_LEN 5
#endif

#define VE_MODULE_TAG "[VEExport]"
void * veExportProcess(void* context);
void * veExportVFilterProcess(void* context);
void * veExportAFilterProcess(void* context);

inline int16_t ClampToInt16(int32_t input) {
    if (input < -0x00008000) {
        return -0x8000;
    } else if (input > 0x00007FFF) {
        return 0x7FFF;
    } else {
        return static_cast<int16_t>(input);
    }
}
/*
bool veTrackSourcesComp(const VETrackSources &a,const VETrackSources &b)
{
    return a.m_track.z_order < b.m_track.z_order;
}

bool veTrackComp(const ve_track &a,const ve_track &b)
{
    return a.z_order < b.z_order;
}
int veTrackCmp2 ( const void *a , const void *b ){
    ve_track::track* a_track = (ve_track::track*)a;
    ve_track::track* b_track = (ve_track::track*)b;
    
    return a_track->track_z_order < b_track->track_z_order;
}
*/
VEExport::VEExport(VEConfig * config,ve_filter_callback callback,ve_export_status_callback statusCallback,ve_export_param &param){
	m_configCopy = *config;
	m_config = &m_configCopy;
	m_callback = callback;
	m_statusCallback = statusCallback;
	m_userExt = param.userExt;
	m_render = param.render;
	m_hwEncode = param.hw_encode;
	m_config->m_hwDecode = param.hw_decode;


	if(!config || !callback || !statusCallback){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!config || !callback || !statusCallback");
	}
#ifdef __ANDROID__
	av_log_set_callback(veLogCallbackFfmpeg);
#endif
#ifdef __APPLE__
	av_log_set_callback(NULL);
#endif
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_hwEncode=%d",m_hwEncode);
    VEConfig::initialize();

#if VE_TEST_NO_V_FILTERING
    m_config->m_width = 504;
    m_config->m_height = 896;
#endif
}
VEExport::~VEExport(){
	this->cancel();
}
void VEExport::getStatus(ve_export_status * exportStatus,int status,int errNo){
	exportStatus->handle = this;
	exportStatus->progress = this->m_progress;
	exportStatus->status = status;
	exportStatus->userExt = this->m_userExt;
	exportStatus->err_no = errNo;
}

int VEExport::init(){

	VEConfig* config = this->m_config;

	int count = 0;
	int ret,i = 0,size;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"Init enter");
	//参数检查
	if(!config->m_width || !config->m_height || !config->m_filename.size() || !config->m_tracks.size()){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!config->m_width || !config->m_height || !config->m_filename.size() || !config->m_tracks.size()");
		return VE_ERR_INPUT_PARAM;
	}

	//帧率
	if(config->m_outputFps < VE_MIN_FPS || config->m_outputFps > VE_MAX_FPS){
		config->m_outputFps = VE_DEFAULT_PFS;
	}

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_width=%d,m_height=%d,m_bitrate=%d,m_filename=%s",config->m_width,config->m_height,m_config->m_bitrate,config->m_filename.size()?config->m_filename.c_str():"");

	std::map<int,VETrackData>::iterator it = config->m_tracks.begin();
	VETrackData* trackData = NULL,*trackData2 = NULL;
	VEClipData* clipData = NULL;
	int duration = 0,preDuration;
	//初始化输入源

	for(;it != config->m_tracks.end();it++){
		trackData = &it->second;

		VE_SOURCE_TYPE type;
		VESource* source = 0;
		std::vector<VETrackSources>* trackSourcesVector;

		switch(trackData->m_track.type){
		case VE_TRACK_VIDEO:
		{
			count += trackData->m_clips.size();
		}
		case VE_TRACK_AUDIO:
		{
			VETrackSources trackSource;
			trackSource.m_track = trackData->m_track;
			size = trackData->m_clips.size();

			if(trackData->m_track.type == VE_TRACK_VIDEO){
				trackSourcesVector = &m_vTrackSources;
			}else{
				trackSourcesVector = &m_aTrackSources;
			}

			//插入clip、track
			if(size){
				for(i=0;i<size;i++){
					clipData = &trackData->m_clips[i];
					if(clipData->m_clip.type == VE_CLIP_PICTURE){
						type = VE_SOURCE_TYPE_PICTURE;
					}else if(clipData->m_clip.type == VE_CLIP_VIDEO){
						type = VE_SOURCE_TYPE_VIDEO;
					}else{
						type = VE_SOURCE_TYPE_AUDIO;
					}
					clipData->m_index = i;

					m_config->getTransitionFilter(trackData,trackData->m_transitions,i,clipData);

					if(clipData->m_clipArrangement == VE_CLIP_ARRANGEMENT_SEQUENCE){
                        if(i == 0){
                            preDuration = 0;
                        }else{
                            m_config->getTrackDuration(clipData->m_clip.track_id,i - 1,1,&preDuration);
                            
                            preDuration -= clipData->m_transitionDuration;
                        }

					}else{
						preDuration = clipData->m_clip.insert_time;
					}
					clipData->m_insertTime = preDuration;

					VEConfig::ajustSpeed(m_config,trackData,&clipData->m_clip,&clipData->m_actualSpeed);
					VEConfig::ajustVolume(m_config,trackData,&clipData->m_clip,&clipData->m_actualVolume);

					source = new VESource(type,m_config,clipData,this);

					trackSource.m_sources.insert(trackSource.m_sources.end(),source);

					if(clipData->m_clip.type == VE_CLIP_VIDEO && clipData->m_clip.info.a_codec_id && clipData->m_clip.volume != 0){

						VETrackSources trackSource2;
						VEClipData audioClipData = *clipData;
						trackSource2.m_track = trackData->m_track;
						audioClipData.m_split = 1;
						audioClipData.m_clip.type = VE_CLIP_AUDIO;
						source = new VESource(VE_SOURCE_TYPE_AUDIO,m_config,&audioClipData,this);

						trackSource2.m_sources.insert(trackSource2.m_sources.end(),source);

						m_aTrackSources.insert(m_aTrackSources.end(),trackSource2);
					}
				}//for

				trackSourcesVector->insert(trackSourcesVector->end(),trackSource);
			}//size
		}
			break;
		default:
			break;
		}

	}
	if(!count){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"no video clip found");
		return VE_ERR_INPUT_PARAM;
	}


	//根据z-order排序video track和multitrack，以及mutltrack内部track的排序
	/*
	sort(m_vTrackSources.begin(),m_vTrackSources.end(),veTrackSourcesComp);
	sort(m_multitracks.begin(),m_multitracks.end(),veTrackComp);

	std::vector<ve_track>::iterator m_multitracks_it = m_multitracks.begin();

	for(;m_multitracks_it != m_multitracks.end();m_multitracks_it++){
		ve_track & track = *m_multitracks_it;
        qsort(track.tracks,track.trackNum,sizeof(track.tracks[0]),veTrackCmp2);
	}
	*/


	//初始化输出目的
	ve_mp4_writer mp4WriterConfig;
	mp4WriterConfig.m_filename = config->m_filename;
	mp4WriterConfig.m_vConfig.m_width = config->m_width;
	mp4WriterConfig.m_vConfig.m_height = config->m_height;
	mp4WriterConfig.m_vConfig.m_fps = config->m_outputFps;
	mp4WriterConfig.m_vConfig.m_gop = mp4WriterConfig.m_vConfig.m_fps<<1;
	mp4WriterConfig.m_vConfig.m_vBitrate = config->m_videoBitrate;
	mp4WriterConfig.m_vConfig.m_vTimebase = m_VTimebase;
	mp4WriterConfig.m_vConfig.m_yuvQueueLen = VE_EXPORT_YUV_QUEUE_LEN + 1;
#ifdef __APPLE__
	mp4WriterConfig.m_vConfig.m_type = VE_ENC_VT_H264;
#else
	if(m_hwEncode){
		mp4WriterConfig.m_vConfig.m_type = VE_ENC_NONE;
	}else{
		mp4WriterConfig.m_vConfig.m_type = VE_ENC_H264;
	}

#endif

	mp4WriterConfig.m_fmt = "mp4";
	config->getTimelineDuration(1,&m_exportDuration);
	m_exportDurationUs = m_exportDuration * 1000;
	mp4WriterConfig.m_duration = m_exportDuration;
	mp4WriterConfig.m_aConfig.m_aBitrate = BITRATE_AAC;
	mp4WriterConfig.m_aConfig.m_aTimebase = m_ATimebase;
	mp4WriterConfig.m_aConfig.m_channels = m_channels;
	mp4WriterConfig.m_aConfig.m_samplefmt = m_samplefmt;
	mp4WriterConfig.m_aConfig.m_bytesPerSample = av_get_bytes_per_sample(mp4WriterConfig.m_aConfig.m_samplefmt) * mp4WriterConfig.m_aConfig.m_channels;
	mp4WriterConfig.m_aConfig.m_channelLayout = AV_CH_LAYOUT_STEREO;
	mp4WriterConfig.m_aConfig.m_samplerate = m_samplerate;
	mp4WriterConfig.m_aConfig.m_type = VE_ENC_FDK_AAC;

	mp4WriterConfig.m_startTime = av_gettime();


	m_veMp4writer = new VEMp4Writer(&mp4WriterConfig);

	if(ret = m_veMp4writer->start()){
		return ret;
	}


	if(0 != pthread_create(&m_vFilterWorker, NULL, veExportVFilterProcess, (void *)this)){
        //VE_LOG_ERROR("VEExport::Init pthread_create veExportVFilterProcess failed!");
        //return VE_ERR_CREATE_THREAD_FAILED;
	}

	if(0 != pthread_create(&m_aFilterWorker, NULL, veExportAFilterProcess, (void *)this)){
        //VE_LOG_ERROR("VEExport::Init pthread_create veExportAFilterProcess failed!");
        //return VE_ERR_CREATE_THREAD_FAILED;
	}
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"Init done");
	return VE_ERR_OK;
}
void VEExport::stop(){


	int i,j,k,trackSize,clipSize;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;
	VESource* source = NULL;


	VE_LOG_TAG_INFO(VE_MODULE_TAG,"stop");
	m_exit = 1;



	//Stop Filter Process
	if(m_vFilterWorker){
		pthread_join(m_vFilterWorker, NULL);
		m_vFilterWorker = 0;
	}
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_vFilterWorker exit");
	if(m_aFilterWorker){
		pthread_join(m_aFilterWorker, NULL);
		m_aFilterWorker = 0;
	}
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_aFilterWorker exit");
	//Stop源
	
	int trackSourcesSize = 2;
	std::vector<VETrackSources> * trackSources[2] = {&m_vTrackSources,&m_aTrackSources};

	for(k=0;k<trackSourcesSize;k++){

		trackSize = trackSources[k]->size();

		for(i=0;i<trackSize;i++){
			VETrackSources & trackSource = (*trackSources[k])[i];

			clipSize = trackSource.m_sources.size();

			for(j=0;j<clipSize;j++){
				source = trackSource.m_sources[j];
				source->stop();
			}
		}
	}
    //Stop muxer
	m_veMp4writer->stop();

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_veMp4writer exit");
}
void VEExport::release(){

	int i,j,k,trackSize,clipSize;
	VETrackData* trackData = NULL;
	VEClipData* clipData = NULL;
	VESource* source = NULL;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"release");
	//delete对象
	int trackSourcesSize = 2;
	std::vector<VETrackSources> * trackSources[2] = {&m_vTrackSources,&m_aTrackSources};

	for(k=0;k<trackSourcesSize;k++){

		trackSize = trackSources[k]->size();

		for(i=0;i<trackSize;i++){
			VETrackSources & trackSource = (*trackSources[k])[i];

			clipSize = trackSource.m_sources.size();

			for(j=0;j<clipSize;j++){
				source = trackSource.m_sources[j];
				delete source;
			}
		}
	}

	delete m_veMp4writer;

	if(m_pcmEmptyBuffer){
		av_free(m_pcmEmptyBuffer);
		m_pcmEmptyBuffer = NULL;
	}

}

void VEExport::progressing(){
	int progress = m_veMp4writer->getProgress();
	this->m_progress = progress < 100?progress:99;

	if(m_veMp4writer->IsComplete()){
		m_done = 1;
	}
}
int VEExport::getFilterCallbackParam(int curTime,ve_filter_callback_param & callbackParam){

	
	VEClipData* clipData = NULL;
	int i,j,gotPicture,ret;
    int multiTrackIndex,trackIndex;
	int trackSize = m_vTrackSources.size(),sourceSize;
    int trackId,clipId;
    int duration,preDuration;
    int found = 0;
    int eof;

	//填充默认multitrack

    multiTrackIndex = 0;
    trackIndex = 0;

    callbackParam.multitrack_num = 1;
	callbackParam.cur_time = m_curVFramePts;

	//callbackParam.multitracks[0]
	for(i=0;i<trackSize;i++){

			VETrackSources & trackSources = m_vTrackSources[i];

			sourceSize = trackSources.m_sources.size();
			trackId = trackSources.m_track.track_id;


			for(j=0;j<sourceSize;j++){

                ve_dec_yuv_buffer yuvBuffer;
                
				VESource* source = trackSources.m_sources[j];

				clipId = source->getClipId();
				clipData = source->getClipData();

				
				if(trackSources.m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY){
					preDuration = clipData->m_insertTime;
                    duration = preDuration + m_config->getClipDuration(clipData);
				}else{

                    if(clipData->m_index == 0){
                        preDuration = 0;
                    }else{
                        m_config->getTrackDuration(trackId,clipData->m_index - 1,1,&preDuration);
                    }
                    m_config->getTrackDuration(trackId,clipData->m_index,1,&duration);
				}
				

				if(source->isStop())continue;

				if(m_curVFramePts > duration){
					source->stop();
					continue;
				}

				if(fabs(preDuration - m_curVFramePts) < 100 + clipData->m_transitionDuration || m_curVFramePts > preDuration){
					if(!source->isStart()){
						source->start();
					}
				}

				eof = 0;
				gotPicture = 0;
				if( ret = source->getYuvBuffer(&yuvBuffer,&gotPicture,&eof,m_curVFramePts)){
					VE_LOG_TAG_ERROR(VE_MODULE_TAG,"source->GetYuvBuffer failed!");
					return ret;
				}

				if(eof){
					continue;
				}

				if(gotPicture){

					if(yuvBuffer.m_ts - yuvBuffer.m_vCurTime > 150 && clipData->m_clip.type == VE_CLIP_VIDEO){
#ifdef __APPLE__
				CVPixelBufferRelease((CVImageBufferRef)yuvBuffer.m_data);
#endif
						source->returnYuv();
						break;
					}

					found = 1;
					ve_v_frame_callback_param * vframeCallbackParam = &callbackParam.multitracks[multiTrackIndex].tracks[trackIndex].frame_data[0];
					if(yuvBuffer.m_transitionFrame){
                        if(vframeCallbackParam->data){
                            vframeCallbackParam = &callbackParam.multitracks[multiTrackIndex].tracks[trackIndex].frame_data[1];
                            vframeCallbackParam->transition_frame = 1;
                            vframeCallbackParam->transition_action = yuvBuffer.m_transitionAction;
                            vframeCallbackParam->transition_id = yuvBuffer.m_transitionId;
                            vframeCallbackParam->transition_start_time = preDuration - clipData->m_transitionDuration;
                            vframeCallbackParam->transition_end_time = preDuration;
                        }
					}

					vframeCallbackParam->data = yuvBuffer.m_data;
					vframeCallbackParam->len = yuvBuffer.m_len;
					vframeCallbackParam->format = yuvBuffer.m_format;
					vframeCallbackParam->rotate = yuvBuffer.m_rotate;
					vframeCallbackParam->width = yuvBuffer.m_width;
					vframeCallbackParam->height = yuvBuffer.m_height;



					vframeCallbackParam->source = (void*)source;

					//clip特效
					VEConfig::getClipVFilters(m_config,trackId,clipId,yuvBuffer.m_clipTs,*vframeCallbackParam);

					vframeCallbackParam->clip_id = clipId;

					//track特效
					VEConfig::getTrackVFilters(m_config,trackId,m_curVFramePts,callbackParam.multitracks[0].tracks[trackIndex]);
					callbackParam.multitracks[0].tracks[trackIndex].track_id = trackId;
					if(yuvBuffer.m_transitionFrame){
						break;
					}
				}else if(callbackParam.multitracks[multiTrackIndex].tracks[trackIndex].frame_data[0].data){//gotPicture
					
				}
			}//sourceSize
			if(found){
				trackIndex++;
				found = 0;
			}

	}//trackSize
	callbackParam.multitracks[multiTrackIndex].tracks_num = trackIndex;
	callbackParam.timeline_id = m_config->m_id;
	// callbackParam.timeline_context = m_config->m_context;
	//timeline特效
	VEConfig::getTimelineVFilters(m_config,m_curVFramePts,callbackParam.multitracks[multiTrackIndex]);

	return VE_ERR_OK;
}
int VEExport::audioFilterProcess(){

	ve_dec_pcm_buffer pcms[VE_MAX_TRACK_NUM];
	ve_enc_pcm_buffer pcm;
	VESource* curSource = NULL;
	VEClipData* clipData = NULL;
	int k,i,j,sourceSize,pcmIndex = 0;
	int trackSize = m_aTrackSources.size();
	int gotPcm,eof;
	int ret;
	int sampleSize,pcmSize;
	int32_t wrapGuard;
	short *pcmPtr1,*pcmPtr2;
    int trackId,clipId;
    int duration,preDuration;
    int curAFramePts = m_curAFramePts / 1000;

	if(m_curAFramePts > m_exportDurationUs){
		av_usleep(AV_USLEEP_GAP);

		return VE_ERR_OK;
	}

	for(i=0;i<trackSize;i++){

		VETrackSources & trackSources = m_aTrackSources[i];
		sourceSize = trackSources.m_sources.size();

		for(j=0;j<sourceSize;j++){
			curSource = trackSources.m_sources[j];

			trackId = trackSources.m_track.track_id;
			clipId = curSource->getClipId();
			clipData = curSource->getClipData();


			if(trackSources.m_track.clip_arrangement == VE_CLIP_ARRANGEMENT_OVERLAY){
				preDuration = clipData->m_insertTime;
                duration = preDuration + m_config->getClipDuration(clipData);
			}else{

                if(clipData->m_index == 0){
                    preDuration = 0;
                }else{
                    m_config->getTrackDuration(trackId,clipData->m_index - 1,1,&preDuration);
                }
                m_config->getTrackDuration(trackId,clipData->m_index,1,&duration);
			}


			if(curSource->isStop())continue;

			if(curAFramePts > duration){
				curSource->stop();
				continue;
			}

            if(fabs(preDuration - curAFramePts) < 100 + clipData->m_transitionDuration || curAFramePts > preDuration){
				if(!curSource->isStart()){
					curSource->start();
				}
			}
			//filter process start for every single clip

			gotPcm = eof = 0;
			ret = curSource->getPcmBuffer(&pcms[pcmIndex],&gotPcm,&eof,curAFramePts);

			if(ret){
				return ret;
			}

			if(eof || !gotPcm){
				continue;
			}

			//filter process end

			//speed,volume,fade

			//pitch

			pcms[pcmIndex].m_source = curSource;
			pcmIndex++;
			break;
		}//j
	}//i


	//mix
	if(pcmIndex){
		sampleSize = pcms[0].m_sampleSize;
		pcmSize = pcms[0].m_len;
		for(i=0;i<sampleSize;i++){

			pcmPtr1 = (short*)pcms[0].m_data;

			for(j=1;j<pcmIndex;j++){

				pcmPtr2 = (short*)pcms[j].m_data;

				wrapGuard = static_cast<int32_t>(pcmPtr1[i * 2]) + static_cast<int32_t>(pcmPtr2[i * 2]);
				pcmPtr1[i * 2] = ClampToInt16(wrapGuard);
				wrapGuard = static_cast<int32_t>(pcmPtr1[i * 2 + 1]) + static_cast<int32_t>(pcmPtr2[i * 2 + 1]);
				pcmPtr1[i * 2 + 1] = ClampToInt16(wrapGuard);
			}
		}
	}else{
		pcmSize = m_outputPcmSize;
	}



	if(!pcmIndex){
		if(!m_pcmEmptyBuffer){
			m_pcmEmptyBuffer = (uint8_t*)av_malloc(m_outputPcmSize);
			memset(m_pcmEmptyBuffer,0,m_outputPcmSize);
		}
		pcm.m_data = m_pcmEmptyBuffer;
	}else{
		pcm.m_data = pcms[0].m_data;
	}
	pcm.m_len = pcmSize;

	ret = m_veMp4writer->writeAndEnc(&pcm);
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"cur_a_frame_pts=%d",curAFramePts);
	for(i=0;i<pcmIndex;i++){
		pcms[i].m_source->returnPcm();
	}

	if(ret){
		return ret;
	}

	m_curAFramePts += pcms[0].m_sampleSize * (int64_t)1000000 / pcms[0].m_samplerate;

	return VE_ERR_OK;
}
int VEExport::videoFilterProcess(){

	int ret;

	int i,j;

	int trackNum = 0;


	if(m_yuvSendCount - m_yuvSendBackCount > VE_EXPORT_GPU_QUEUE_LEN || m_veMp4writer->getYuvLen() > VE_EXPORT_YUV_QUEUE_LEN){
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"filtered_yuv_len=%d,m_yuvSendCount=%d,m_yuvSendBackCount=%d",m_veMp4writer->getYuvLen(),m_yuvSendCount,m_yuvSendBackCount);
		av_usleep(AV_USLEEP_GAP);
		return VE_ERR_OK;
	}
	if(m_curVFramePts > m_exportDuration + 50){
		if(m_yuvSendBackCount == m_yuvSendCount){
			m_veMp4writer->vComplete();

			if(m_curAFramePts > m_exportDurationUs){
				m_veMp4writer->complete();

				m_veMp4writer->stop();
			}
		}

		av_usleep(AV_USLEEP_GAP);

		return VE_ERR_OK;
    }else{
    	m_callbackParam.multitracks[0].tracks_num = 0;
    	m_callbackParam.multitracks[0].multitrack_filters_len = 0;

    	memset(&m_callbackParam,0,sizeof(m_callbackParam));
        ret = getFilterCallbackParam(m_curVFramePts,m_callbackParam);

        if(ret){
            return ret;
        }
    }

    trackNum = m_callbackParam.multitracks[0].tracks_num;
    
    if(trackNum){
    	m_yuvSendCount++;
    	VE_LOG_TAG_INFO(VE_MODULE_TAG,"cur_v_frame_pts=%d,m_yuvSendCount=%d,m_yuvSendBackCount=%d",m_curVFramePts,m_yuvSendCount,m_yuvSendBackCount);
    	m_callback(this,&m_callbackParam,m_userExt);
    }

	for(i=0;i<trackNum;i++){

		for(j=0;j<2;j++){
			ve_v_frame_callback_param & vFrameMultiTrackCallbackParam = m_callbackParam.multitracks[0].tracks[i].frame_data[j];

	        VESource* source = (VESource*)vFrameMultiTrackCallbackParam.source;

	        if(source){
	            source->returnYuv();
	        }
		}

	}

	m_curVFramePts += 1000 / m_config->m_outputFps;

    return VE_ERR_OK;
}
void VEExport::start(){

	comn::AutoCritSec lock(m_cs);

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"start enter");
	if(m_progress > 0.01 || m_status)return;
	if(m_exit){
        m_exit = 0;

    	if(0 != pthread_create(&m_worker, NULL, veExportProcess, (void *)this)){
            VE_LOG_TAG_ERROR(VE_MODULE_TAG,"pthread_create veExportProcess failed!");
            return;
    	}

	}
}

void VEExport::cancel(){

	comn::AutoCritSec lock(m_cs);

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"cancel");
	m_exit = 1;
	m_cancel = 1;

	if(m_worker){
		pthread_join(m_worker, NULL);
		m_worker = 0;
	}
}

void VEExport::sendData(ve_filtered_data* filteredData){

	ve_enc_yuv_buffer yuv;
	int ret;

	yuv.m_data = (uint8_t*)filteredData->data;
	yuv.m_len = filteredData->len;

#ifdef __ANDROID__
	if(filteredData->frame_type == ANDROID_MEDIACODEC_FRAME || filteredData->frame_type == ANDROID_MEDIACODEC_KEY_FRAME){
		yuv.m_format = VE_H264_PACKET;
	}else if(filteredData->frame_type == ANDROID_RGBA){
		yuv.m_format = VE_RGBA;
	}
#endif
#ifdef __APPLE__

		yuv.m_format = VE_YUV_VT_PIXELBUFFER;

#endif

	yuv.m_width = filteredData->width;
	yuv.m_height = filteredData->height;
	yuv.m_pts = filteredData->cur_time;
	yuv.m_frameType = filteredData->frame_type;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"frame_type=%d,pts=%lld,data=%p,len=%d,format=%d,width=%d,height=%d,end_frame=%d,m_yuvSendCount=%d,m_yuvSendBackCount=%d",filteredData->frame_type,yuv.m_pts,yuv.m_data,yuv.m_len,yuv.m_format,yuv.m_width,yuv.m_height,filteredData->end_frame,m_yuvSendCount,m_yuvSendBackCount)

#ifdef __ANDROID__
	if(filteredData->frame_type == ANDROID_MEDIACODEC_PPS_SPS){
		m_veMp4writer->writeExtradata(yuv.m_data,yuv.m_len);
		return;
	}
#endif
	if(!yuv.m_len || !yuv.m_data || m_status || m_exit){
		return;
	}

#ifdef __ANDROID__
	if(filteredData->end_frame){
		return;
	}
#endif
#ifdef __APPLE__
    CVPixelBufferRef destPixel = (CVPixelBufferRef)yuv.m_data;
    CVPixelBufferRetain((CVImageBufferRef)yuv.m_data);
#endif


    ret = m_veMp4writer->write(&yuv);

    if(ret){
    	m_status = ret;
    }
    m_yuvSendBackCount++;
}

void * veExportProcess(void* context){

	VEExport* veExport = (VEExport*)context;

	int ret;
	float lastProgress = 0;
	ve_export_status exportStatus;
	exportStatus.handle = context;

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"veExportProcess enter");

	ret = veExport->init();

	if(ret){

		veExport->m_status = ret;
		veExport->getStatus(&exportStatus,VE_EXPORT_ERR,veExport->m_status);
		veExport->m_statusCallback(&exportStatus);
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"veExportProcess exit veExport->Init() veExport->m_status=%d",veExport->m_status);
		return 0;
	}

	while(!veExport->m_exit){

		veExport->progressing();

		if(veExport->m_status){

    		veExport->getStatus(&exportStatus,VE_EXPORT_ERR,veExport->m_status);
    		veExport->m_statusCallback(&exportStatus);
    		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"veExportProcess failed! veExport->m_status=%d",veExport->m_status);
			break;
		}
        if(veExport->m_progress - lastProgress >= 1 || fabs(veExport->m_progress - 100) < 0.01){
            lastProgress = veExport->m_progress;
            veExport->getStatus(&exportStatus,VE_EXPORT_PROCESSING,VE_ERR_OK);
            veExport->m_statusCallback(&exportStatus);
        }
        if(veExport->m_done){
            break;
        }
        av_usleep(150000);

	}

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"veExportProcess exit");
	veExport->stop();

	veExport->release();


	if(veExport->m_done){
		//VE_LOG_INFO("VEExport veExportProcess MP4Optimize:%s",veExport->m_veMp4writer->m_config.filename.c_str());
#ifdef __APPLE__
        //MP4Optimize(veExport->m_config->m_filename.c_str(),NULL);
#else
        //MP4Optimize(veExport->m_config->m_filename.c_str(),NULL);
#endif

        veExport->m_progress = 100;
        veExport->getStatus(&exportStatus,VE_EXPORT_END,VE_ERR_OK);
        veExport->m_statusCallback(&exportStatus);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"veExportProcess m_progress=%f",veExport->m_progress);
	}
	if(veExport->m_cancel){
        veExport->getStatus(&exportStatus,VE_EXPORT_CANCEL,veExport->m_status);
        veExport->m_statusCallback(&exportStatus);
	}
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"veExportProcess exit done");

	return NULL;
}
void * veExportVFilterProcess(void* context){

	VEExport* veExport = (VEExport*)context;
	ve_export_status exportStatus;
	int ret;
	exportStatus.handle = context;

	while(!veExport->m_exit){
		ret =  veExport->videoFilterProcess();

		if(ret){
			veExport->m_status = ret;
			veExport->getStatus(&exportStatus,VE_EXPORT_ERR,veExport->m_status);
			veExport->m_statusCallback(&exportStatus);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"veExportVFilterProcess exit veExport->m_status=%d",veExport->m_status);
			return 0;
		}
	}
    
    VE_LOG_TAG_INFO(VE_MODULE_TAG,"veExportVFilterProcess exit");
	return NULL;
}

void * veExportAFilterProcess(void* context){

	VEExport* veExport = (VEExport*)context;
	ve_export_status exportStatus;
	int ret;
	exportStatus.handle = context;

	while(!veExport->m_exit){
		ret =  veExport->audioFilterProcess();

		if(ret){
			veExport->m_status = ret;
			veExport->getStatus(&exportStatus,VE_EXPORT_ERR,veExport->m_status);
			veExport->m_statusCallback(&exportStatus);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"veExportAFilterProcess exit veExport->m_status=%d",veExport->m_status);
			return 0;
		}
	}

    VE_LOG_TAG_INFO(VE_MODULE_TAG,"veExportAFilterProcess exit");
	return NULL;
}
