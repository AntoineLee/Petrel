#include "VEMp4Writer.h"
#ifdef __APPLE__
#include "ios+ver.h"
#include "VTDecoder.h"
#endif

#define VE_MODULE_TAG "[VEMp4Writer]"

#define BITRATE_720P	4500000
#define BITRATE_720P_Y_SIZE	777600
#define PCM_BUFFER_SIZE (1024 *64)
#define PCM_SAMPLES_PER_FRAME 1024
void* veMuxingProcess(void* context);
void* veVEncodingProcess(void* context);

VEMp4Writer::VEMp4Writer	(ve_mp4_writer* config){
	m_config = *config;
}
VEMp4Writer::~VEMp4Writer(){
	stop();
}
int VEMp4Writer::getYuvLen(){return m_filteredYuvQueue.size();}

float VEMp4Writer::getProgress(){return m_progress;};
void VEMp4Writer::complete(){m_completed = true;VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");}
void VEMp4Writer::vComplete(){m_vCompleted = true;if(VE_ENC_NONE == m_config.m_vConfig.m_type)m_flushEncoderDone = 1;VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");}
bool VEMp4Writer::IsComplete(){ return m_completed && m_vCompleted && m_flushEncoderDone && m_vMuxPktQueue.empty() && m_aMuxPktQueue.empty();}

VE_ERR VEMp4Writer::start(){

	VE_ERR ret;

	comn::AutoCritSec lock(m_cs);

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");
	if(!m_exit){
		return VE_ERR_OK;
	}

	ret = open();

	if(ret){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"open() ret = %d",ret);
		m_status = ret;
		return ret;
	}

	m_exit = 0;

	pthread_create(&m_muxingThread, NULL, veMuxingProcess, (void *)this);
	/*
	if(0 != pthread_create(&m_muxingThread, NULL, veMuxingProcess, (void *)this)){
		VE_LOG_ERROR("VEMp4Writer::Start pthread_create veMuxingProcess failed!");
		m_status = VE_ERR_CREATE_THREAD_FAILED;
		return m_status;
	}
	*/
	pthread_create(&m_vEncodingThread, NULL, veVEncodingProcess, (void *)this);
	/*
	if(0 != pthread_create(&m_vEncodingThread, NULL, veVEncodingProcess, (void *)this)){
		VE_LOG_ERROR("VEMp4Writer::Start pthread_create veVEncodingProcess failed!");
		m_status = VE_ERR_CREATE_THREAD_FAILED;
		return m_status;
	}
	*/
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"done");


    return VE_ERR_OK;
}
void VEMp4Writer::stop(){

	VE_ERR ret;

	comn::AutoCritSec lock(m_cs);

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	if(m_exit){
		return;
	}
	m_stop = true;

	if(m_completed){
		aEncode(true);

		if(m_config.m_vConfig.m_type != VE_ENC_NONE){//需要编码线程
			while(!m_status && !m_filteredYuvQueue.empty()){
				av_usleep(AV_USLEEP_GAP);
			}
			m_flushEncoder = 1;

			while(!m_status && !m_flushEncoderDone){
				av_usleep(AV_USLEEP_GAP);
			}

		}

		while(!m_aMuxPktQueue.empty() || !m_vMuxPktQueue.empty()){
			av_usleep(AV_USLEEP_GAP);
		}
	}



	m_exit = 1;

    if(m_vEncodingThread){
    	pthread_join(m_vEncodingThread, NULL);
    	m_vEncodingThread = 0;
    }

    if(m_muxingThread){
    	pthread_join(m_muxingThread, NULL);
    	m_muxingThread = 0;
    }

    this->close();

    VE_LOG_TAG_INFO(VE_MODULE_TAG,"done");
    return;
}
VE_ERR VEMp4Writer::writeExtradata(uint8_t* data,int len){

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"len=%d",len);

	//printf("m_vEncContext->extradata=%p,m_vEncContext->extradata_size=%d,len=%d",m_vEncContext->extradata,m_vEncContext->extradata_size,len);

	av_free(m_vEncContext->extradata);
	m_vEncContext->extradata = (uint8_t*)av_malloc(len + 32);

	memcpy(m_vEncContext->extradata,data,len);

	return VE_ERR_OK;

}
VE_ERR VEMp4Writer::write(ve_enc_yuv_buffer* yuv){

	if(m_status){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_status=%d",m_status);
		return m_status;
	}
	if(m_stop){
		return VE_ERR_OK;;
	}
	if(!yuv->m_data || !yuv->m_len){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!yuv->data || !yuv->len");
		return VE_ERR_OK;
	}
#ifdef __APPLE__
	if(yuv->m_format == VE_YUV_VT_PIXELBUFFER){
		m_filteredYuvQueue.push(*yuv);
	}
#endif
	if(yuv->m_format == VE_H264_PACKET){
		AVPacket avpkt;
		av_init_packet(&avpkt);

		avpkt.stream_index = 0;
		avpkt.duration = 0;
		avpkt.data = NULL;
		avpkt.size = 0;
		if(yuv->m_frameType == ANDROID_MEDIACODEC_KEY_FRAME){
			avpkt.flags = AV_PKT_FLAG_KEY;
		}else if(yuv->m_frameType == ANDROID_MEDIACODEC_PPS_SPS){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"yuv->frame_type == ANDROID_MEDIACODEC_PPS_SPS");
			return VE_ERR_OK;
		}

		av_grow_packet(&avpkt,yuv->m_len);
		avpkt.pts = avpkt.dts =  av_rescale_q(yuv->m_pts, (AVRational){1,1000}, m_config.m_vConfig.m_vTimebase);

		memcpy(avpkt.data,yuv->m_data,yuv->m_len);

		m_vMuxPktQueue.push(avpkt);
	}else if(yuv->m_format == VE_RGBA){

		if(m_filteredYuvSlotQueue.empty()){
			int i=0,size = m_config.m_vConfig.m_yuvQueueLen;
			ve_enc_yuv_buffer emptyYuv;
			for(int i=0;i<size;i++){
				m_filteredYuvSlotQueue.push(emptyYuv);
			}
		}
		ve_enc_yuv_buffer  filledYuv = m_filteredYuvSlotQueue.front();
		m_filteredYuvSlotQueue.pop();


		if(filledYuv.m_len < yuv->m_len){
			delete [] filledYuv.m_data;
			filledYuv.m_len = yuv->m_len;
			filledYuv.m_data = new uint8_t[filledYuv.m_len];
		}
		memcpy(filledYuv.m_data,yuv->m_data,yuv->m_len);

		filledYuv.m_width = yuv->m_width;
		filledYuv.m_height = yuv->m_height;
		filledYuv.m_format = yuv->m_format;
		filledYuv.m_frameType = yuv->m_frameType;
		filledYuv.m_pts = yuv->m_pts;
		m_filteredYuvQueue.push(filledYuv);
	}


	return VE_ERR_OK;
}
VE_ERR VEMp4Writer::writeAndEnc(ve_enc_pcm_buffer* pcm){

	int nbSamples;

	if(m_status){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_status=%d",m_status);
		return m_status;
	}
	if(m_stop){
		return VE_ERR_OK;;
	}
	if(!pcm->m_data || !pcm->m_len){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!pcm->data || !pcm->len");
		return VE_ERR_OK;
	}

	m_pcmBufferPos = 0;

	if(pcm->m_len + m_pcmBufferPos > PCM_BUFFER_SIZE){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"pcm->len + m_m_pcm_buffer_pos > PCM_BUFFER_SIZE");
		return VE_ERR_PARAM_ERR;
	}

	if(!m_pcmBuffer){
		m_pcmBuffer = (uint8_t*)av_malloc(PCM_BUFFER_SIZE);
	}

	memcpy(m_pcmBuffer + m_pcmBufferPos,pcm->m_data,pcm->m_len);

	m_pcmBufferPos = pcm->m_len;

	do{
		nbSamples = m_pcmBufferPos / m_config.m_aConfig.m_bytesPerSample;
		if(nbSamples < PCM_SAMPLES_PER_FRAME){
			return VE_ERR_OK;
		}
		aEncode(false);
        break;
	}while(1);

	return VE_ERR_OK;
}

