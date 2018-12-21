#ifndef CN_360_LIVE_CLOUD_PUBLISH_VE_LOG_H_
#define CN_360_LIVE_CLOUD_PUBLISH_VE_LOG_H_

#define VE_LOG_TAG "[VE_LOG]"

#define VE_ERROR_INFO_LEN 1024

extern int veLogEnable;
extern char ve_error_info[VE_ERROR_INFO_LEN];

extern int ve_log_enable;

#ifdef __cplusplus
extern "C" void videoEditLog(const char* TAG,int level, const char *fmt, ...);
#else
void videoEditLog(const char* TAG,int level, const char *fmt, ...);
#endif


#ifdef __ANDROID__

// mediacodec decoder plgin log tag
#define TAGMEDIACODEC "[MediacodecDecoder]"

#include <android/log.h>

#define  VE_LOG_DEBUG(...)  if(veLogEnable){__android_log_print(ANDROID_LOG_INFO, VE_LOG_TAG, __VA_ARGS__);}
#define  VE_LOG_INFO(...)  if(veLogEnable){__android_log_print(ANDROID_LOG_INFO, VE_LOG_TAG, __VA_ARGS__);}
#define  VE_LOG_WARN(...)  if(veLogEnable){__android_log_print(ANDROID_LOG_INFO, VE_LOG_TAG, __VA_ARGS__);}
#define  VE_LOG_ERROR(...)  if(veLogEnable){__android_log_print(ANDROID_LOG_INFO, VE_LOG_TAG, __VA_ARGS__);}

#define  VE_LOG_TAG_INFO(tag,...) if(veLogEnable){videoEditLog(tag,1,__VA_ARGS__);}
#define  VE_LOG_TAG_DEBUG(tag,...)  if(veLogEnable){videoEditLog(tag,2,__VA_ARGS__);}
#define  VE_LOG_TAG_WARN(tag,...)  if(veLogEnable){videoEditLog(tag,3,__VA_ARGS__);}
#define  VE_LOG_TAG_ERROR(tag,...)  if(veLogEnable){videoEditLog(tag,4,__VA_ARGS__);}

#else

#include <stdio.h>


#define  VE_LOG_INFO(...) if(veLogEnable){videoEditLog(VE_LOG_TAG,1,__VA_ARGS__);}
#define  VE_LOG_DEBUG(...)  if(veLogEnable){videoEditLog(VE_LOG_TAG,2,__VA_ARGS__);}
#define  VE_LOG_WARN(...)  if(veLogEnable){videoEditLog(VE_LOG_TAG,3,__VA_ARGS__);}
#define  VE_LOG_ERROR(...)  if(veLogEnable){videoEditLog(VE_LOG_TAG,4,__VA_ARGS__);}

#define  VE_LOG_TAG_INFO(tag,...) if(veLogEnable){videoEditLog(tag,1,__VA_ARGS__);}
#define  VE_LOG_TAG_DEBUG(tag,...)  if(veLogEnable){videoEditLog(tag,2,__VA_ARGS__);}
#define  VE_LOG_TAG_WARN(tag,...)  if(veLogEnable){videoEditLog(tag,3,__VA_ARGS__);}
#define  VE_LOG_TAG_ERROR(tag,...)  if(veLogEnable){videoEditLog(tag,4,__VA_ARGS__);}

#endif
#endif
