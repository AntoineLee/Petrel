#include "VEBitstream.h"
#include "VECommon.h"

VEBitstream::VEBitstream(const char* name){
	m_name = name;

	if(!m_name.compare("aac_adtstoasc")){
		m_bsfc = av_bitstream_filter_init("aac_adtstoasc");
	}else if(!m_name.compare("h264_mp4toannexb")){
		m_bsfc = av_bitstream_filter_init("h264_mp4toannexb");
	}else if(!m_name.compare("hevc_mp4toannexb")){
		m_bsfc = av_bitstream_filter_init("hevc_mp4toannexb");
	}

}
VEBitstream::~VEBitstream(){
    if(m_bsfc){
        av_bitstream_filter_close(m_bsfc);
    }
}


int VEBitstream::applyBitstreamFilter(AVCodecContext* codecCtx,AVPacket * pkt){
	if(m_bsfc){
		return av_apply_bitstream_filters(codecCtx,pkt,m_bsfc);
	}
	return 0;
}
