#include "VEWebm.h"
#include "VELog.h"
#include "VEConfig.h"
#include "libyuv.h"
#include "VESource.h"


#define VE_MODULE_TAG "[VEWebm]"

void * veWebmProcess(void* context);

VEWebm::VEWebm(ve_webm_config* config){

	m_outputFilename = config->output_filename;
	m_config = *config;
	m_config.output_filename = m_outputFilename.c_str();

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_config.frame_format=%d",m_config.frame_format);
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_config.output_filename=%s",m_config.output_filename?m_config.output_filename:"");

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_config.output_fps=%d",m_config.output_fps);
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_config.output_width=%d",m_config.output_width);
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_config.output_height=%d",m_config.output_height);
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_config.output_video_bitrate=%d",m_config.output_video_bitrate);

	VEConfig::initialize();
#ifdef __ANDROID__
	av_log_set_callback(veLogCallbackFfmpeg);
#endif
}
VEWebm::~VEWebm(){
}
VE_WEBM_ERR VEWebm::start(){
	comn::AutoCritSec lock(m_cs);

	VE_WEBM_ERR ret = openOutputFile();

	if(ret){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"openOutputFile failed!");
		return ret;
	}
	if(!m_exit)return VE_WEBM_ERR_OK;
	m_exit = 0;

    pthread_create(&m_worker, NULL, veWebmProcess, (void *)this);
    /*
	if(0 != pthread_create(&m_worker, NULL, veWebmProcess, (void *)this)){
        VE_LOG_ERROR("VEWebm::Start pthread_create veWebmProcess failed!");
        return VE_WEBM_ERR_CREATE_THREAD_FAILED;
	}
     */

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"done");

	return VE_WEBM_ERR_OK;
}
VE_WEBM_ERR VEWebm::stop(){
	comn::AutoCritSec lock(m_cs);

	m_exit = 1;

	if(m_worker){
		pthread_join(m_worker, NULL);
		m_worker = 0;
	}

	VE_WEBM_ERR ret = closeOutputFile();

	VE_LOG_TAG_INFO(VE_MODULE_TAG,"enter");

	return ret;
}
VE_WEBM_ERR VEWebm::sendFrame(ve_webm_frame* frame){
	comn::AutoCritSec lock(m_cs);

	int ret;
	int i,j;
	uint8_t* alpha,*alphaSrc;
	AVPacket pkt;
	enum AVPixelFormat format;
	int64_t startTime = av_gettime(),endTime;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    /*
    if(m_config.frame_format == VE_WEBM_COLOR_YUVA420P){
    	if(!frame  || frame->width <= 0 || frame->height <= 0 || !frame->data[0] || !frame->data[1] || !frame->data[2] ||
    			frame->linesize[0] <= 0 || frame->linesize[1] <= 0 || frame->linesize[2] <= 0){
            VE_LOG_ERROR("VEWebm::SendFrame !frame || frame->width <= 0 || frame->height <= 0 || !frame->data[0] || !frame->data[1] || !frame->data[2] || frame->linesize[0] <= 0 || frame->linesize[1] <= 0 || frame->linesize[2] <= 0");
            return VE_WEBM_ERR_INPUT_PARAM;
    	}
    	format = AV_PIX_FMT_YUVA420P;
    }
     */
    if(m_config.frame_format == VE_WEBM_COLOR_ARGB){
		if(!frame  || frame->width <= 0 || frame->height <= 0 || !frame->data[0] ||
				frame->linesize[0] <= 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"VE_WEBM_COLOR_ARGB !frame  || frame->width <= 0 || frame->height <= 0 || !frame->data[0] || frame->linesize[0] <= 0");
			return VE_WEBM_ERR_INPUT_PARAM;
		}
		format = AV_PIX_FMT_RGBA;
    }
    if(m_config.frame_format == VE_WEBM_COLOR_RGBA){
        if(!frame  || frame->width <= 0 || frame->height <= 0 || !frame->data[0] ||
           frame->linesize[0] <= 0){
            VE_LOG_TAG_ERROR(VE_MODULE_TAG,"VE_WEBM_COLOR_RGBA !frame  || frame->width <= 0 || frame->height <= 0 || !frame->data[0] || frame->linesize[0] <= 0");
            return VE_WEBM_ERR_INPUT_PARAM;
        }
        format = AV_PIX_FMT_RGBA;
    }
    
    

	if(m_exit){
		return VE_WEBM_ERR_OK;
	}

	if(!m_vEncFrame){
		m_vEncFrame =  av_frame_alloc();
        /*
        if(!m_vEncFrame){
            VE_LOG_ERROR("VEWebm::SendFrame av_frame_alloc failed!");
            return VE_WEBM_ERR_MALLOC_FAILED;
        }
         */
        m_vEncFrame->width = m_config.output_width;
        m_vEncFrame->height = m_config.output_height;
        m_vEncFrame->format = format = AV_PIX_FMT_YUVA420P;
        av_frame_get_buffer(m_vEncFrame, 16);
	}
    


    if(!m_vFrame){
        m_vFrame = av_frame_alloc();
        /*
        if(!m_vFrame){
            VE_LOG_ERROR("VEWebm::SendFrame av_frame_alloc 2 failed!");
            return VE_WEBM_ERR_MALLOC_FAILED;
        }
         */

        m_vFrame->width = frame->width;
        m_vFrame->height = frame->height;
        m_vFrame->format = format = AV_PIX_FMT_YUVA420P;
        av_frame_get_buffer(m_vFrame, 16);
    }


    alphaSrc = (uint8_t*)frame->data[0];
    alpha = (uint8_t*)m_vFrame->data[3];
    
    if(m_config.frame_format == VE_WEBM_COLOR_RGBA){
            ret = libyuv::ABGRToI420(frame->data[0],frame->linesize[0],m_vFrame->data[0],m_vFrame->linesize[0],m_vFrame->data[1],m_vFrame->linesize[1],m_vFrame->data[2],m_vFrame->linesize[2],frame->width,frame->height);
    }else if(m_config.frame_format == VE_WEBM_COLOR_ARGB){
                ret = libyuv::RGBAToI420(frame->data[0],frame->linesize[0],m_vFrame->data[0],m_vFrame->linesize[0],m_vFrame->data[1],m_vFrame->linesize[1],m_vFrame->data[2],m_vFrame->linesize[2],frame->width,frame->height);
    }




    for(i=0;i<frame->height;i++){

    	for(j=0;j<frame->width;j++){

    		alpha[i * m_vFrame->linesize[3] + j] = alphaSrc[i * frame->linesize[0]+ j * 4 + 3];
    	}
    }

    if(!m_swsCtx || m_vFrame->width != m_swsCtxWidth || m_vFrame->height != m_swsCtxHeight){

        sws_freeContext(m_swsCtx);

        m_swsCtx = NULL;
        m_swsCtxWidth = m_vFrame->width;
        m_swsCtxHeight = m_vFrame->height;
        m_swsCtxFormat = format;

        m_swsCtx = sws_getContext(m_vFrame->width, m_vFrame->height,
        		format,m_config.output_width, m_config.output_height, AV_PIX_FMT_YUVA420P, SWS_BILINEAR, 0, 0, 0);
    }

    //if(m_vFrame->width != m_config.output_width || m_vFrame->height != m_config.output_height){
    	sws_scale(m_swsCtx, m_vFrame->data, m_vFrame->linesize, 0, m_vFrame->height,m_vEncFrame->data, m_vEncFrame->linesize);
    //}


    m_vEncFrame->pts = frame->pts;

    int gotOutput;

    ret = avcodec_encode_video2(m_vEncContext, &pkt, m_vEncFrame, &gotOutput);

	 if(gotOutput && pkt.data){

        pkt.duration = 0;

        pkt.stream_index = 0;

		av_packet_rescale_ts(&pkt,
							 m_vEncContext->time_base,
							 m_vTimebase);

		 m_vMuxPktQueue.push_back(pkt);

		 endTime = av_gettime();

		 VE_LOG_TAG_INFO(VE_MODULE_TAG,"cost=%lld",(endTime - startTime));

	 }

	return VE_WEBM_ERR_OK;
}
VE_WEBM_ERR VEWebm::openOutputFile(){

	int ret;


	if(!m_outputFmtCtx){

		AVOutputFormat *fmt;

		if(!m_outputFilename.size()){
			return VE_WEBM_ERR_INPUT_PARAM;
		}
		avformat_alloc_output_context2(&m_outputFmtCtx, NULL, "webm", m_outputFilename.c_str());
		if(!m_outputFmtCtx){
			return VE_WEBM_ERR_OPEN_FILE_FAILED;
		}

		fmt = m_outputFmtCtx->oformat;

		m_vEncoder = &ff_libvpx_vp8_encoder;

		AVStream *video_stream = avformat_new_stream(m_outputFmtCtx, m_vEncoder);

        /*
		if(!video_stream){
			return VE_WEBM_ERR_NEW_STREAM_FAILED;
		}
         */
		video_stream->id = 0;
		video_stream->codec->codec_id = fmt->video_codec;
		m_vBitrate = (int)(((m_config.output_width * m_config.output_height * 1.0)/BITRATE_720P_Y_SIZE) * BITRATE_720P);
		video_stream->codec->bit_rate =  m_vBitrate;
		video_stream->codec->width = m_config.output_width;
		video_stream->codec->height = m_config.output_height;
		video_stream->time_base = m_vTimebase;
		video_stream->codec->time_base = m_vTimebase;
		video_stream->codec->gop_size = m_gop;

		video_stream->codec->pix_fmt = AV_PIX_FMT_YUVA420P;

        video_stream->avg_frame_rate.num = m_config.output_fps;
		video_stream->avg_frame_rate.den = 1;

		video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		VE_LOG_TAG_INFO(VE_MODULE_TAG,"width=%d,height=%d",m_config.output_width,m_config.output_height);

		m_vEncContext = video_stream->codec;

		m_vEncContext->profile = 0;

		av_opt_set_int(m_vEncContext, "auto-alt-ref", 0,AV_OPT_SEARCH_CHILDREN);//ios会初始化失败
		av_opt_set_int(m_vEncContext, "lag-in-frames", 0,AV_OPT_SEARCH_CHILDREN);//android会初始化失败

		if ((ret = avcodec_open2(m_vEncContext, m_vEncoder, NULL)) < 0) {

			//VE_LOG_ERROR("VEWebm::openOutputFile( avcodec_open2 failed,m_config.output_width=%d,m_config.output_height=%d,m_vEncoder->video_codec_id=%d,m_vEncContext->codec_id=%d,m_vEncoder->type=%d,m_vEncContext->codec_type=%d",m_config.output_width, m_config.output_height,m_vEncoder->id,m_vEncContext->codec_id,m_vEncoder->type,m_vEncContext->codec_type);

			return VE_WEBM_ERR_OPEN_ENCODER_FAILED;
		}

		ret = avio_open(&m_outputFmtCtx->pb, m_outputFilename.c_str(), AVIO_FLAG_WRITE);
		if(ret){
			return VE_WEBM_ERR_OPEN_FILE_FAILED;
		}

		ret = avformat_write_header(m_outputFmtCtx, NULL);
		if(ret){
			return VE_WEBM_ERR_WRITE_FILE_FAILED;
		}
	}

	return VE_WEBM_ERR_OK;
}
VE_WEBM_ERR VEWebm::closeOutputFile(){

    av_write_trailer(m_outputFmtCtx);
    avio_closep(&m_outputFmtCtx->pb);
    if(m_vEncContext){
        avcodec_close(m_vEncContext);
        m_vEncContext = NULL;
    }

    avformat_free_context(m_outputFmtCtx);
    m_outputFmtCtx = NULL;

    do{

        while(!m_vMuxPktQueue.empty()){
            AVPacket & pkt = m_vMuxPktQueue.front();
            av_packet_unref(&pkt);
            m_vMuxPktQueue.pop_front();

        }
    }while(0);

	if(m_vEncFrame){
		av_frame_free(&m_vEncFrame);
		m_vEncFrame = NULL;
	}

	if(m_vFrame){
		av_frame_free(&m_vFrame);
		m_vFrame = NULL;
	}

	if(m_swsCtx){
		sws_freeContext(m_swsCtx);
		m_swsCtx = NULL;
	}



	return VE_WEBM_ERR_OK;
}
VE_WEBM_ERR VEWebm::muxing(){

	char err[256] = {0};
	AVPacket avPkt;

	AVPacket pkt;
	int gotOutput;
	int64_t startTime = av_gettime(),endTime;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

	if(m_exit && !m_flushEncoder && m_vEncFrame){

        /*
		do{
			gotOutput = 0;

			int ret = avcodec_encode_video2(m_vEncContext, &pkt, NULL, &gotOutput);
            
			 if(gotOutput && pkt.data){

				pkt.duration = 0;

				pkt.stream_index = 0;

				av_packet_rescale_ts(&pkt,
									 m_vEncContext->time_base,
									 m_vTimebase);

				 m_vMuxPktQueue.push_back(pkt);

				 endTime = av_gettime();

				 VE_LOG_INFO("VEWebm::muxing flush encoder cost=%lld",(endTime - startTime));

			 }else{
				 m_flushEncoder = true;
			 }
		}while(gotOutput);
         */
        m_flushEncoder = true;
    }

	if(m_vMuxPktQueue.empty()){
		/*
		if(m_exit && !m_flushEncoder && m_vEncFrame){

		}
		*/
		av_usleep(20000);
		return VE_WEBM_ERR_OK;
	}

	avPkt = m_vMuxPktQueue.front();
	m_vMuxPktQueue.pop();

    if(m_firstVMuxPkt){
    	m_firstVMuxPkt = false;
    	avPkt.pts = avPkt.dts = 0;
    }

    if(avPkt.dts <= m_lastVMuxPktDts){
        avPkt.dts = m_lastVMuxPktPts + 1;
    }
    if(avPkt.pts < avPkt.dts){
   	 avPkt.pts = avPkt.dts;
    }

    m_lastVMuxPktPts = avPkt.pts;
    m_lastVMuxPktDts = avPkt.dts;

    avPkt.duration = 0;

    avPkt.stream_index = 0;

    av_packet_rescale_ts(&avPkt,
						 m_vEncContext->time_base,
						 m_vTimebase);


    int ret = av_interleaved_write_frame(m_outputFmtCtx, &avPkt);

    
    VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_count: %d,m_lastVMuxPktPts=%lld",m_count,m_lastVMuxPktPts);
    m_count++;
    av_packet_unref(&avPkt);

    if(ret){
		  av_strerror(ret, err, 256);

		  //VE_LOG_ERROR("VEWebm::muxing av_interleaved_write_frame failed! err:%s",err);

		  return VE_WEBM_ERR_WRITE_FILE_FAILED;
    }

	return VE_WEBM_ERR_OK;
}
void * veWebmProcess(void* context){

	VEWebm* webm = (VEWebm*)context;

	VE_WEBM_ERR ret;

	do{


		if(webm->m_exit && webm->m_flushEncoder){
			break;
		}
		ret = webm->muxing();

		if(ret)break;

	}while(1);

    return 0;
}

