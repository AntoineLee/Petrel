//
//  VEUtils.hpp
//  VEPlayer
//
//  Created by daxi on 18/7/13.
//  Copyright © 2016年 360. All rights reserved.
//

#ifndef ve_utils_hpp
#define ve_utils_hpp

#include <stdio.h>
#include <stdint.h>
#include "VEConfig.h"
#include "VEMediaSource.h"

#define BEGIN_LOOP_QUEUE_PACKET(queue,pkt)  \
while (1) \
{ \
    if(queue.putPacketBack(pkt)) \
        break; \
    else \
        usleep(20 * 1000);

#define END_LOOP_QUEUE_PACKET(exit,seek,pkt) \
if(exit || seek) \
{ \
    if(pkt.packet) \
        av_packet_free(&pkt.packet); \
    break; \
} \
}

#define BEGIN_LOOP_QUEUE_FRAME(manager,frame,id) \
while(1) \
{ \
    if(manager->writeFrame(frame,id) != true) \
    { \
        usleep(20 * 1000); \
    } \
    else \
        break;

#define END_LOOP_QUEUE_FRAME() \
}

struct segment;
struct VideoFrame2;
namespace  VEPlayerUtils
{
    //slv options
    int GetSlvDuration(slv_info* slv,int start_time,int end_time);
    int GetSlvStartTimeInOriginalTime(slv_info* slv,int start_time);
    int GetSlvOriginalToStartTime(slv_info *slv, int start_time);
    
    //int GetSegOriginDuration(segment *seg,int insert_start,int insert_end);
    int GetSegOriginTime(segment *seg,int insert_time);
    int GetSegRealTime(segment *seg,int clip_time);
    
    void GetPixelBufferFromNV12(int width,int height,uint8_t* y,uint8_t* uv,int linesize,void** pixelBuffer);
    void GetPixelBufferFromRGBA(int format,int width,int height,uint8_t* rgba_yuv[4],int linesize[4],void** pixelBuffer);
    
    ve_v_frame_callback_param VideoFrame2ParamFrame(VEConfig *config,VideoFrame2 frame);
    
    int64_t GetCurrentTime();
    int SetThreadHighPriority();
    //bool IsFloatEqual(float a,float b);
    
    int ReleaseVideoFrame(VideoFrame2 *frame);
    int ReleaseAudioFrame(AudioFrame2 *frame);
    VideoFrame2& CopyVideoFrame(VideoFrame2 src);
    //int ReleaseAVPacket(MediaPacket *pkt);
    
}
#endif