VE_ERR VEMp4Writer::open(){

	int ret;
	int tryCount = 0;

	if(!m_outputFmtCtx){

		AVOutputFormat *fmt;

		if(!m_config.m_filename.size()){
			return VE_ERR_INPUT_PARAM;
		}
		avformat_alloc_output_context2(&m_outputFmtCtx, NULL, m_config.m_fmt.c_str(), m_config.m_filename.c_str());
		if(!m_outputFmtCtx){
			return VE_ERR_OPEN_FILE_FAILED;
		}

		fmt = m_outputFmtCtx->oformat;
#ifdef __ANDROID__
		m_vEncoder = &ff_libx264_encoder;
		//m_vEncoder = &ff_libmedia_codec_enc_encoder;
#endif
#ifdef __APPLE__
        m_vEncoder = &ff_h264_videotoolbox_encoder;
        //m_vEncoder = &ff_libx264_encoder;

#endif

		AVStream *videoStream = avformat_new_stream(m_outputFmtCtx, m_vEncoder);

		if(!videoStream){
			return VE_ERR_NEW_STREAM_FAILED;
		}
		videoStream->id = 0;
		videoStream->codec->codec_id = fmt->video_codec;

		if(!m_config.m_vConfig.m_vBitrate){
			m_config.m_vConfig.m_vBitrate = (int)(((m_config.m_vConfig.m_width * m_config.m_vConfig.m_height * 1.0)/BITRATE_720P_Y_SIZE) * BITRATE_720P);
		}


		videoStream->codec->bit_rate =  m_config.m_vConfig.m_vBitrate;
		videoStream->codec->rc_max_rate =  m_config.m_vConfig.m_vBitrate * 2;
		videoStream->codec->width = m_config.m_vConfig.m_width;
		videoStream->codec->height = m_config.m_vConfig.m_height;
		videoStream->time_base = m_config.m_vConfig.m_vTimebase;
		videoStream->codec->time_base = m_config.m_vConfig.m_vTimebase;
		videoStream->codec->gop_size = m_config.m_vConfig.m_gop;
		videoStream->codec->framerate = AVRational{m_config.m_vConfig.m_fps,1};

#ifdef __APPLE__
		videoStream->codec->pix_fmt = AV_PIX_FMT_NV12;
#else
		videoStream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
		videoStream->codec->ticks_per_frame = m_config.m_vConfig.m_vTimebase.den / m_config.m_vConfig.m_fps;
#endif


		videoStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		VE_LOG_TAG_INFO(VE_MODULE_TAG,"width=%d,height=%d,duration=%d",m_config.m_vConfig.m_width,m_config.m_vConfig.m_height,m_config.m_duration);

		m_vEncContext = videoStream->codec;

#ifdef __ANDROID__

		if(m_config.m_vConfig.m_type == VE_ENC_NONE){
			//for android hw encode
			//m_vEncContext->profile = FF_PROFILE_H264_BASELINE;
		}else{
			//for android soft encode

		}
		//av_opt_set_int(m_vEncContext, "crf", 23,AV_OPT_SEARCH_CHILDREN);
		av_opt_set(m_vEncContext, "profile", "baseline",AV_OPT_SEARCH_CHILDREN);
#endif
		while(1){

			ret = avcodec_open2(m_vEncContext, m_vEncoder, NULL);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_open2 ret=%d,m_config.m_vConfig.m_width=%d,m_config.m_vConfig.m_height=%d,m_vEncoder->video_codec_id=%d,m_vEncContext->codec_id=%d,m_vEncoder->type=%d,m_vEncContext->codec_type=%d",ret,m_config.m_vConfig.m_width, m_config.m_vConfig.m_height,m_vEncoder->id,m_vEncContext->codec_id,m_vEncoder->type,m_vEncContext->codec_type);
			/*
			if ((ret = avcodec_open2(m_vEncContext, m_vEncoder, NULL)) < 0) {
				tryCount++;
				VE_LOG_ERROR("VEMp4Writer::Open( avcodec_open2 failed,m_config.m_vConfig.m_width=%d,m_config.m_vConfig.m_height=%d,m_vEncoder->video_codec_id=%d,m_vEncContext->codec_id=%d,m_vEncoder->type=%d,m_vEncContext->codec_type=%d",m_config.m_vConfig.m_width, m_config.m_vConfig.m_height,m_vEncoder->id,m_vEncContext->codec_id,m_vEncoder->type,m_vEncContext->codec_type);
#ifdef __APPLE__
				if(tryCount < 3){
					av_usleep(1000000);
					continue;
				}
#endif
				return VE_ERR_OPEN_ENCODER_FAILED;
			}
			*/
			break;
		}

		//uint8_t* p_data = (uint8_t*)m_vEncContext->extradata;
		/*
		if(p_data){
			for(int i=0;i<m_vEncContext->extradata_size;i++){
				VE_LOG_INFO("VEMp4Writer::Open  m_vEncContext->extradata=%x",p_data[i]);
			}
		}

		VE_LOG_INFO("VEMp4Writer::Open m_vEncContext->extradata =%p,m_vEncContext->extradata_size=%d",m_vEncContext->extradata,m_vEncContext->extradata_size);
		*/

		int aBitrate = m_config.m_aConfig.m_aBitrate;

		if(!aBitrate){
			aBitrate = BITRATE_AAC;
		}
#ifdef __APPLE__
		//m_aEncoder = &ff_aac_at_encoder;
		m_aEncoder = &ff_libfdk_aac_encoder;
		/*
		if(m_aEncoder == &ff_libfdk_aac_encoder){
			aBitrate = m_config.m_aConfig.m_aBitrate;
		}else if(m_aEncoder == &ff_aac_at_encoder){
			aBitrate = m_config.m_aConfig.m_aBitrate * 2;
		}
		*/

#else
		m_aEncoder = &ff_libfdk_aac_encoder;

#endif

		AVStream *audioStream = avformat_new_stream(m_outputFmtCtx, m_aEncoder);
		if(!audioStream){
			return VE_ERR_NEW_STREAM_FAILED;
		}
        audioStream->id = 1;
		audioStream->codec->sample_fmt = m_config.m_aConfig.m_samplefmt;
		audioStream->codec->bit_rate    = aBitrate;
		audioStream->codec->sample_rate = m_config.m_aConfig.m_samplerate;

		audioStream->codec->channels        = m_config.m_aConfig.m_channels;
		audioStream->codec->channel_layout = m_config.m_aConfig.m_channelLayout;
		audioStream->time_base = m_config.m_aConfig.m_aTimebase;
		audioStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		VE_LOG_TAG_INFO(VE_MODULE_TAG,"channels=%d,sample_rate=%d",m_config.m_aConfig.m_channels,m_config.m_aConfig.m_samplerate);

		//ff_stream_add_bitstream_filter(audioStream,"aac_adtstoasc",NULL);
		m_aacBsfc = new VEBitstream("aac_adtstoasc");

		m_aEncContext = audioStream->codec;

#ifdef __APPLE__
		if(m_aEncoder == &ff_libfdk_aac_encoder){
			m_aEncContext->profile = FF_PROFILE_AAC_HE;
		}
#endif

		if ((ret = avcodec_open2(m_aEncContext, m_aEncoder, NULL)) < 0) {
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_open2 failed,video_codec_id=%d", m_aEncoder->id);
			return VE_ERR_OPEN_ENCODER_FAILED;
		}

		ret = avio_open(&m_outputFmtCtx->pb, m_config.m_filename.c_str(), AVIO_FLAG_WRITE);
		if(ret){
			return VE_ERR_OPEN_FILE_FAILED;
		}


	}

	return VE_ERR_OK;
}
void VEMp4Writer::close(){

	if(m_outputFmtCtx){
		av_write_trailer(m_outputFmtCtx);
		avio_closep(&m_outputFmtCtx->pb);

		if(m_vEncContext){
			avcodec_close(m_vEncContext);
			m_vEncContext = NULL;
		}
		if(m_aEncContext){
			avcodec_close(m_aEncContext);
			m_aEncContext = NULL;
		}
		avformat_free_context(m_outputFmtCtx);
		m_outputFmtCtx = NULL;
	}
    if(m_aacBsfc){
        delete m_aacBsfc;
        m_aacBsfc = NULL;
    }
    if(!m_completed){
    	unlink(m_config.m_filename.c_str());
    }else{
    	m_progress = 100;
    }
    if(m_aEncFrame){
		av_frame_free(&m_aEncFrame);
    	m_aEncFrame = NULL;
    }
    if(m_pcmBuffer){
    	av_free(m_pcmBuffer);
    	m_pcmBuffer = NULL;
    }

    if(m_vEncFrame){
    	av_free(m_vEncFrame);
    	m_vEncFrame = NULL;
    }

    while(!m_aMuxPktQueue.empty()){
    	AVPacket  pkt = m_aMuxPktQueue.front();
    	m_aMuxPktQueue.pop();
    	av_packet_unref(&pkt);
    }
    while(!m_vMuxPktQueue.empty()){
    	AVPacket  pkt = m_vMuxPktQueue.front();
    	m_vMuxPktQueue.pop();
    	av_packet_unref(&pkt);
    }

    int retainCount;

    while(!m_filteredYuvQueue.empty()){
    	ve_enc_yuv_buffer  yuv = m_filteredYuvQueue.front();
    	m_filteredYuvQueue.pop();
#ifdef __APPLE__
		    CVPixelBufferRef destPixel = (CVPixelBufferRef)yuv.m_data;

		    while(1){
				retainCount = (int)CFGetRetainCount(destPixel);
				if(retainCount > 0){
					CVPixelBufferRelease(destPixel);
				}
				if(retainCount == 1)break;
		    }
#else
		    if(m_config.m_vConfig.m_type == VE_ENC_NONE){
		    	delete [] yuv.m_data;
		    }
#endif
    }

    while(!m_filteredYuvSlotQueue.empty()){
    	ve_enc_yuv_buffer  yuv = m_filteredYuvSlotQueue.front();
    	m_filteredYuvSlotQueue.pop();
		delete [] yuv.m_data;
    }

	return;
}

