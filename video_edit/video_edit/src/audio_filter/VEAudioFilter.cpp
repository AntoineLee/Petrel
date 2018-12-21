#include "VECommon.h"
#include "VEAudioFilter.h"
#include "ve_interface.h"
#include "VEConfig.h"

static bool veAfRegistered = false;
extern "C" AVFilter ff_ve_af_afade;

#define VE_MODULE_TAG "[VEAudioFilter]"

#if 1
VEAudioFilter::VEAudioFilter(){

}
VEAudioFilter::~VEAudioFilter(){
	releaseMem();
}

void VEAudioFilter::initialize(){
	if(!veAfRegistered){
		veAfRegistered = true;
		avfilter_register(&ff_ve_af_afade);
	}
}
bool VEAudioFilter::isInit(){
	return m_inited;
}
int VEAudioFilter::init(AVCodecContext* ctx){

	if(isInit()){
		return VE_ERR_OK;
	}
	if(!m_filters){
		return VE_ERR_INPUT_PARAM;
	}
	return this->addFilters(ctx,m_filters);
}

int VEAudioFilter::addFilters(const char* filtersExpr){

	int size = strlen(filtersExpr) + 1;
	m_filters = new char[size];

    memcpy(m_filters,filtersExpr,size);

    VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_filters=%s",m_filters);

	return VE_ERR_OK;
}
int VEAudioFilter::addFilters(AVCodecContext* ctx,const char* filtersExpr){

	char args[512];

    int outSampleFmts[] = { m_samplefmt, -1 };
    int64_t outChannelLayouts[] = { m_channelLayout, -1 };
    int outSampleRates[] = { m_samplerate, -1 };
    
	int ret = VE_ERR_OK;

	VEConfig::initialize();
	VEAudioFilter::initialize();


	int size = strlen(filtersExpr) + 1;


	if(!m_filters){
		m_filters = new char[size];
	    memcpy(m_filters,filtersExpr,size);
	}
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"m_filters=%s",m_filters);


	if(m_filterGraph){
		return ret;
	}

	m_aBufferSrc  = avfilter_get_by_name("abuffer");
    m_aBufferSink = avfilter_get_by_name("abuffersink");
    m_outputs = avfilter_inout_alloc();
    m_inputs  = avfilter_inout_alloc();

    m_filterGraph = avfilter_graph_alloc();

    m_frame = av_frame_alloc();
    m_filtFrame = av_frame_alloc();

    /*
    if (!m_outputs || !m_inputs || !m_filterGraph || !m_frame || !m_filtFrame) {
        ret = VE_ERR_MALLOC_FAILED;
        goto AddFilters_end;
    }
    */

	if(!ctx->channel_layout){
		if(ctx->channels > 1 ){
			ctx->channel_layout = AV_CH_LAYOUT_STEREO;
		}else{
			ctx->channel_layout = AV_CH_LAYOUT_MONO;
		}
	}


    snprintf(args, sizeof(args),
                "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llx",
                ctx->time_base.num, ctx->time_base.den, ctx->sample_rate,
                av_get_sample_fmt_name(ctx->sample_fmt), ctx->channel_layout);


    if((ret = avfilter_graph_create_filter(&m_bufferSrcCtx, m_aBufferSrc, "in",
                                       args, NULL, m_filterGraph)) < 0
    		|| (ret = avfilter_graph_create_filter(&m_bufferSinkCtx, m_aBufferSink, "out",
                    NULL, NULL, m_filterGraph)) < 0
					|| (ret = av_opt_set_int_list(m_bufferSinkCtx, "sample_fmts", outSampleFmts, -1,
                            AV_OPT_SEARCH_CHILDREN)) < 0
							|| (ret = av_opt_set_int_list(m_bufferSinkCtx, "channel_layouts", outChannelLayouts, -1,
                              AV_OPT_SEARCH_CHILDREN)) < 0
							  || (ret = av_opt_set_int_list(m_bufferSinkCtx, "sample_rates", outSampleRates, -1,
		                              AV_OPT_SEARCH_CHILDREN)) < 0){
    	ret = VE_ERR_CREATE_FILTER_FAILED;
        goto AddFilters_end;
    }

    m_outputs->name       = av_strdup("in");
    m_outputs->filter_ctx = m_bufferSrcCtx;
    m_outputs->pad_idx    = 0;
    m_outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    m_inputs->name       = av_strdup("out");
    m_inputs->filter_ctx = m_bufferSinkCtx;
    m_inputs->pad_idx    = 0;
    m_inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(m_filterGraph, filtersExpr,
                                        &m_inputs, &m_outputs, NULL)) < 0){
    	ret = VE_ERR_CREATE_FILTER_FAILED;
        goto AddFilters_end;
    }


    if ((ret = avfilter_graph_config(m_filterGraph, NULL)) < 0){
    	ret = VE_ERR_CREATE_FILTER_FAILED;
    	goto AddFilters_end;
    }

    m_inited = true;

    return VE_ERR_OK;

