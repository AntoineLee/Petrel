#include "VESource.h"
#include "VEAudioFilter.h"
#include "VESwsscale.h"
#include "VESoundTouch.h"

#define VE_MAX_YUV_BUFFER_NUM 2
#define VE_MAX_PCM_BUFFER_NUM 5

#define VE_MODULE_TAG "[VESource]"

bool VESource::m_unitTest = false;
#ifdef __APPLE__
#include "ios+ver.h"
#include "VTDecoder.h"
#define DEFAULT_COLOR AV_PIX_FMT_NV12

#else
#define DEFAULT_COLOR AV_PIX_FMT_YUV420P

#endif

void* veSourceReadpacketProcess(void* context);
void* veSourceVideoProcess(void* context);
void* veSourceAudioProcess(void* context);

int GetPixelBufferNV12Align(int width,int height){
#ifdef __APPLE__
	OSType type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
	CVPixelBufferRef dstPixelbuffer = NULL;

    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);

    if(height % 2){
    	height++;
    }

	CVPixelBufferCreate(kCFAllocatorDefault, width, height, type, attrs, &dstPixelbuffer);
	CVPixelBufferLockBaseAddress(dstPixelbuffer, 0);
	void* dstY = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 0);
	void* dstUv = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 1);

    size_t yLinesize = CVPixelBufferGetBytesPerRowOfPlane(dstPixelbuffer, 0);
    CVPixelBufferUnlockBaseAddress(dstPixelbuffer, 0);


    CFRelease(empty);
    CFRelease(attrs);
    CVPixelBufferRelease(dstPixelbuffer);

    int alignSize = 8;

    while(1){
        if(yLinesize % alignSize == 0){
            alignSize <<= 1;
        }else{
            alignSize >>= 1;
            break;
        }
    }
    return alignSize;
#endif
    return 16;
}

#ifdef __APPLE__

void GetPixelBufferFromRGBA(int format,int width,int height,uint8_t* rgbaYuv[4],int linesize[4],void** pixelBuffer){

	OSType type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;

	if(format == AV_PIX_FMT_ARGB){
		type = kCVPixelFormatType_32ARGB;
	}else if(format == AV_PIX_FMT_RGBA){
		type = kCVPixelFormatType_32RGBA;
	}else if(format == AV_PIX_FMT_ABGR){
		type = kCVPixelFormatType_32ABGR;
	}else if(format == AV_PIX_FMT_BGRA){
		type = kCVPixelFormatType_32BGRA;
	}else if(format == AV_PIX_FMT_YUVA444P){
		type = kCVPixelFormatType_4444YpCbCrA8R;
	}

	CVPixelBufferRef dstPixelbuffer = NULL;

    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);

	CVPixelBufferCreate(kCFAllocatorDefault, width, height, type, attrs, &dstPixelbuffer);
	CVPixelBufferLockBaseAddress(dstPixelbuffer, 0);
	void* dstRgba = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 0);
    int bytes_per_row = CVPixelBufferGetBytesPerRow(dstPixelbuffer);


    uint8_t *dest = (uint8_t*)dstRgba;

    int i,j,pos;
	if(kCVPixelFormatType_4444YpCbCrA8R == type){
		//TODO
	}else{
		if(bytes_per_row == linesize[0]){
			memcpy(dstRgba, (uint8_t *)rgbaYuv[0], linesize[0] * height);
		}else{
			int i = 0;
			uint8_t *pData = rgbaYuv[0];
			for(i; i<height;i++){
				memcpy(dest+i*bytes_per_row, pData+i*linesize[0],linesize[0]);
			}
		}
	}


    *pixelBuffer = (void*)dstPixelbuffer;
    CFRelease(empty);
    CFRelease(attrs);

}
void GetPixelBufferFromNV12(int width,int height,uint8_t* y,uint8_t* uv,int linesize,void** pixelBuffer){

	OSType type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
	CVPixelBufferRef dstPixelbuffer = NULL;

    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);

    if(height % 2){
    	height++;
    }

	CVPixelBufferCreate(kCFAllocatorDefault, width, height, type, attrs, &dstPixelbuffer);
	CVPixelBufferLockBaseAddress(dstPixelbuffer, 0);
	void* dstY = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 0);
	void* dstUv = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 1);
	memcpy(dstY, (uint8_t *)y, linesize * height);
	memcpy(dstUv, (uint8_t *)uv, linesize * height /2);
    CVPixelBufferUnlockBaseAddress(dstPixelbuffer, 0);

    *pixelBuffer = (void*)dstPixelbuffer;
    CFRelease(empty);
    CFRelease(attrs);

}
#endif

