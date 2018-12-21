#ifndef __VE_BITSTREAM_H__
#define __VE_BITSTREAM_H__

#include <string>
struct AVCodecContext;
struct AVPacket;
struct AVBitStreamFilterContext;

class VEBitstream{

public:

	VEBitstream(const char* name);
	~VEBitstream();

	int applyBitstreamFilter(AVCodecContext* codecCtx,AVPacket * pkt);

private:

	std::string m_name;

	AVBitStreamFilterContext *m_bsfc{NULL};
};

#endif
