#include "VEPlayerUtils.h"
#include "VEMediaSource.h"

#include <sys/time.h>
#include <time.h>
#include "string.h"
#include <math.h>
#include <pthread.h>

#ifdef __APPLE__
#include <VideoToolbox/VideoToolbox.h>
#endif



namespace  VEPlayerUtils{
    
int GetSlvDuration(slv_info* slv,int start_time,int end_time)
{
    if(slv->active){
        int slv_duration = 0;
        int slv_index = 0,slv_max = slv->len;
        
        if(start_time >= end_time){
            return 0;
        }
        for(;slv_index < slv_max;slv_index++){
            if(start_time >= slv->start_time[slv_index] && start_time < slv->end_time[slv_index] &&
               end_time > slv->start_time[slv_index] && end_time <= slv->end_time[slv_index]){
                slv_duration += (end_time - start_time) / slv->speed[slv_index];
                break;
            }
            
            if(start_time >= slv->start_time[slv_index] && start_time < slv->end_time[slv_index] &&
               end_time > slv->end_time[slv_index]){
                slv_duration += (slv->end_time[slv_index] - start_time) / slv->speed[slv_index];
                continue;
            }
            
            if(start_time <= slv->start_time[slv_index] && end_time > slv->start_time[slv_index] &&
               end_time <= slv->end_time[slv_index]){
                slv_duration += (end_time - slv->start_time[slv_index]) / slv->speed[slv_index];
                continue;
            }
            if(start_time <= slv->start_time[slv_index] && end_time >= slv->end_time[slv_index]){
                slv_duration += (slv->end_time[slv_index] - slv->start_time[slv_index]) / slv->speed[slv_index];
                continue;
            }
        }
        return slv_duration;
    }
    return 0;
}
    
int GetSlvStartTimeInOriginalTime(slv_info* slv,int start_time)
{
    if(slv->active){
        int prev_slv_duration = 0,slv_duration = 0,cur_slv_duration = 0;
        int original_start_time = 0,offset = 0;
        int slv_index = 0,slv_max = slv->len;
        
        if(start_time <= 0){
            return 0;
        }
        
        for(;slv_index < slv_max;slv_index++){
            
            cur_slv_duration = (slv->end_time[slv_index] -  slv->start_time[slv_index]) /  slv->speed[slv_index];
            
            slv_duration += cur_slv_duration;
            
            if(start_time <= slv_duration){
                
                offset = (start_time - prev_slv_duration) * slv->speed[slv_index];
                
                original_start_time = slv->start_time[slv_index] + offset;
                return original_start_time;
            }
            prev_slv_duration += cur_slv_duration;
        }
        
        return slv_duration;
        
    }else{
        return start_time;
    }
}
    
int GetSlvOriginalToStartTime(slv_info *slv, int start_time)
{
    if(slv->active){
        int prev_slv_duration = 0,slv_duration = 0,cur_slv_duration = 0;
        int original_start_time = 0,offset = 0;
        int slv_index = 0,slv_max = slv->len;
            
        if(start_time <= 0){
            return 0;
        }
            
        for(;slv_index < slv_max;slv_index++){
                
            cur_slv_duration = (slv->end_time[slv_index] -  slv->start_time[slv_index]);
                
            slv_duration += cur_slv_duration;
                
            if(start_time <= slv_duration){
                    
                offset = (start_time - prev_slv_duration) / slv->speed[slv_index];
                int j = 0;
                for(j;j<slv_index;j++)
                {
                    original_start_time += (slv->end_time[j] - slv->start_time[j]) / slv->speed[j];
                }
                original_start_time += offset;
                return original_start_time;
            }
            prev_slv_duration += cur_slv_duration;
        }
            
        return slv_duration;
            
    }else{
        return start_time;
    }
}

//timeline时间对应的clip时间
//int GetSegOriginDuration(segment *seg,int insert_start,int insert_end)
//{
//    int duration = 0;
//    int insert_start_ori = GetSegOriginTime(seg, insert_start);
//    int insert_end_ori = GetSegOriginTime(seg, insert_end);
//    
//    duration = insert_end_ori - insert_start_ori;
//    return duration;
//}

//clip packet读出来的时间
int GetSegOriginTime(segment *seg,int insert_time)
{
    int origin_time = 0;
    int offset = (insert_time - seg->insert_start_time) * seg->speed;
    if(seg->slv.active){
        int64_t rts_start = GetSlvOriginalToStartTime(&(seg->slv), seg->slv.clip_start_time);
        offset = GetSlvStartTimeInOriginalTime(&(seg->slv), rts_start + offset) - seg->slv.clip_start_time;
    }else{
        offset = seg->start_time + offset;
    }
    
    origin_time = offset; //* seg->speed;
    return origin_time;
}
    
int GetSegRealTime(segment *seg,int clip_time)
{
    int real_time = clip_time;
    if(seg->slv.active){
        int64_t rts_start = GetSlvOriginalToStartTime(&(seg->slv), seg->slv.clip_start_time);
        int64_t rts_end = GetSlvOriginalToStartTime(&(seg->slv), clip_time + seg->slv.clip_start_time);
        real_time = rts_end - rts_start;
    }
    real_time = real_time / seg->speed;
    return real_time;
}
    

void GetPixelBufferFromNV12(int width,int height,uint8_t* y,uint8_t* uv,int linesize,void** pixelBuffer)
{
#ifdef  __APPLE__
    OSType type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
    CVPixelBufferRef dstPixelbuffer = NULL;
    
    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);
    
