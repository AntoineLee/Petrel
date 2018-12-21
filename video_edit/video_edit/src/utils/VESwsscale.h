#ifndef __VE_SWSSCALE_H__
#define __VE_SWSSCALE_H__

#include "VECommon.h"
#include "ve_interface.h"

class VESwsscale{
public:
	VESwsscale(){}
	~VESwsscale();

	static void getOpenGLWH(int width,int height,int* destWidth,int* destHeight);
	VE_ERR process(AVFrame* src,AVFrame** dest,int destFmt,int destWidth,int destHeight);
private:
    int m_swsCtxFormat{-1};
    int m_swsCtxWidth{0};
    int m_swsCtxHeight{0};
    SwsContext* m_swsCtx{NULL};

    AVFrame* m_vFrame{NULL};

    AVFrame*	m_vFrameI420{NULL}; //swscale
};


#endif /* __VE_SWSSCALE_H__ */