void VESource::getAudioFilterString(ve_audio_filter_param *param,char* outStr/* len > 256*/){

	float db,factor;

	char fadeStr[128]={0};
    //A1 = A2 * pow(10,-48/20);

	int volume = param->m_volume;
	VE_AUDIO_FILTER_TYPE fade = param->m_fade;
	float samplerateForSpeed = param->m_samplerateForSpeed;

	int64_t startSamples = param->m_startSamples;
	int64_t nbSamples = param->m_nbSamples;
	int gainMin = param->m_gainMin;
	int gainMax = param->m_gainMax;

	if(volume == 0){
		db = 0;
	}else if(volume <= 100){
		factor = volume * 0.3 - 30;
		db = pow(10,factor/20);
	}else{
		factor = volume * 0.12 - 12;
		db = pow(10,factor/20);
	}

	if(fade == VE_AUDIO_FILTER_FADE_IN){
		snprintf(fadeStr,128,",ve_afade=t=in:ss=%lld:ns=%lld:gain_start=%d:gain_end=%d",startSamples,nbSamples,gainMin,gainMax);
	}else if(fade == VE_AUDIO_FILTER_FADE_OUT){
		snprintf(fadeStr,128,",ve_afade=t=out:ss=%lld:ns=%lld:gain_start=%d:gain_end=%d",startSamples,nbSamples,gainMin,gainMax);
	}


	//if(fabs(speed - 1.0) < 0.01){
	/*
	if(0){
		snprintf(outStr,128,"volume=volume=%4.2f%s",db,fadeStr);
	}else{
	*/
	{
		//新算法
		snprintf(outStr,256,"volume=volume=%4.2f%s",db,fadeStr);

		snprintf(outStr + strlen(outStr),128,",asetrate=r=%d",(int)samplerateForSpeed);

	}
}
void VESource::setUnitTest(bool test){
	m_unitTest = test;
}
bool VESource::isStart(){return !m_exit;}
bool VESource::isStop(){return m_stop;}
VEClipData* VESource::getClipData(){return &m_clipData;}
bool VESource::isOpen(){ return m_fmtCtx?true:false;}
VE_ERR VESource::getSourceInfo(const char *filename,ve_clip_info * info,slv_info* slv){

	AVFormatContext *fmtCtx = NULL;

	char err[256] = {0};
	int count = 0;
	VE_ERR ret;
    int rotate;
	int duration = 0;

	ret = openSource(&fmtCtx, filename);

	if(ret){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"failed! :%s",filename);

		return ret;
	}
	/*
	if(!fmtCtx){
		return VE_ERR_OPEN_FILE_FAILED;
	}
	*/

	if(m_unitTest || fmtCtx->nb_streams < 2){
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"stream information(nb_streams < 2) about file:%s",filename);
	}

	ve_clip_info info2;

	*info = info2;

	info->bitrate = fmtCtx->bit_rate;
	int vCodecId = AV_CODEC_ID_NONE,aCodecId = AV_CODEC_ID_NONE;
	int videoStreamIndex = -1,audioStreamIndex = -1;
	for (unsigned int i = 0; i < fmtCtx->nb_streams; i++){

		AVStream *stream = fmtCtx->streams[i];

		 if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO){

			 audioStreamIndex = i;
			 if(stream->codec->codec_id == AV_CODEC_ID_AAC){
				 info->a_codec_format = VE_CODEC_AAC;
			 }else{
				 info->a_codec_format = VE_CODEC_UNKNOWN;
			 }
			 aCodecId = stream->codec->codec_id;
			 info->channels = stream->codec->channels;
			 info->samplerate = stream->codec->sample_rate;

			 info->sample_fmt = stream->codec->sample_fmt;
			 info->channelLayout = stream->codec->channel_layout;
			 info->a_bitrate = stream->codec->bit_rate;

			 info->a_duration = (int)av_rescale_q(stream->duration,stream->time_base,(AVRational){1,1000});

		 }else if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO){

			 videoStreamIndex = i;
			 if(stream->codec->codec_id == AV_CODEC_ID_H264){
				 info->v_codec_format = VE_CODEC_H264;
			 }else if(stream->codec->codec_id == AV_CODEC_ID_HEVC){
				 info->v_codec_format = VE_CODEC_HEVC;
			 }else{
				 info->v_codec_format = VE_CODEC_UNKNOWN;
			 }

			 vCodecId = stream->codec->codec_id;
			 info->width = stream->codec->width;
			 info->height = stream->codec->height;
			 if(stream->duration > 0){
				  info->duration = info->v_duration = (int)av_rescale_q(stream->duration,stream->time_base,(AVRational){1,1000});
			  }else if(fmtCtx->duration > 0){
				  info->duration = info->v_duration = fmtCtx->duration / 1000;
			  }

			 if(stream->avg_frame_rate.den){
				 info->fps = stream->avg_frame_rate.num/stream->avg_frame_rate.den;
			 }

			 info->v_bitrate = stream->codec->bit_rate;

             if(stream->codec->gop_size > 0){
                 info->gop_size = stream->codec->gop_size;
             }

			 AVDictionaryEntry *m = NULL;
			 while((m=av_dict_get(stream->metadata,"",m,AV_DICT_IGNORE_SUFFIX))!=NULL){

				 if(m_unitTest || !strncmp(m->key,"rotate",6)){
                     rotate = atoi(m->value) / 90;
                     rotate %= 4;
                     info->rotate = (VE_ROTATE)rotate;

				 }
			 }
		 }
	}

	if(videoStreamIndex == -1){
		info->duration = (int)av_rescale_q(fmtCtx->streams[audioStreamIndex]->duration,fmtCtx->streams[audioStreamIndex]->time_base,(AVRational){1,1000});
	}

	if(m_unitTest || audioStreamIndex == -1 && videoStreamIndex == -1){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"audioStreamIndex == -1 && videoStreamIndex == -1");
		return VE_ERR_OPEN_FILE_FAILED;
	}


	if( audioStreamIndex == -1 && info->duration < 100 && vCodecId != AV_CODEC_ID_VP8){
		info->picture = 1;
		if(!info->duration){
			info->duration = getDurationForPicture(fmtCtx,videoStreamIndex);
		}
        if(info->duration >= 100){
            info->picture = 0;
        }
    }else{
        if(fmtCtx->duration){
            duration = (int)av_rescale_q(fmtCtx->duration,(AVRational){1,AV_TIME_BASE},(AVRational){1,1000});

            info->duration = duration;

        }
    }


	avformat_close_input(&fmtCtx);

	info->v_codec_id = vCodecId;
	info->a_codec_id = aCodecId;

	if(slv && slv->active){
		int slvIndex,slvMax = slv->len;
		float segmentDuration = 0,segmentDurationAfterSpeed = 0,segmentDurationTemp = 0;
		int duration;
		for(slvIndex = 0;slvIndex < slvMax;slvIndex++){
			segmentDurationTemp = slv->end_time[slvIndex] - slv->start_time[slvIndex];
			segmentDuration += segmentDurationTemp;
			segmentDurationAfterSpeed += segmentDurationTemp / slv->speed[slvIndex];
		}
		duration = segmentDurationAfterSpeed + 0.5;
		info->a_duration = duration;
		info->v_duration = duration;
		info->duration = info->v_duration;
	}

	if(1){

		VE_LOG_TAG_INFO(VE_MODULE_TAG,"stream information about file:%s",filename);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->duration:%d",info->duration);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->a_duration:%d",info->a_duration);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->v_duration:%d",info->v_duration);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->gop_size:%d",info->gop_size);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->rotate:%d",info->rotate);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->bitrate:%d",info->bitrate);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->v_codec_format:%d",info->v_codec_format);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->a_codec_format:%d",info->a_codec_format);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->v_bitrate:%d",info->v_bitrate);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->a_bitrate:%d",info->a_bitrate);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->width:%d",info->width);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->height:%d",info->height);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->fps:%d",info->fps);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->channels:%d",info->channels);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->samplerate:%d",info->samplerate);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->picture:%d",info->picture);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->v_codec_id:%d",vCodecId);
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info->a_codec_id:%d",aCodecId);


	}

	return VE_ERR_OK;
}
VE_ERR VESource::openSource(AVFormatContext **fmtCtx,const char* filename){


	if(!fmtCtx || !filename || *fmtCtx){
		return VE_ERR_INPUT_PARAM;
	}

    int ret = VE_ERR_OK;
    int count = 0;
	char err[256] = {0};

	do{
		count++;
		/* open input file, and allocate format context */
		if ( (ret = avformat_open_input(fmtCtx, filename, NULL, NULL) )< 0) {
			av_strerror(ret, err, 256);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"Could not open source file:%s,ret=%d,err info:%s",filename,ret,err);

			if(count >= 4){
				*fmtCtx = NULL;
				return VE_ERR_OPEN_FILE_FAILED;
			}else{
				av_usleep(40000);
				continue;
			}
		}
		/* retrieve stream information */
		if (m_unitTest || (ret = avformat_find_stream_info(*fmtCtx, NULL)) < 0) {
			if(m_unitTest){
				ret = -1;
			}
			av_strerror(ret, err, 256);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"Could not find stream information about file:%s,ret =%d,err info:%s",filename,ret,err);
			avformat_close_input(fmtCtx);

			if(count >= 4){
				return VE_ERR_FILE_CORRUPT;
			}else{
				av_usleep(40000);
			}
		}
	}while(ret < 0);

    return VE_ERR_OK;
}
int VESource::getDurationForPicture(AVFormatContext *fmtCtx,int streamIndex){

	int ret,eof = 0;
	AVPacket pkt;
	int64_t firstPts = 0,lastPts = 0;

	av_init_packet(&pkt);

	do{
		pkt.data = NULL;
		pkt.size = 0;
		ret = av_read_frame(fmtCtx, &pkt);

		if( ret < 0){
			eof = 1;
		}else{

			if(pkt.stream_index != streamIndex)continue;

			if(!firstPts){
				firstPts = pkt.pts;
			}
			lastPts = pkt.pts;
		}
	}while(!eof);

	return av_rescale_q((lastPts - firstPts),fmtCtx->streams[streamIndex]->time_base,(AVRational){ 1, 1000 });
}
VE_ERR VESource::open(){

	int count = 0;
	VE_ERR ret;
    char err[256] = {0};
    int readGap = 500;
    int tryOpenDecoder = 1;

	if(m_clipData.m_clip.slv.active){
		readGap = 100;
	}

	if(!m_fmtCtx){

		ret = openSource(&m_fmtCtx, m_clipData.m_filename.c_str());
		if(ret){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"openSource failed! :%s",m_clipData.m_filename.c_str());
			return ret;
		}
	}else{
		return VE_ERR_OK;
	}
	m_videoStreamIndex = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	m_audioStreamIndex = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);



	if(m_audioStreamIndex >= 0 && m_clipData.m_clip.type == VE_CLIP_AUDIO){
		if(!m_aFrame){
			m_aFrame = av_frame_alloc();
            /*
			if(!m_aFrame){
				VE_LOG_ERROR("VESource::open m_aFrame = av_frame_alloc() failed!");
				return VE_ERR_MALLOC_FAILED;
			}
             */
		}
		if(!m_aDecContext){
			m_aDecContext = m_fmtCtx->streams[m_audioStreamIndex]->codec;
		}

		m_aDecoder = avcodec_find_decoder(m_aDecContext->codec_id);

		if (avcodec_open2(m_aDecContext, m_aDecoder, NULL) < 0) {
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_open2 failed,video_codec_id=%d", m_aDecContext->codec_id);
			return VE_ERR_OPEN_DECODER_FAILED;
		}

		m_aTimebase = m_fmtCtx->streams[m_audioStreamIndex]->time_base;

		m_aDuration = av_rescale_q(m_fmtCtx->streams[m_audioStreamIndex]->duration,m_aTimebase,(AVRational){ 1, 1000 });

		m_clipDuration = m_clipData.m_clip.end_time - m_clipData.m_clip.start_time;
	}

	if(m_videoStreamIndex >= 0 && (m_clipData.m_clip.type == VE_CLIP_VIDEO || m_clipData.m_clip.type == VE_CLIP_PICTURE)){
		if(!m_vFrame){
			m_vFrame = av_frame_alloc();
			/*
			if(!m_vFrame){
				VE_LOG_ERROR("VESource::open m_vFrame = av_frame_alloc() failed!");
				return VE_ERR_MALLOC_FAILED;
			}
			*/
		}
		if(!m_vDecContext){
			m_vDecContext = m_fmtCtx->streams[m_videoStreamIndex]->codec;
		}

#ifdef __ANDROID__
			VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_config->m_hwDecode=%d",m_config->m_hwDecode);
			if(AV_CODEC_ID_H264 == m_vDecContext->codec_id){
#ifdef _VE_PUBLISH_
				m_vDecoder = &ff_h264_decoder;
#else
				if(m_config->m_hwDecode && GetAndroidVersion() >= 19){
					VE_LOG_TAG_INFO(VE_MODULE_TAG,"h264_mediacodec");

					m_vDecoder = &ff_h264_mediacodec_decoder;
				}else{
					m_vDecoder = &ff_h264_decoder;
				}
#endif
			}else if(AV_CODEC_ID_HEVC == m_vDecContext->codec_id){

#ifdef _VE_PUBLISH_
				m_vDecoder = avcodec_find_decoder(m_vDecContext->codec_id);
#else
				if(m_config->m_hwDecode && GetAndroidVersion() >= 19){
					VE_LOG_TAG_INFO(VE_MODULE_TAG,"hevc_mediacodec");

					m_vDecoder = &ff_hevc_mediacodec_decoder;
				}else{
					m_vDecoder = avcodec_find_decoder(m_vDecContext->codec_id);
				}
#endif
			}else{
				m_vDecoder = avcodec_find_decoder(m_vDecContext->codec_id);
			}
#endif
#ifdef __APPLE__

		if(AV_CODEC_ID_H264 == m_vDecContext->codec_id){
			//m_vDecoder = &ff_h264_decoder;
			m_vDecoder = &h264_videotoolbox_decoder;
			//m_vDecContext->pix_fmt = AV_PIX_FMT_NV12;
			m_vDecContext->opaque = m_vtMode;
			m_vDecContext->pix_fmt = AV_PIX_FMT_VIDEOTOOLBOX;


		}else if(AV_CODEC_ID_HEVC == m_vDecContext->codec_id && isIOSVerAbove(11.0)){
			m_vDecoder = &hevc_videotoolbox_decoder;
			//m_vDecContext->pix_fmt = AV_PIX_FMT_NV12;
			m_vDecContext->pix_fmt = AV_PIX_FMT_VIDEOTOOLBOX;
			m_vDecContext->opaque = m_vtMode;
		}else{
			m_vDecoder = avcodec_find_decoder(m_vDecContext->codec_id);
		}
#endif

		do{
			if (avcodec_open2(m_vDecContext, m_vDecoder, NULL) < 0) {
#ifdef __APPLE__
				m_vDecoder = avcodec_find_decoder(m_vDecContext->codec_id);
				if(tryOpenDecoder){
					tryOpenDecoder = 0;
					continue;
				}
#endif
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_open2 failed,video_codec_id=%d", m_vDecContext->codec_id);
				return VE_ERR_OPEN_DECODER_FAILED;
			}
			break;
		}while(1);

		m_vTimebase = m_fmtCtx->streams[m_videoStreamIndex]->time_base;
        
        
        m_vDuration = av_rescale_q(m_fmtCtx->streams[m_videoStreamIndex]->duration,m_vTimebase,(AVRational){ 1, 1000 });


		if(m_fmtCtx->streams[m_videoStreamIndex]->codec->codec_id ==AV_CODEC_ID_H264){
			m_h264Mp4ToAnnexBBsfc = new VEBitstream("h264_mp4toannexb");
		}else if(AV_CODEC_ID_HEVC == m_fmtCtx->streams[m_videoStreamIndex]->codec->codec_id && !strcmp(m_vDecoder->name,"hevc_videotoolbox_dec")){
			m_h264Mp4ToAnnexBBsfc = new VEBitstream("hevc_mp4toannexb");
		}
		if(m_clipData.m_clip.type == VE_CLIP_VIDEO){
			m_clipDuration = m_clipData.m_clip.end_time - m_clipData.m_clip.start_time;
		}else{
			m_clipDuration = m_clipData.m_clip.duration;
		}
	}
    
    m_firstVPktPts = m_firstAPktPts = -1;
    
    if(m_clipData.m_clip.start_time){
        m_seekTo = m_clipData.m_clip.start_time;
        if(m_clipData.m_clip.slv.active){
        	m_seekTo = m_clipData.m_clip.slv.clip_start_time;
        }
    }

    if(m_clipData.m_clip.type == VE_CLIP_VIDEO || m_clipData.m_clip.type == VE_CLIP_AUDIO){
    	m_seek = 1;
    	/*
    	if(m_clipData.m_clip.duration > m_clipData.m_clip.end_time - m_clipData.m_clip.start_time){
    		m_loop = 1;
    	}
    	*/
    }

    ve_dec_yuv_buffer yuv;
    ve_dec_pcm_buffer pcm;
    ve_audio_filter_param afParam;
    int i;
    if(m_clipData.m_clip.type == VE_CLIP_VIDEO){
    	for(i=0;i<VE_MAX_YUV_BUFFER_NUM;i++){
    		m_yuvPool.push_back(yuv);
    		m_yuvPool.post();
    	}
    }else if(m_clipData.m_clip.type == VE_CLIP_PICTURE){
    	m_yuvPool.push_back(yuv);
    	m_yuvPool.post();
    }else if(m_clipData.m_clip.type == VE_CLIP_AUDIO){
    	for(i=0;i<VE_MAX_PCM_BUFFER_NUM;i++){
    		m_pcmPool.push_back(pcm);
    		m_pcmPool.post();
    	}

    	char expr[256];

    	m_audioFilters = new VEAudioFilter();

    	afParam.m_volume = m_clipData.m_actualVolume;
    	afParam.m_samplerateForSpeed = m_aDecContext->sample_rate * m_clipData.m_actualSpeed;

    	getAudioFilterString(&afParam,expr);
    	m_audioFilters->addFilters(expr);

		if(m_audioFilters){

			if(m_audioFilters->init(m_aDecContext)){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_audioFilters->Init failed!");
				return VE_ERR_CREATE_FILTER_FAILED;
			}
		}
    }

	return VE_ERR_OK;
}
VE_ERR VESource::init(){

	VE_ERR ret;

	if(!isOpen()){
		ret = open();
		if(ret)return ret;
	}

	return VE_ERR_OK;
}
VE_ERR VESource::release(){



	while(!m_yuvPool.empty()){
		ve_dec_yuv_buffer yuv = m_yuvPool.front();
		m_yuvPool.pop();
#ifdef __ANDROID__
        if(yuv.m_len){
            delete[] yuv.m_data;
        }
#endif
	}
	while(!m_yuvQueue.empty()){
		ve_dec_yuv_buffer yuv = m_yuvQueue.front();
		m_yuvQueue.pop();
#ifdef __ANDROID__
		if(yuv.m_len){
			delete[] yuv.m_data;
		}
#endif
#ifdef __APPLE__
        if(yuv.m_format == VE_COLOR_IOS_PIXELBUFFER){
                CVPixelBufferRelease(CVImageBufferRef(yuv.m_data));
        }
#endif
	}

	while(!m_pcmQueue.empty()){
		ve_dec_pcm_buffer pcm = m_pcmQueue.front();
		m_pcmQueue.pop();
		if(pcm.m_len){
			delete[] pcm.m_data;
		}
	}
	while(!m_pcmPool.empty()){
		ve_dec_pcm_buffer pcm = m_pcmPool.front();
		m_pcmPool.pop();
		if(pcm.m_len){
			delete[] pcm.m_data;
		}
	}

	while(!m_aPktQueue.empty()){
		VEPacket  pkt = m_aPktQueue.front();
		m_aPktQueue.pop();
		if(pkt.m_pkt.size){
			av_packet_unref(&pkt.m_pkt);
		}
	}

	while(!m_vPktQueue.empty()){
		VEPacket pkt = m_vPktQueue.front();
		m_vPktQueue.pop();
		if(pkt.m_pkt.size){
			av_packet_unref(&pkt.m_pkt);
		}
	}

	if(!m_aDecContext){
		avcodec_close(m_aDecContext);
		m_aDecContext = NULL;
	}

	if(!m_vDecContext){
		avcodec_close(m_vDecContext);
		m_vDecContext = NULL;
	}
	if(m_fmtCtx){
		avformat_close_input(&m_fmtCtx);
		m_fmtCtx = 0;
	}
	if(m_aFrame){
		av_frame_free(&m_aFrame);
		m_aFrame = NULL;
	}
	if(m_vFrame){
		av_frame_free(&m_vFrame);
		m_vFrame = NULL;
	}
	if(m_h264Mp4ToAnnexBBsfc){
		delete m_h264Mp4ToAnnexBBsfc;
		m_h264Mp4ToAnnexBBsfc = 0;
	}
	if(m_swsscale){
		delete m_swsscale;
		m_swsscale = 0;
	}
	if(m_yuv){
		delete [] m_yuv;
	}
	if(m_audioFilters){
		delete m_audioFilters;
		m_audioFilters = 0;
	}
	if(m_pcmBuffer){
		delete m_pcmBuffer;
		m_pcmBuffer = 0;
	}

    if(m_soundTouch)
    {
        delete m_soundTouch;
        m_soundTouch = NULL;
    }
    if(m_soundTouchBuffer){
    	delete m_soundTouchBuffer;
    	m_soundTouchBuffer = NULL;
    }

    if(m_vtMode){
    	delete m_vtMode;
    	m_vtMode = NULL;
    }

	return VE_ERR_OK;
}