    CVPixelBufferCreate(kCFAllocatorDefault, width, height, type, attrs, &dstPixelbuffer);
    CVPixelBufferLockBaseAddress(dstPixelbuffer, 0);
    void* dst_y = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 0);
    void* dst_uv = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 1);
    memcpy(dst_y, (uint8_t *)y, linesize * height);
    memcpy(dst_uv, (uint8_t *)uv, linesize * height /2);
    size_t y_linesize = CVPixelBufferGetBytesPerRowOfPlane(dstPixelbuffer, 0);
    
    if(y_linesize == linesize){
        memcpy(dst_y, (uint8_t *)y, linesize * height);
        memcpy(dst_uv, (uint8_t *)uv, linesize * height /2);
    }else{
        int min_linesize = y_linesize > linesize ? linesize:y_linesize;
        int i = 0;
        for(i;i<height;i++){
            memcpy((uint8_t *)dst_y+i*y_linesize, y+i*linesize,min_linesize);
        }
        i = 0;
        for(i;i<height/2;i++){
            memcpy((uint8_t *)dst_uv+i*y_linesize, uv+i*linesize,min_linesize);
        }
    }
    
    CVPixelBufferUnlockBaseAddress(dstPixelbuffer, 0);
    
    *pixelBuffer = (void*)dstPixelbuffer;
    CFRelease(empty);
    CFRelease(attrs);
#else
    *pixelBuffer = NULL;
#endif
    
}


void GetPixelBufferFromRGBA(int format,int width,int height,uint8_t* rgba_yuv[4],int linesize[4],void** pixelBuffer)
{
#ifdef  __APPLE__
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
    void* dst_rgba = CVPixelBufferGetBaseAddressOfPlane(dstPixelbuffer, 0);
    
    int bytes_per_row = CVPixelBufferGetBytesPerRow(dstPixelbuffer);
    
    uint8_t *dest = (uint8_t*)dst_rgba;
    
    int i,j,pos;
    if(kCVPixelFormatType_4444YpCbCrA8R == type){
        //TODO
    }else{
        if(bytes_per_row == linesize[0]){
            memcpy(dst_rgba, (uint8_t *)rgba_yuv[0], linesize[0] * height);
        }else{
            int i = 0;
            uint8_t *pData = rgba_yuv[0];
            for(i; i<height;i++){
                memcpy(dest+i*bytes_per_row, pData+i*linesize[0],linesize[0]);
            }
        }
        CVPixelBufferUnlockBaseAddress(dstPixelbuffer, 0);
    }
    
    *pixelBuffer = (void*)dstPixelbuffer;
    CFRelease(empty);
    CFRelease(attrs);
#else
    *pixelBuffer = NULL;
#endif
    
}
    