VE_ERR VEMp4Writer::muxing(){

	int printTime;
	float progress;
	float progress2;

	VEQueue<AVPacket> *pktQueue = NULL;
	int ret;
	AVPacket pkt;

	if(m_lastVMuxPktPtsInMs - m_lastAMuxPktPtsInMs > 200){
		pktQueue = &m_aMuxPktQueue;
		if(pktQueue->empty()){
			pktQueue = &m_vMuxPktQueue;
		}
	}else{
		pktQueue = &m_vMuxPktQueue;

		if(pktQueue->empty() && m_vCompleted){
			pktQueue = &m_aMuxPktQueue;
		}
	}

	if(pktQueue->empty()){
		av_usleep(AV_USLEEP_GAP);
		return VE_ERR_OK;
	}
    pkt = pktQueue->front();
    pktQueue->pop();

    if(pkt.pts < 0){
		pkt.pts = pkt.dts = 0;
	}
	if(pkt.stream_index == 0){

		if(m_firstVMuxPkt){
			m_firstVMuxPkt = false;
			pkt.pts = pkt.dts = 0;
            pkt.duration = 0;
        }
		 if(pkt.dts <= m_lastVMuxPktDts){
			 pkt.dts = m_lastVMuxPktPts + 1;
		 }
		 if(pkt.pts < pkt.dts){
			 pkt.pts = pkt.dts;
		 }




		m_lastVMuxPktPts = pkt.pts;
		m_lastVMuxPktDts = pkt.dts;
		m_lastVMuxPktPtsInMs = av_rescale_q(m_lastVMuxPktPts,m_config.m_vConfig.m_vTimebase,(AVRational){ 1, 1000 });
		m_lastVMuxPktDtsInMs = av_rescale_q(m_lastVMuxPktDts,m_config.m_vConfig.m_vTimebase,(AVRational){ 1, 1000 });

		m_muxVPktCount++;

        if(!m_firstVMuxPkt){
            pkt.duration = pkt.pts - m_lastVMuxPktPts;
        }
        if(m_flushEncoderDone && m_vMuxPktQueue.empty()){
        	pkt.duration = av_rescale_q(m_config.m_duration - m_lastVMuxPktPtsInMs,(AVRational){ 1, 1000 },m_config.m_vConfig.m_vTimebase);
        }

	}else{

		int64_t curPtsInMs = av_rescale_q(pkt.pts,(AVRational){ 1, m_config.m_aConfig.m_samplerate },(AVRational){ 1, 1000 });

    	int64_t aFrameDuration = av_rescale_q(pkt.duration,(AVRational){ 1, m_config.m_aConfig.m_samplerate },(AVRational){ 1, 1000 });
    	if( curPtsInMs + aFrameDuration > m_config.m_duration){
			av_packet_unref(&pkt);
			return VE_ERR_OK;
		}

		m_muxAPktCount++;
		m_lastAMuxPktPts = pkt.pts;
		m_lastAMuxPktPtsInMs = av_rescale_q(m_lastAMuxPktPts,(AVRational){ 1, m_config.m_aConfig.m_samplerate },(AVRational){ 1, 1000 });


		m_aacBsfc->applyBitstreamFilter(m_aEncContext,&pkt);

		//VE_LOG_INFO("VEMp4Writer::muxing a_pkt_count=%d,last_a_mux_pkt_pts_in_ms=%lld",m_muxAPktCount,m_lastAMuxPktPtsInMs);
	}


	VE_LOG_TAG_INFO(VE_MODULE_TAG,"(%lld) last_v_mux_pkt_pts_in_ms=%lld,last_a_mux_pkt_pts_in_ms=%lld,v_mux_pkt_count=%d,a_mux_pkt_count=%d",(av_gettime() - m_config.m_startTime) / 1000,m_lastVMuxPktPtsInMs,m_lastAMuxPktPtsInMs,m_muxVPktCount,m_muxAPktCount);

	if(!m_writeHeader){
		ret = avformat_write_header(m_outputFmtCtx, NULL);
		m_writeHeader = 1;
		if(ret){
			return VE_ERR_WRITE_FILE_FAILED;
		}
	}

    ret = av_interleaved_write_frame(m_outputFmtCtx, &pkt);

    av_packet_unref(&pkt);

    if(ret){
    	char err[256] = {0};
        av_strerror(ret, err, 256);
        VE_LOG_TAG_ERROR(VE_MODULE_TAG,"av_interleaved_write_frame ret=%d,info=%s",ret,err);
    }


	progress = m_lastAMuxPktPtsInMs * 100.0 / m_config.m_duration;
	progress2 = m_lastVMuxPktPtsInMs * 100.0 / m_config.m_duration;

	if(progress2 > progress && progress2 <= 100){
		progress = progress2;
	}
    if(progress < 100){
        m_progress = progress;
    }


    if(!m_firstMuxTs){
    	printTime = 0;
    }else{
    	printTime = (av_gettime() - m_firstMuxTs) / 1000;
    }

    if(!m_firstMuxTs){
    	m_firstMuxTs = av_gettime();
    }

    VE_LOG_TAG_INFO(VE_MODULE_TAG,"progress=%f,cost_time=%d",m_progress,printTime);

    return VE_ERR_OK;
}