VE_ERR VESource::readPacket(){

	 	char err[256] = {0};
        int ret,eof = 0;
        int64_t offset;

        int readGap = 500;

        int startTime,endTime;

        VEPacket vePkt;

		ve_clip*	clip = &m_clipData.m_clip;
		ve_clip_info & info = m_clipData.m_clip.info;


		if(clip->slv.active){
			readGap = 100;
			startTime = clip->slv.clip_start_time;
			endTime = clip->slv.clip_end_time;
		}else{
			startTime = clip->start_time;
			endTime = clip->end_time;
		}
		if(m_eof){
			av_usleep(40000);
			return VE_ERR_OK;
		}

		//读包，判断文件结束，seek

		AVPacket pkt,pkt2;
		av_init_packet(&pkt);
		av_init_packet(&pkt2);
		pkt2.data = NULL;
		pkt2.size = 0;

		int aPktCount = m_aPktQueue.size();
		int vPktCount = m_vPktQueue.size();


		//队列少于50，开始读包

		if(vPktCount > 50 || aPktCount > 50){
			av_usleep(40000);
			return VE_ERR_OK;
		}

		//读一个packet，计算相关变量参数
		while (1) {
			pkt.data = NULL;
			pkt.size = 0;

			ret = av_read_frame(m_fmtCtx, &pkt);

			if(0 == ret && pkt.stream_index != m_videoStreamIndex && pkt.stream_index != m_audioStreamIndex){
				av_packet_unref(&pkt);
				continue;
			}

			//计算当前packet偏移位置单位ms，判断切片是否结束
			if(m_foundFirstPktPts){

				if(VE_SOURCE_TYPE_VIDEO == m_type && pkt.stream_index == m_videoStreamIndex){

					offset = av_rescale_q(pkt.pts - m_firstVPktPts,m_fmtCtx->streams[m_videoStreamIndex]->time_base,(AVRational){ 1, 1000 });
					offset += m_firstPktPtsInMs;

				}else if(VE_SOURCE_TYPE_AUDIO == m_type && pkt.stream_index == m_audioStreamIndex){

					offset = av_rescale_q(pkt.pts - m_firstAPktPts,m_fmtCtx->streams[m_audioStreamIndex]->time_base,(AVRational){ 1, 1000 });
					offset += m_firstPktPtsInMs;

				}else{
					offset = 0;
				}
			}else{
				offset = 0;
			}
			if(clip->slv.active){
				m_offset = m_loopCount * m_clipDuration + VEConfig::getSlvDuration(&clip->slv,clip->slv.clip_start_time,offset);
			}else{
				m_offset = m_loopCount * m_clipDuration + offset - startTime;
			}

			if(ret < 0 || !m_loop && offset && offset > endTime + readGap || m_loop && m_offset > clip->duration + readGap){

				/*
				if(m_loop){
					if(m_offset < clip->duration){
						m_loopCount++;
						m_offset = m_loopCount * m_clipDuration;
						//loop
						m_seek = 1;
					}else{
						eof = 1;
					}
				}else
				*/
				{
					m_offset = m_clipDuration;
					eof = 1;
				}

				if(eof){
					av_packet_unref(&pkt);
					break;
				}
			}

			//查找音视频起始包时间

			if(pkt.stream_index == m_videoStreamIndex && m_firstVPktPts < 0){
				if(pkt.pts < 0){
					pkt.pts = 0;
				}
				m_firstVPktPts = pkt.pts;
				m_firstVPktPtsInMs = av_rescale_q(m_firstVPktPts,m_fmtCtx->streams[m_videoStreamIndex]->time_base,(AVRational){ 1, 1000 });

				if(m_firstPktPtsInMs == -1){
					m_firstPktPtsInMs = m_firstVPktPtsInMs;
				}else if(m_firstVPktPtsInMs < m_firstPktPtsInMs){
					m_firstPktPtsInMs = m_firstVPktPtsInMs;
				}

			}else if(pkt.stream_index == m_audioStreamIndex && m_firstAPktPts < 0){
				if(pkt.pts < 0){
					pkt.pts = 0;
				}
				m_firstAPktPts = pkt.pts;
				m_firstAPktPtsInMs = av_rescale_q(m_firstAPktPts,m_fmtCtx->streams[m_audioStreamIndex]->time_base,(AVRational){ 1, 1000 });
				if(m_firstPktPtsInMs == -1){
					m_firstPktPtsInMs = m_firstAPktPtsInMs;
				}else if(m_firstAPktPtsInMs < m_firstPktPtsInMs){
					m_firstPktPtsInMs = m_firstAPktPtsInMs;
				}
			}

			if(m_firstVPktPts < 0 && m_videoStreamIndex >= 0){
				av_packet_unref(&pkt);
				continue;
			}
			if(m_firstAPktPts < 0 && m_audioStreamIndex >= 0){
				av_packet_unref(&pkt);
				continue;
			}
			//查找起始包时间
			if(m_videoStreamIndex >= 0 && m_firstVPktPts >= 0 && m_audioStreamIndex >= 0 && m_firstAPktPts >= 0){
				m_foundFirstPktPts = 1;
			}else if(m_videoStreamIndex >= 0 && m_firstVPktPts >= 0 && m_audioStreamIndex < 0){
				m_foundFirstPktPts = 1;
			}else if(m_audioStreamIndex >= 0 && m_firstAPktPts >= 0 && m_videoStreamIndex < 0){
				m_foundFirstPktPts = 1;
			}

			if(!m_foundFirstPktPts){
				av_packet_unref(&pkt);
				continue;
			}

			//seek

			if(m_seek){
				int seek_stream_index = VE_SOURCE_TYPE_AUDIO == m_type ? m_audioStreamIndex:m_videoStreamIndex;
				int64_t seek_to_time = av_rescale_q(m_seekTo,(AVRational){ 1, 1000 },m_fmtCtx->streams[seek_stream_index]->time_base);
				av_packet_unref(&pkt);

				if(seek_stream_index == m_videoStreamIndex && m_fmtCtx->streams[m_videoStreamIndex]->index_entries){
					seek_to_time += m_fmtCtx->streams[m_videoStreamIndex]->index_entries[0].timestamp;
				}

				ret = av_seek_frame(m_fmtCtx,seek_stream_index,seek_to_time,AVSEEK_FLAG_BACKWARD);
				if(ret < 0){
					//VE_LOG_ERROR("VESource::read_packet av_seek_frame failed,filename=%s",clip->filename);
					return VE_ERR_AV_SEEK_FRAME_FAILED;
				}
				m_seek = 0;
				continue;
			}

			if(m_type == VE_SOURCE_TYPE_AUDIO && pkt.stream_index == m_videoStreamIndex){
				av_packet_unref(&pkt);
				continue;
			}

			if((m_type == VE_SOURCE_TYPE_VIDEO || m_type == VE_SOURCE_TYPE_PICTURE) && pkt.stream_index == m_audioStreamIndex){
				av_packet_unref(&pkt);
				continue;
			}
			if(pkt.size == 0){
				av_packet_unref(&pkt);
				continue;
			}
			break;
		}//while


		//读到一个包
		if(!eof){

			if((m_type == VE_SOURCE_TYPE_VIDEO || m_type == VE_SOURCE_TYPE_PICTURE) && pkt.stream_index == m_videoStreamIndex){
				vePkt.m_pkt = pkt;
				vePkt.m_loopCount = m_loopCount;
				m_vPktQueue.push_back(vePkt);

			}else if(m_type == VE_SOURCE_TYPE_AUDIO && pkt.stream_index == m_audioStreamIndex){
				vePkt.m_pkt = pkt;
				vePkt.m_loopCount = m_loopCount;
				m_aPktQueue.push_back(vePkt);
			}else{
				//empty
			}

		}else{
			if((m_type == VE_SOURCE_TYPE_VIDEO || m_type == VE_SOURCE_TYPE_PICTURE)){
				vePkt.m_pkt = pkt2;
				vePkt.m_loopCount = m_loopCount;
				m_vPktQueue.push_back(vePkt);

			}else if(m_type == VE_SOURCE_TYPE_AUDIO){
				vePkt.m_pkt = pkt2;
				vePkt.m_loopCount = m_loopCount;
				m_aPktQueue.push_back(vePkt);
			}else{
				//empty
			}
			m_eof = 1;
		}


	return VE_ERR_OK;
}
VE_ERR VESource::videoProcess(){

	VEPacket pkt;

    VE_ERR ret;
    int gotPicture;
	int flushDecoder = 0;

	if(m_vPktQueue.empty()){
		av_usleep(30000);
		return VE_ERR_OK;
	}

	pkt = m_vPktQueue.front();
	m_vPktQueue.pop_front();

	flushDecoder = pkt.m_pkt.size ==  0 ? 1: 0;

    if(m_h264Mp4ToAnnexBBsfc && !flushDecoder){
        m_h264Mp4ToAnnexBBsfc->applyBitstreamFilter(m_vDecContext,&pkt.m_pkt);
    }
	if(m_clipData.m_clip.type == VE_CLIP_PICTURE && flushDecoder){
		return VE_ERR_OK;
	}

    do{
		do{
			
			if(avcodec_decode_video2(m_vDecContext, m_vFrame, &gotPicture, &pkt.m_pkt) < 0){
				//return VE_ERR_DECODE_FAILED;
			}
		}while(m_clipData.m_clip.type == VE_CLIP_PICTURE && !gotPicture);//for picture

		if(gotPicture){
			ret = prepareYuv(pkt.m_loopCount);
			if(ret){
				return ret;
			}
		}
		if(!flushDecoder){
			av_packet_unref(&pkt.m_pkt);
		}
    }while(!m_exit && m_clipData.m_clip.type == VE_CLIP_VIDEO && flushDecoder && gotPicture);//for video flush decoder

    if(flushDecoder && !gotPicture){
    	m_flushDecoderDone = 1;
    	m_yuvQueue.post();
    	m_yuvPool.post();
    }

	return VE_ERR_OK;
}
VE_ERR VESource::audioProcess(){

	VEPacket pkt;

    VE_ERR ret;
    int gotPcm;
	int flushDecoder = 0;

	if(m_aPktQueue.empty()){
		av_usleep(30000);
		return VE_ERR_OK;
	}

	pkt = m_aPktQueue.front();
	m_aPktQueue.pop_front();

	flushDecoder = pkt.m_pkt.size ==  0 ? 1: 0;

	if(!flushDecoder){
		do{

			if(avcodec_decode_audio4(m_aDecContext, m_aFrame, &gotPcm, &pkt.m_pkt) < 0){
				return VE_ERR_DECODE_FAILED;
			}

			if(gotPcm){
				if(m_firstPcm){
					m_firstPcm = 0;
				}else{
					ret = preparePcm(pkt.m_loopCount);
					if(ret){
						return ret;
					}
				}
			}
			if(!flushDecoder){
				av_packet_unref(&pkt.m_pkt);
			}
		}while(!m_exit && flushDecoder && gotPcm);//for video flush decoder
	}else{
		gotPcm = 0;
	}

    if(flushDecoder && !gotPcm){
    	m_flushDecoderDone = 1;
    	m_pcmQueue.post();
    	m_pcmPool.post();

    }

	return VE_ERR_OK;
}
VE_ERR VESource::refreshAudioFilters(int curTime,int slvCurTime){

	std::map<int,VEFilterData>::iterator aFilterIt = m_clipData.m_filters.begin();
	VEFilterData* aFilter = NULL;
	ve_audio_filter_param afParam;
	int ret;
	char expr[256];
	float slvActualSpeed = 1.0;
	int vCurTime = curTime;

	if(slvCurTime != -1){
		vCurTime = slvCurTime;
	}

	if(m_clipData.m_filters.size()){
		for(aFilterIt;aFilterIt != m_clipData.m_filters.end();aFilterIt++){
			aFilter = &aFilterIt->second;

			if(aFilter->m_filter.type == VE_FILTER_AUDIO && vCurTime >= aFilter->m_filter.start_time && vCurTime <= aFilter->m_filter.end_time){
				break;
			}
			aFilter = NULL;
		}
	}

	afParam.m_volume = m_clipData.m_actualVolume;
	if(slvCurTime == -1){
	}else{
		slvActualSpeed = VEConfig::getCurSlvSpeed(&m_clipData.m_clip.slv,curTime);
	}
	afParam.m_samplerateForSpeed = m_aDecContext->sample_rate * m_clipData.m_actualSpeed * slvActualSpeed;
	 if(aFilter){
		 int64_t startSamples = ((int64_t)aFilter->m_filter.start_time + m_clipData.m_clip.start_time) * m_aDecContext->sample_rate  / 1000;
		 int64_t nbSamples = ((int64_t)(aFilter->m_filter.end_time - aFilter->m_filter.start_time)) * m_aDecContext->sample_rate  / 1000;
		 afParam.m_gainMin = aFilter->m_filter.gain_min;
		 afParam.m_gainMax = aFilter->m_filter.gain_max;
		 afParam.m_fade = aFilter->m_filter.af_type;
		 afParam.m_startSamples = startSamples;
		 afParam.m_nbSamples = nbSamples;
		 getAudioFilterString(&afParam,expr);
	 }else{
		getAudioFilterString(&afParam,expr);
	 }

	if(!m_audioFilters->isSameFilters(expr)){
		delete m_audioFilters;
		m_audioFilters = NULL;
		if(!m_audioFilters){

			m_audioFilters = new VEAudioFilter();
			/*
			 if(!m_audioFilters){
				 return VE_ERR_MALLOC_FAILED;
			 }
			 */
			 m_aDecContext->time_base = m_aTimebase;
			 ret = m_audioFilters->addFilters(m_aDecContext,expr);
			 if(ret){
				 VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_audioFilters->AddFilters failed!");
				 return VE_ERR_CREATE_FILTER_FAILED;
			 }
		}
	}
	return VE_ERR_OK;
}
VE_ERR VESource::preparePcm(int loopCount){

	VE_ERR ret;
	int filteredSize;
	AVFrame frame;
	int offset,offsetCur;
	int outSamples;
	short* filteredData;
	int bytesPerSample = av_get_bytes_per_sample(m_samplefmt) * m_channels;
	int slvOffsetCur = -1;
	ve_dec_pcm_buffer pcm;
	int startTime,endTime;
	//resample

	if(!m_pcmBuffer){
	    m_pcmBufferLen = av_samples_get_buffer_size(NULL, m_channels, m_samplerate, m_samplefmt, 0);
	    m_pcmBuffer = new uint8_t[m_pcmBufferLen];
	}

	frame = *m_aFrame;

	offset = av_rescale_q(frame.pkt_pts - m_firstAPktPts,m_fmtCtx->streams[m_audioStreamIndex]->time_base,(AVRational){ 1, 1000 });
	offset += m_firstAPktPtsInMs - m_firstPktPtsInMs;

	offsetCur = offset;

	if(m_clipData.m_clip.slv.active){

		offset -= m_clipData.m_clip.slv.clip_start_time;
		startTime = m_clipData.m_clip.slv.clip_start_time;
		endTime = m_clipData.m_clip.slv.clip_end_time;
		slvOffsetCur = VEConfig::getSlvTime(&m_clipData.m_clip.slv,offsetCur);
		ret = refreshAudioFilters(offsetCur,slvOffsetCur);
	}else{
		offset -= m_clipData.m_clip.start_time;
		startTime = m_clipData.m_clip.start_time;
		endTime = m_clipData.m_clip.end_time;
		ret = refreshAudioFilters(offsetCur);
	}

	if(offsetCur < startTime || offsetCur > endTime){
		return VE_ERR_OK;
	}

	if(ret){
		return ret;
	}

	ret = (VE_ERR)m_audioFilters->process(m_aFrame,&filteredData,&filteredSize);
    
	if(ret){
		return ret;
	}

	if(filteredSize){

		if(m_clipData.m_clip.pitch && !m_soundTouch){
					if(m_soundTouch == NULL)
					{
						m_soundTouch = new VESoundTouch();
					}
					if(m_soundTouchBuffer == NULL)
					{
						m_soundTouchBuffer = new uint8_t[1024*1024];
					}
					m_soundTouch->configSoundTouchPitch(m_clipData.m_clip.pitch, m_channels, m_samplerate);
		}

		if(m_clipData.m_clip.pitch && m_soundTouch){
			m_soundTouch->processData((uint8_t*)filteredData, filteredSize, m_soundTouchBuffer, &filteredSize);
			filteredData = (short*)m_soundTouchBuffer;
		}

		memcpy(m_pcmBuffer + m_pcmPos,filteredData,filteredSize);
	}
    outSamples = filteredSize / bytesPerSample;

    m_pcmLeft += outSamples * bytesPerSample;


    m_pcmPos =0;
    while(m_pcmLeft >= m_outputPcmLen){

    	m_pcmPool.wait();
    	if(!m_pcmPool.size() || m_exit){
    		return VE_ERR_OK;
    	}
    	pcm = m_pcmPool.front();
    	m_pcmPool.pop();


    	if(pcm.m_len < m_outputPcmLen){
    		delete[] pcm.m_data;
    		pcm.m_len = m_outputPcmLen;
    		pcm.m_data = new uint8_t[pcm.m_len];
    	}
    	memcpy(pcm.m_data,m_pcmBuffer + m_pcmPos,m_outputPcmLen);

    	if(m_curPcmPts == -1){
    		m_curPcmPts = 0;
    	}else{
    		m_curPcmPts += 1000000 * (uint64_t)(m_outputPcmLen / bytesPerSample) / m_samplerate;
    	}
    	pcm.m_ts = m_curPcmPts;

		m_pcmLeft -= m_outputPcmLen;
		m_pcmPos += m_outputPcmLen;

		m_pcmQueue.push_back(pcm);
		m_pcmQueue.post();

	}

	memmove(m_pcmBuffer,m_pcmBuffer + m_pcmPos,m_pcmLeft);
	m_pcmPos = m_pcmLeft;

	return VE_ERR_OK;
}
VE_ERR VESource::prepareYuv(int loopCount){

    int width,height;
	int yLength,yuvI420Length;
    ve_dec_yuv_buffer yuv;
    AVFrame frame;
    AVFrame *vFrameI420 = NULL;
    uint8_t* i420Out;
	int scaleH,scaleW,scaleMax;
	int defaultColor = DEFAULT_COLOR;
	VE_ERR ret = VE_ERR_OK;
	int offset,offsetCur,clipTs;
    int prevDuration;
    int drop = 0;
    int startTime,endTime;

	//颜色空间

	offset = av_rescale_q(m_vFrame->pkt_pts - m_firstVPktPts,m_fmtCtx->streams[m_videoStreamIndex]->time_base,(AVRational){ 1, 1000 });
	offset += m_firstVPktPtsInMs - m_firstPktPtsInMs;

	offsetCur = offset;

	if(m_clipData.m_clip.slv.active){

		startTime = m_clipData.m_clip.slv.clip_start_time;
		endTime = m_clipData.m_clip.slv.clip_end_time;
		clipTs = VEConfig::getSlvDuration(&m_clipData.m_clip.slv,startTime,offset);
		offset -= m_clipData.m_clip.slv.clip_start_time;
	}else{

        if(m_clipData.m_clip.type == VE_CLIP_PICTURE){
            
            startTime = 0;
            endTime = m_clipData.m_clip.duration;
        }else{
            offset -= m_clipData.m_clip.start_time;
            startTime = m_clipData.m_clip.start_time;
            endTime = m_clipData.m_clip.end_time;
        }
		clipTs = offset;
	}


    if(m_clipData.m_clip.slv.active){
        offset = VEConfig::getSlvDuration(&m_clipData.m_clip.slv,startTime,offsetCur) / m_clipData.m_actualSpeed;
        offset += (m_clipDuration * loopCount) / m_clipData.m_actualSpeed;
    }else{
        offset += m_clipDuration * loopCount;
        offset /= m_clipData.m_actualSpeed;
    }

	if(m_curVCount){

		int sample_time = m_curVCount * 33;

		if(offset >= sample_time || fabs(sample_time - offset) < 16){
			//pass
		}else{
			drop = 1;
		}
	}

	

	if(drop || offsetCur < startTime || offsetCur + 40> endTime){
#ifdef __APPLE__
		if((m_vDecoder == &h264_videotoolbox_decoder || m_vDecoder == &hevc_videotoolbox_decoder) && m_vFrame->linesize[7] == AV_PIX_FMT_VIDEOTOOLBOX){
			CVPixelBufferRelease(CVImageBufferRef(m_vFrame->data[7]));
		}
#endif
		return VE_ERR_OK;
	}







	VESwsscale::getOpenGLWH(m_vFrame->width,m_vFrame->height,&scaleW,&scaleH);
    do{
       	if(AV_PIX_FMT_YUVA420P == m_vFrame->format || m_vFrame->format >= AV_PIX_FMT_YUVA422P && m_vFrame->format <=AV_PIX_FMT_YUVA444P16LE ||
                    m_vFrame->format >= AV_PIX_FMT_ARGB && m_vFrame->format <= AV_PIX_FMT_ABGR ||
        			m_vFrame->format >= AV_PIX_FMT_RGBA64BE && m_vFrame->format <= AV_PIX_FMT_BGRA64LE ||
        			m_vFrame->format >= AV_PIX_FMT_GBRAP && m_vFrame->format <= AV_PIX_FMT_GBRAP16LE ||
        			m_vFrame->format >= AV_PIX_FMT_AYUV64LE && m_vFrame->format <= AV_PIX_FMT_AYUV64BE ||
        			m_vFrame->format >= AV_PIX_FMT_GBRAP12BE && m_vFrame->format <= AV_PIX_FMT_GBRAP10LE ||
        			m_vFrame->format >= AV_PIX_FMT_GBRAPF32BE && m_vFrame->format <= AV_PIX_FMT_GBRAPF32LE){

        		defaultColor = AV_PIX_FMT_BGRA;
        	}
	#ifdef __APPLE__



		if((m_vDecoder == &h264_videotoolbox_decoder || m_vDecoder == &hevc_videotoolbox_decoder) && m_vFrame->linesize[7] == AV_PIX_FMT_VIDEOTOOLBOX){
				frame =  *m_vFrame;
				break;

		}else if(m_vFrame->format != defaultColor || scaleW != m_vFrame->width || scaleH != m_vFrame->height){
	#else
		if(m_vFrame->format != defaultColor || scaleW != m_vFrame->width || scaleH != m_vFrame->height){
	#endif
			VE_LOG_TAG_INFO(VE_MODULE_TAG,"need sws_scale m_vFrame->format=%d",m_vFrame->format);

			if(!m_swsscale){
				m_swsscale = new VESwsscale();
			}
			m_swsscale->process(m_vFrame,&vFrameI420,defaultColor,scaleW,scaleH);

			frame = *vFrameI420;

		}else{
			frame = *m_vFrame;
		}



	}while(0);

    width = frame.width;
    height = frame.height;
        
	yLength = frame.width * frame.height;
	if(defaultColor == AV_PIX_FMT_BGRA){
		yuvI420Length = yLength * 4;
	}else{
		yuvI420Length = yLength * 3/2;
	}

#ifdef __ANDROID__

	if(m_yuvLen < yuvI420Length){
		delete [] m_yuv;
		m_yuvLen = 0;
		m_yuv = NULL;
	}
	if(!m_yuv){
		m_yuv = new uint8_t[yuvI420Length];
		if(!m_yuv){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_yuv = new uint8_t failed!");
			ret = VE_ERR_MALLOC_FAILED;
			goto err_exit;
		}
		m_yuvLen = yuvI420Length;
	}

#endif
	i420Out = (uint8_t*)m_yuv;
#ifdef __ANDROID__
	if(defaultColor == AV_PIX_FMT_BGRA){
		memcpy(m_yuv,frame.data[0],frame.linesize[0] * frame.height);
	}else{
		libyuv::I420Copy((uint8_t*)frame.data[0],frame.linesize[0],(uint8_t*)frame.data[1],frame.linesize[1],(uint8_t*)frame.data[2],frame.linesize[2],i420Out, frame.width,
						 i420Out + yLength, frame.width / 2,
						 i420Out + yLength * 5 / 4, frame.width / 2,
						 frame.width, frame.height);
	}
#endif

    m_yuvPool.wait();

    if(!m_yuvPool.size()){
    	return VE_ERR_OK;
    }

    yuv = m_yuvPool.front();
    m_yuvPool.pop();

    //prevDuration =
    if(m_type == VE_SOURCE_TYPE_VIDEO){
		yuv.m_ts = offset + m_clipData.m_insertTime;
		yuv.m_clipTs = clipTs;
    }else{
    	yuv.m_ts = m_clipData.m_insertTime;
    }


#ifdef __ANDROID__
	if(yuv.m_len < yuvI420Length){

		delete [] yuv.m_data;
		yuv.m_len = yuvI420Length;
		yuv.m_data = new uint8_t[yuvI420Length];

		if(!yuv.m_data){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"yuv->data = new uint8_t[yuvI420Length] failed!");
			ret = VE_ERR_MALLOC_FAILED;
			goto err_exit;
		}
	}


	memcpy(yuv.m_data,i420Out,yuvI420Length);
	if(defaultColor == AV_PIX_FMT_BGRA){
		yuv.m_format = VE_COLOR_BGRA;
	}else{
		yuv.m_format = VE_COLOR_YUV420P;
	}

	yuv.m_len = yuvI420Length;
#endif
#ifdef __APPLE__
	if(defaultColor == AV_PIX_FMT_YUVA444P){
		GetPixelBufferFromRGBA(frame.format,width,height,frame.data,frame.linesize,(void**)&yuv.m_data);
		yuv.m_format = VE_COLOR_IOS_PIXELBUFFER;
		yuv.m_len = yuvI420Length;

		yuv.m_refCount = 1;
	}else if(frame.format == AV_PIX_FMT_ARGB || frame.format == AV_PIX_FMT_RGBA || frame.format == AV_PIX_FMT_ABGR || frame.format == AV_PIX_FMT_BGRA){
		GetPixelBufferFromRGBA(frame.format,width,height,frame.data,frame.linesize,(void**)&yuv.m_data);
		yuv.m_format = VE_COLOR_IOS_PIXELBUFFER;
		yuv.m_len = yuvI420Length;

		yuv.m_refCount = 1;
	}else if(frame.linesize[7] == AV_PIX_FMT_VIDEOTOOLBOX){

    	yuv.m_data = frame.data[7];
    	yuv.m_format = VE_COLOR_IOS_PIXELBUFFER;
    	yuv.m_len = yuvI420Length;
    	yuv.m_refCount = 1;
    }else if(frame.format == AV_PIX_FMT_NV12){

        GetPixelBufferFromNV12(width,height,frame.data[0],frame.data[1],frame.linesize[0],(void**)&yuv.m_data);
		yuv.m_format = VE_COLOR_IOS_PIXELBUFFER;
		yuv.m_len = yuvI420Length;

		yuv.m_refCount = 1;

    }
#endif

	yuv.m_width = frame.width;
	yuv.m_height = frame.height;
	if(m_clipData.m_clip.type == VE_CLIP_PICTURE && m_clipData.m_clip.picture_rotate){
		yuv.m_rotate = m_clipData.m_clip.picture_rotate;
	}else{
		yuv.m_rotate = m_clipData.m_clip.info.rotate;
	}


	m_yuvQueue.push_back(yuv);
	m_yuvQueue.post();

	return VE_ERR_OK;

	err_exit:
	return ret;
}
VESource::VESource(VE_SOURCE_TYPE type,VEConfig *config,VEClipData* clipData,VESourceListener* listener){

	m_type = type;

	m_config = config;

	m_clipData = *clipData;

	m_listener = listener;

	m_vtMode = (void*)new VEVTMode();
	((VEVTMode*)m_vtMode)->m_mode = VE_VT_MODE_EXPORT;
}
VESource::~VESource(){
	stop();
}
int VESource::getClipId(){
	return m_clipData.m_clip.clip_id;
}
VE_ERR VESource::start(){

	VE_ERR ret;

	comn::AutoCritSec lock(m_cs);

	if(!m_exit){
		return VE_ERR_OK;
	}


	ret = init();

	if(ret){
		m_status = ret;
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"error for track_id:%d,index:%d,filename:%d",m_clipData.m_clip.track_id,m_clipData.m_index,m_clipData.m_filename.size()?m_clipData.m_filename.c_str():"");
		return ret;
	}
    
    m_exit = 0;

    pthread_create(&m_readThread, NULL, veSourceReadpacketProcess, (void *)this);
	/*
    if(0 != pthread_create(&m_readThread, NULL, veSourceReadpacketProcess, (void *)this)){
		VE_LOG_ERROR("VESource::start pthread_create veSourceReadpacketProcess failed!");
		return VE_ERR_CREATE_THREAD_FAILED;
	}
     */


	if(m_clipData.m_clip.info.v_codec_id && (m_clipData.m_clip.type == VE_CLIP_VIDEO || m_clipData.m_clip.type == VE_CLIP_PICTURE)){
        pthread_create(&m_videoThread, NULL, veSourceVideoProcess, (void *)this);
        /*
		if(0 != pthread_create(&m_videoThread, NULL, veSourceVideoProcess, (void *)this)){
			VE_LOG_ERROR("VESource::start pthread_create veSourceVideoProcess failed!");
			return VE_ERR_CREATE_THREAD_FAILED;
		}
         */
	}
	if(m_clipData.m_clip.info.a_codec_id && m_clipData.m_clip.type == VE_CLIP_AUDIO){
        pthread_create(&m_audioThread, NULL, veSourceAudioProcess, (void *)this);
        /*
		if(0 != pthread_create(&m_audioThread, NULL, veSourceAudioProcess, (void *)this)){
	        VE_LOG_ERROR("VESource::start pthread_create veSourceAudioProcess failed!");
	        return VE_ERR_CREATE_THREAD_FAILED;
		}
         */
	}

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"for track_id:%d,index:%d,filename:%d",m_clipData.m_clip.track_id,m_clipData.m_index,m_clipData.m_filename.size()?m_clipData.m_filename.c_str():"");

	return VE_ERR_OK;
}
VE_ERR VESource::stop(){

	VE_ERR ret;

	comn::AutoCritSec lock(m_cs);


	if(m_exit)return VE_ERR_OK;

    m_exit = 1;
    
	signal();


    if(m_readThread){
    	pthread_join(m_readThread, NULL);
    	m_readThread = 0;
    }

    if(m_videoThread){
    	pthread_join(m_videoThread, NULL);
    	m_videoThread = 0;
    }

    if(m_audioThread){
    	pthread_join(m_audioThread, NULL);
    	m_audioThread = 0;
    }

    ret = release();

    m_stop = 1;

    /*
	if(ret){
		m_status = ret;
    	VE_LOG_ERROR("VESource::stop error for track_id:%d,index:%d,filename:%d",m_clipData.m_clip.track_id,m_clipData.m_index,m_clipData.m_filename.size()?m_clipData.m_filename.c_str():"");
    	return ret;
    }
	*/
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"for track_id:%d,index:%d",m_clipData.m_clip.track_id,m_clipData.m_index);
	return VE_ERR_OK;
}
void VESource::signal(){

	if(m_type == VE_SOURCE_TYPE_AUDIO){
		m_pcmQueue.post();
		m_pcmPool.post();
	}else{
		m_yuvQueue.post();
		m_yuvPool.post();

	}

}