ve_v_frame_callback_param VideoFrame2ParamFrame(VEConfig *config,VideoFrame2 frame)
{
    ve_v_frame_callback_param frame_param;
    frame_param.data = frame.pData;

#ifdef __APPLE__
    frame_param.format = VE_COLOR_IOS_PIXELBUFFER;
#else //android
    if(frame.native)
        frame_param.format = VE_COLOR_ANDROID_SURFACE_TEXTURE;
    else if(frame.size > (frame.width * frame.height * 3))
        frame_param.format = VE_COLOR_BGRA;
    else
        frame_param.format = VE_COLOR_YUV420P;
#endif
    frame_param.width = frame.width;
    frame_param.height = frame.height;
    frame_param.transition_frame = frame.transitionId >= 0 ? 1 : 0;
    frame_param.transition_id = frame.transitionId;
    if(frame_param.transition_frame){
        std::map<int,VETrackData>::iterator it = config->m_tracks.begin();
        for(it;it != config->m_tracks.end();it++){
            std::map<int,VETransitionData>::iterator it_transiton = it->second.m_transitions.begin();
            for(it_transiton;it_transiton!=it->second.m_transitions.end();it_transiton++){
                if(it_transiton->second.m_transition.transition_id == frame.transitionId){
                    frame_param.transition_action = it_transiton->second.m_transition.action;
                    frame_param.transition_start_time = frame.startTime;
                    frame_param.transition_end_time = frame.endTime;
                }
            }
        }
    }
    frame_param.len = frame.size;
    frame_param.rotate = frame.rotate;
    frame_param.clip_id = frame.clipId;
    VEConfig::getClipVFilters(config, frame.trackId, frame.clipId, frame.clipTimestamp, frame_param);
    return frame_param;
}
    
int64_t GetCurrentTime()
{
    struct timeval t_start;
        
    //get start time
    gettimeofday(&t_start, NULL);
    int64_t start = ((long long)t_start.tv_sec)*1000+(long long)t_start.tv_usec/1000;
    return start;
}
    
int SetThreadHighPriority()
{
    struct sched_param sched;
    int policy;
    pthread_t thread = pthread_self();
        
    if (pthread_getschedparam(thread, &policy, &sched) < 0){
        return -1;
    }
    sched.sched_priority = sched_get_priority_max(policy);
 
    if (pthread_setschedparam(thread, policy, &sched) < 0){
        return -1;
    }
    return 0;
}
    
//bool IsFloatEqual(float a,float b)
//{
//    bool bEqual = false;
//    if(fabs(a-b) < 1e-7){
//        bEqual = true;
//    }
//    return bEqual;
//}
    
int ReleaseVideoFrame(VideoFrame2 *frame)
{
    if(frame && frame->pData && !frame->test){
        if(frame->native == 0){
            delete [] frame->pData;
        }else{
#ifdef __APPLE__
            CVPixelBufferRelease((CVImageBufferRef)frame->pData);
#endif
// #ifdef __ANDROID__
//                             m_sinkg->statusCallback(VIDEO_EDIT_PREVIEW_FRAME_DROP);
// #endif
        }
        frame->pData = NULL;
        frame->size = 0;
    }
    return 0;
}
    
int ReleaseAudioFrame(AudioFrame2 *frame)
{
    if(frame && frame->pData){
        delete [] frame->pData;
        frame->pData = NULL;
        frame->len = 0;
    }
    return 0;
}
    
//int ReleaseAVPacket(MediaPacket *pkt)
//{
//    if(pkt && pkt->packet){
//        av_packet_free(&pkt->packet);
//    }
//    return 0;
//}
    
VideoFrame2& CopyVideoFrame(VideoFrame2 src)
{
    VideoFrame2 dst;
    dst = src;
    if(src.native){
#ifdef __APPLE__
        CVPixelBufferRetain((CVImageBufferRef)dst.pData);
#else
        dst.textureId = src.textureId;
        dst.matrix = src.matrix;
#endif
    }else{
        dst.pData = new uint8_t[dst.size];
        memcpy(dst.pData, src.pData, dst.size);
    }
    return dst;
}
    
}