VE_ERR VEMp4Writer::vEncoding(){

	if(m_flushEncoder){
		return vEncode(true);
	}
	if(m_filteredYuvQueue.empty()){
		av_usleep(AV_USLEEP_GAP);
		return VE_ERR_OK;
	}

	return vEncode(false);
}
VE_ERR VEMp4Writer::aEncode(bool flush){

	int ret;
	char err[256] = {0};
	int gotOutput = 0;

	if(!m_aEncContext){
		return VE_ERR_OK;
	}
	if(!m_aEncFrame){
		m_aEncFrame = av_frame_alloc();

		if(!m_aEncFrame){
			return VE_ERR_MALLOC_FAILED;
		}
	}

	if(!flush){

		if(m_lastAPts == -1){
			m_lastAPts = 0;
		}else{
			m_lastAPts+= 1000000 * (uint64_t)PCM_SAMPLES_PER_FRAME / m_config.m_aConfig.m_samplerate;
		}
		m_aEncFrame->pts = m_aEncFrame->pkt_pts = m_aEncFrame->pkt_dts = av_rescale_q(m_lastAPts,(AVRational){ 1, 1000000 },(AVRational){ 1, m_config.m_aConfig.m_samplerate });

		m_aEncFrame->nb_samples = PCM_SAMPLES_PER_FRAME;


		ret = avcodec_fill_audio_frame(m_aEncFrame, m_config.m_aConfig.m_channels, m_config.m_aConfig.m_samplefmt,
									   (const uint8_t*)m_pcmBuffer,PCM_SAMPLES_PER_FRAME * m_config.m_aConfig.m_bytesPerSample, 0);


		if(ret < 0){
			av_strerror(ret, err, 256);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"m_config.m_aConfig.m_channels=%d,m_config.m_aConfig.m_samplefmt=%d,left_samples=%d,info:%s",m_config.m_aConfig.m_channels,m_config.m_aConfig.m_samplefmt);

			return VE_ERR_FILL_AUDIO_FAILED;
		}
	}

	do{
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = NULL; // packet data will be allocated by the encoder
		pkt.size = 0;
		gotOutput = 0;
		if(!flush){
			ret = avcodec_encode_audio2(m_aEncContext, &pkt, m_aEncFrame, &gotOutput);

		}else{
			ret = avcodec_encode_audio2(m_aEncContext, &pkt, NULL, &gotOutput);
		}
/*
#ifdef __APPLE__
                    if(ret < 0){
                        avcodec_close(m_aEncContext);
                        av_usleep(1000000);
                        if ((ret = avcodec_open2(m_aEncContext, m_aEncoder, NULL)) < 0) {
                            VE_LOG_ERROR("VEMp4Writer::a_encode( avcodec_open2 failed,m_aEncoder->audio_codec_id=%d,m_aEncContext->codec_id=%d,m_aEncoder->type=%d,m_aEncContext->codec_type=%d", m_aEncoder->id,m_aEncContext->codec_id,m_aEncoder->type,m_aEncContext->codec_type);
                            return VE_ERR_OPEN_ENCODER_FAILED;
                        }
                        continue;
                    }
#endif
*/
		pkt.stream_index = 1;

		if(ret){
			return VE_ERR_ENCODE_FAILED;
		}

		if(gotOutput){

             if(m_lastAPts2 <  0){
            	 m_lastAPts2 = 0;
             }
             pkt.pts = pkt.dts = m_lastAPts2;
             m_lastAPts2 += pkt.duration;

             VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_lastAPts=%lld",m_lastAPts);
			 m_aMuxPktQueue.push_back(pkt);
		}

		if(flush && gotOutput){
			continue;
		}
		break;
	}while(1);

	return VE_ERR_OK;
}
VE_ERR VEMp4Writer::vEncode(bool flush){

	ve_enc_yuv_buffer yuv;
	int ret;
	AVPacket pkt;
	int gotOutput = 0;

	if(m_flushEncoderDone){
		return VE_ERR_OK;
	}
	if(!flush){
		yuv = m_filteredYuvQueue.front();
		m_filteredYuvQueue.pop();

		if(!m_vEncFrame){
			m_vEncFrame =  av_frame_alloc();
#ifdef __ANDROID__
			        m_vEncFrame->width = m_config.m_vConfig.m_width;
			        m_vEncFrame->height = m_config.m_vConfig.m_height;
			        m_vEncFrame->format = AV_PIX_FMT_YUV420P;
			        av_frame_get_buffer(m_vEncFrame, 16);
#endif
		}
		if(!m_vEncFrame){
			return VE_ERR_MALLOC_FAILED;
		}

#ifdef __APPLE__
		m_vEncFrame->data[3] = yuv.m_data;
		m_vEncFrame->format = AV_PIX_FMT_VIDEOTOOLBOX;

		m_vEncContext->pix_fmt = AV_PIX_FMT_VIDEOTOOLBOX;
#else
		libyuv::ABGRToI420((uint8_t*)yuv.m_data,m_config.m_vConfig.m_width * 4,m_vEncFrame->data[0],m_vEncFrame->linesize[0],m_vEncFrame->data[1],m_vEncFrame->linesize[1],m_vEncFrame->data[2],m_vEncFrame->linesize[2],m_config.m_vConfig.m_width,m_config.m_vConfig.m_height);

#endif

		m_vEncFrame->pts = yuv.m_pts * 1000;
		m_vEncFrame->width = m_config.m_vConfig.m_width;
		m_vEncFrame->height = m_config.m_vConfig.m_height;

	}

	if(flush){
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_flushEncoder");
	}

	do{

		av_init_packet(&pkt);
		pkt.data = NULL;    // packet data will be allocated by the encoder
		pkt.size = 0;
		gotOutput = 0;
		if(!flush){
			do{
				ret = avcodec_encode_video2(m_vEncContext, &pkt, m_vEncFrame, &gotOutput);
	#ifdef __APPLE__
				if(ret < 0){

					av_usleep(800000);
					avcodec_close(m_vEncContext);
					av_usleep(200000);
					m_vEncContext->pix_fmt = AV_PIX_FMT_YUV420P;
					if ((ret = avcodec_open2(m_vEncContext, m_vEncoder, NULL)) < 0) {
						VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_open2 failed,m_vEncoder->video_codec_id=%d,m_vEncContext->codec_id=%d,m_vEncoder->type=%d,m_vEncContext->codec_type=%d", m_vEncoder->id,m_vEncContext->codec_id,m_vEncoder->type,m_vEncContext->codec_type);
						return VE_ERR_OPEN_ENCODER_FAILED;
					}
					continue;

				}
	#endif
				break;
			}while(0);

		}else{
			//VE_LOG_INFO("v_encode flush 1");
			ret = avcodec_encode_video2(m_vEncContext, &pkt, (AVFrame*)NULL, &gotOutput);
			//VE_LOG_INFO("v_encode flush 2");
		}

		if(gotOutput){

            pkt.duration = 0;
            pkt.stream_index = 0;
            av_packet_rescale_ts(&pkt,m_vEncContext->time_base,m_config.m_vConfig.m_vTimebase);
			m_vMuxPktQueue.push(pkt);
		}
#ifdef __ANDROID__
		if(!flush){
			if(m_filteredYuvQueue.size() >= m_config.m_vConfig.m_yuvQueueLen){
				delete [] yuv.m_data;
			}else{
				m_filteredYuvSlotQueue.push(yuv);
			}

		}
#endif
		if(flush && gotOutput){
			continue;
		}
		break;
	}while(1);

	if(flush){
		m_flushEncoderDone = 1;
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_flushEncoderDone");
	}
	return VE_ERR_OK;
}


void* veMuxingProcess(void* context){

	VE_ERR ret;

	VEMp4Writer * mp4writer = (VEMp4Writer*)context;

	while(!mp4writer->m_exit){

		ret = mp4writer->muxing();

		if(ret){
			mp4writer->m_status = ret;
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"failed!");
			break;
		}
	}
    return 0;

}

void* veVEncodingProcess(void* context){

	VE_ERR ret;

	VEMp4Writer * mp4writer = (VEMp4Writer*)context;

	mp4writer->m_flushEncoder = 0;

	while(!mp4writer->m_exit){

		ret = mp4writer->vEncoding();

		if(ret){
			mp4writer->m_status = ret;
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"v_encoding failed!");
			break;
		}
	}
    return 0;
}