VE_ERR VESource::getYuvBuffer(ve_dec_yuv_buffer* yuv,int *gotPicture,int* eof,int vCurTime){

	int preDuration;
	int offset;
	int transition = 0;
    int duration;
    int ajustTs = 0;
	int interval = 1000 / m_config->m_outputFps;

	*gotPicture = 0;

	if(m_status){
		return m_status;
	}

	if(m_flushDecoderDone && m_yuvQueue.size() == 0){
        *eof = 1;
		return VE_ERR_OK;
		
	}

	if(m_flushDecoderDone && m_clipData.m_clip.type == VE_CLIP_VIDEO && m_yuvQueue.size() == 1){
		ajustTs = 1;
	}


	if(m_clipData.m_clipArrangement == VE_CLIP_ARRANGEMENT_OVERLAY){
		preDuration = m_clipData.m_insertTime;

        duration = preDuration + m_config->getClipDuration(&m_clipData);
	}else{
        if(m_clipData.m_index == 0){
            preDuration = 0;
        }else{
            m_config->getTrackDuration(m_clipData.m_clip.track_id,m_clipData.m_index - 1,1,&preDuration);
        }

		offset = vCurTime - (preDuration - m_clipData.m_transitionDuration);
        if(offset < 0){
            return VE_ERR_OK;
        }
		if(offset >= 0 && offset < m_clipData.m_transitionDuration){
			transition = 1;
		}
        m_config->getTrackDuration(m_clipData.m_clip.track_id,m_clipData.m_index,1,&duration);
	}
	

	if(vCurTime >= (preDuration - m_clipData.m_transitionDuration) && vCurTime + interval <= duration){


		do{
			m_yuvQueue.wait();
			if(!m_yuvQueue.size()){
				*eof = 1;
				return VE_ERR_OK;
			}
			*yuv = m_yuvQueue.front();
			m_yuvQueue.pop();

			if(m_type == VE_SOURCE_TYPE_VIDEO){

				yuv->m_vCurTime = vCurTime;


				if(ajustTs){
					yuv->m_ts = duration - interval;
				}

				if(yuv->m_vCurTime - yuv->m_ts > 16 ){

	#ifdef __APPLE__
					CVPixelBufferRelease((CVImageBufferRef)yuv->m_data);
	#endif
					m_yuvBuffer = *yuv;
					returnYuv();
                    
					continue;
				}
				if(yuv->m_ts > yuv->m_vCurTime){
			#ifdef __APPLE__
					if(yuv->m_refCount == 1){
						CVPixelBufferRetain((CVImageBufferRef)yuv->m_data);
						yuv->m_refCount++;
					}
			#endif
				}
			}else if(m_type == VE_SOURCE_TYPE_PICTURE){
		#ifdef __APPLE__
				if(yuv->m_refCount == 1){
					CVPixelBufferRetain((CVImageBufferRef)yuv->m_data);
					yuv->m_refCount++;
				}
		#endif

				yuv->m_clipTs = (vCurTime - preDuration) * m_clipData.m_actualSpeed;
			}
			*gotPicture = 1;
			break;
		}while(1);
	}else if(vCurTime + interval > duration){
		*eof = 1;
		return VE_ERR_OK;
	}else{
		return VE_ERR_OK;
	}

	if(*gotPicture){
		if(transition){
			yuv->m_transitionFrame = 1;
			yuv->m_transitionAction = m_clipData.m_transitionAction;
			yuv->m_transitionId = m_clipData.m_transitionId;
		}
		m_yuvBuffer = *yuv;
		return VE_ERR_OK;
	}else{
		return VE_ERR_PARAM_ERR;
	}
}