AddFilters_end:

	if(m_inputs){
		avfilter_inout_free(&m_inputs);
		m_inputs = NULL;
	}
	if(m_outputs){
		avfilter_inout_free(&m_outputs);
		m_outputs = NULL;
	}
	if(m_frame){
		av_frame_free(&m_frame);
	}
	if(m_filtFrame){
		av_frame_free(&m_filtFrame);
	}

	return ret;
}
void VEAudioFilter::releaseMem(){

	if(m_filters){
		delete [] m_filters;
	}
	if(m_filterGraph){
		avfilter_graph_free(&m_filterGraph);
		m_filterGraph = NULL;
	}
	if(m_frame){
		av_frame_free(&m_frame);
	}
	if(m_filtFrame){
		av_frame_free(&m_filtFrame);
	}
	if(m_pcmBuf){
		av_freep(&m_pcmBuf);
	}
}
bool VEAudioFilter::isSameFilters(const char* filtersExpr){

	if(!filtersExpr)return false;

	if(!m_filters)return false;

	if(0 == strcmp(filtersExpr,m_filters)){
		return true;
	}
	return false;
}
int VEAudioFilter::process(AVFrame * srcFrame,short** dest_pcm,int *destPcmLen){

	int ret;
	char err[256] = {0};
	int curPcmSize = 0;
	*destPcmLen = 0;

	if ((ret = av_buffersrc_add_frame_flags(m_bufferSrcCtx, srcFrame, 0)) < 0) {
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"av_buffersrc_add_frame_flags failed!");
		return VE_ERR_CREATE_FILTER_FAILED;
	}

	/* pull filtered audio from the filtergraph */
	while (1) {
		ret = av_buffersink_get_frame(m_bufferSinkCtx, m_filtFrame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            break;
		}
		if (ret < 0){
			return VE_ERR_CREATE_FILTER_FAILED;
		}

		curPcmSize = m_filtFrame->nb_samples *av_get_bytes_per_sample(m_samplefmt) * m_channels;
		if(!m_pcmBuf){
			m_pcmBufLen = curPcmSize<<1;
			m_pcmBuf = (uint8_t*)av_mallocz(m_pcmBufLen);
/*
            if(!m_pcmBuf){
				return VE_ERR_MALLOC_FAILED;
			}
 */
		}else if(m_pcmBufLen >= (m_pcmBufDataLen + curPcmSize)){
		}else{
            m_pcmBufLen = (m_pcmBufDataLen + curPcmSize)* 2;
			m_pcmBuf = (uint8_t*)av_realloc(m_pcmBuf,m_pcmBufLen);
            /*
			if(!m_pcmBuf){
				return VE_ERR_MALLOC_FAILED;
			}
             */
		}
		memcpy(m_pcmBuf + m_pcmBufDataLen ,m_filtFrame->data[0],curPcmSize);
		m_pcmBufDataLen += curPcmSize;
		av_frame_unref(m_filtFrame);
	}
	*dest_pcm = (short*)m_pcmBuf;
	*destPcmLen = m_pcmBufDataLen;
    m_pcmBufDataLen = 0;

    return VE_ERR_OK;
}
#endif
