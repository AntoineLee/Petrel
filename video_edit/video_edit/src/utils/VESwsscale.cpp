#include "VESwsscale.h"

#ifdef __APPLE__

#define DEFAULT_SIZE 4096
#else

#define DEFAULT_SIZE 4096
#endif

int GetPixelBufferNV12Align(int width,int height);
VESwsscale::~VESwsscale(){
	if(m_vFrameI420){
		av_frame_free(&m_vFrameI420);
		m_vFrameI420 = NULL;
	}
	if(m_swsCtx){
		sws_freeContext(m_swsCtx);
		m_swsCtx = NULL;
	}
}
VE_ERR VESwsscale::process(AVFrame* src,AVFrame** dest,int destFmt,int destWidth,int destHeight){

	VE_ERR ret = VE_ERR_OK;
	int scaleH = destHeight;
	int scaleW = destWidth;
	int yLength;


	m_vFrame = src;

	if(m_swsCtxWidth != scaleW || m_swsCtxHeight != scaleH || m_swsCtxFormat != destFmt){
		sws_freeContext(m_swsCtx);
		m_swsCtx = NULL;
		m_swsCtxWidth = scaleW;
		m_swsCtxHeight = scaleH;
		m_swsCtxFormat = destFmt;

	}
	if(!m_swsCtx){
		m_swsCtx = sws_getContext(m_vFrame->width, m_vFrame->height,
				(enum AVPixelFormat)m_vFrame->format,
				scaleW, scaleH, (enum AVPixelFormat)destFmt, SWS_BILINEAR, 0, 0, 0);
		/*
		if(!m_swsCtx){
			VE_LOG_ERROR("VESwsscale::process sws_getContext failed!");
			ret = VE_ERR_MALLOC_FAILED;
			goto err_exit;
		}
		*/
	}
	yLength = m_swsCtxWidth  * m_swsCtxHeight;

	if(!m_vFrameI420 || m_vFrameI420->width != scaleW || m_vFrameI420->height != scaleH){
		av_frame_free(&m_vFrameI420);
		m_vFrameI420 = NULL;

		m_vFrameI420 = av_frame_alloc();
		/*
		if(!m_vFrameI420){
			VE_LOG_ERROR("VESource::process m_vFrameI420 av_frame_alloc failed!");
			ret = VE_ERR_MALLOC_FAILED;
			goto err_exit;
		}
		*/


		m_vFrameI420->width = scaleW;
		m_vFrameI420->height = scaleH;
		m_vFrameI420->format = destFmt;

		int linesize_align = GetPixelBufferNV12Align(scaleW,scaleH);
		av_frame_get_buffer(m_vFrameI420, 16);

	}
	int64_t startTime,endTime;
	startTime = av_gettime();
	sws_scale(m_swsCtx, m_vFrame->data, m_vFrame->linesize, 0, m_vFrame->height,m_vFrameI420->data, m_vFrameI420->linesize);
	endTime = av_gettime();
	//VE_LOG_INFO("VESource::process sws_scale_cost=%lld",endTime - startTime);
	*dest = m_vFrameI420;

	return VE_ERR_OK;

	err_exit:

	return ret;
}
void VESwsscale::getOpenGLWH(int width,int height,int* destWidth,int* destHeight){

		int scaleMax;
		int scaleH = height;
		int scaleW = width;

	    if(scaleH > scaleW){
	    	scaleMax = scaleH;
	    }else{
	    	scaleMax = scaleW;
	    }
	    if(scaleMax > DEFAULT_SIZE){
	    	if(scaleH > scaleW){

	    		scaleW =  scaleW * DEFAULT_SIZE / scaleH;
	            scaleH = DEFAULT_SIZE;
	    	}else{

	    		scaleH = scaleH * DEFAULT_SIZE / scaleW;
	            scaleW = DEFAULT_SIZE;
	        }
	    }
	    while(scaleW % 8){
	    	scaleW++;
	    }
	    if(scaleH % 2){
	    	scaleH++;
	    }
	    *destWidth = scaleW;
	    *destHeight = scaleH;
}