VE_ERR VESource::getPcmBuffer(ve_dec_pcm_buffer* pcm,int* gotPcm,int* eof,int vCurTime){


	int preDuration;
	int duration = m_config->getClipDuration(&m_clipData);

	int interval = 1000 * (m_outputPcmLen >> 2) / m_samplerate;


	*gotPcm = 0;

	if(m_status){
		return m_status;
	}

	if(m_flushDecoderDone && m_pcmQueue.size() == 0){
		*eof = 1;
		return VE_ERR_OK;
	}


	if(m_clipData.m_clipArrangement == VE_CLIP_ARRANGEMENT_OVERLAY){
		preDuration = m_clipData.m_insertTime;

        duration = preDuration + m_config->getClipDuration(&m_clipData);
	}else{
        if(m_clipData.m_index == 0){
            preDuration = 0;
        }else{
            m_config->getTrackDuration(m_clipData.m_clip.track_id,m_clipData.m_index - 1,1,&preDuration);
        }

        m_config->getTrackDuration(m_clipData.m_clip.track_id,m_clipData.m_index,1,&duration);
	}

	if(VE_SOURCE_TYPE_AUDIO == m_type){

		if(vCurTime >= (preDuration - m_clipData.m_transitionDuration) && vCurTime + interval <= duration){
        	m_pcmQueue.wait();
        	if(!m_pcmQueue.size()){
        		*eof = 1;
        		return VE_ERR_OK;
        	}
        	m_purPcm = *pcm = m_pcmQueue.front();
            m_pcmQueue.pop();

        	*gotPcm = 1;

        	return VE_ERR_OK;
        }else if(vCurTime + interval > duration){
        	*eof = 1;
        	return VE_ERR_OK;
        }else{
        	return VE_ERR_OK;
        }
	}
    return VE_ERR_PARAM_ERR;
}
void VESource::returnYuv(){

	ve_dec_yuv_buffer* yuv = &m_yuvBuffer;
    
    
    
    yuv->m_transitionFrame = 0;
    yuv->m_transitionId = -1;
    yuv->m_transitionAction = NULL;
    
	if(yuv->m_format == VE_COLOR_IOS_PIXELBUFFER){
		yuv->m_refCount--;
	}
	if(m_type == VE_SOURCE_TYPE_PICTURE){
		m_yuvQueue.push_front(*yuv);
		m_yuvQueue.post();
	}else{

		if(yuv->m_vCurTime >= 0 && yuv->m_ts  > yuv->m_vCurTime){
			m_yuvQueue.push_front(*yuv);
			m_yuvQueue.post();
		}else{
			m_yuvPool.push(*yuv);
			m_yuvPool.post();
		}
	}
}

void VESource::returnPcm(){


	m_pcmPool.push(m_purPcm);
	m_pcmPool.post();
}
void* veSourceReadpacketProcess(void* context){

	VESource* source =(VESource*)context;
	VE_ERR ret;

	while(!source->m_exit){

		ret = source->readPacket();

		if(ret){

			source->m_status = ret;
			if(source->m_listener){
				source->m_listener->setStatus(ret);
			}
			source->m_exit = 1;
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"veSourceReadpacketProcess failed! ret = %d",ret);
			break;
		}
	}
	return 0;
}
void* veSourceVideoProcess(void* context){

	VESource* source =(VESource*)context;
	VE_ERR ret;

	while(!source->m_exit){

		ret = source->videoProcess();

		if(ret){

			source->m_status = ret;
			if(source->m_listener){
				source->m_listener->setStatus(ret);
			}
			source->m_exit = 1;
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"veSourceVideoProcess failed! ret = %d",ret);
			break;
		}
	}
	return 0;
}
void* veSourceAudioProcess(void* context){

	VESource* source =(VESource*)context;
	VE_ERR ret;

	while(!source->m_exit){

		ret = source->audioProcess();

		if(ret){

			source->m_status = ret;
			if(source->m_listener){
				source->m_listener->setStatus(ret);
			}
			source->m_exit = 1;
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"veSourceAudioProcess failed! ret = %d",ret);
			break;
		}
	}
	return 0;
}
