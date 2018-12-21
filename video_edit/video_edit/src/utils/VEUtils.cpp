#include "VEUtils.h"
#include "ve_interface.h"

int veLogEnable = 1;
log_printer veLogPrinter = NULL;
char ve_error_info[VE_ERROR_INFO_LEN];


void veLogCallbackFfmpeg(void *ptr,int level,const char *fmt,va_list vl)
{
    char line[2048];
    vsnprintf(line, sizeof(line) - 1, fmt, vl);
#ifdef __APPLE__
    VE_LOG_TAG_INFO(FFMPEG_TAG,line);
#endif
    /*
    if(level > 32){//verbose
        VE_LOG_INFO_TAG(FFMPEG_TAG,"%s:%s\n","[av_log]",line);
    }else if(level > 16){
        VE_LOG_INFO_TAG(FFMPEG_TAG,"%s:%s\n","[av_log]",line);
    }else{
        VE_LOG_ERROR_TAG(FFMPEG_TAG,"%s:%s\n","[av_log]",line);
    }
     */
    
}

extern "C"{

void videoEditLog(const char* TAG,int level, const char *fmt, ...)
{

#define VE_LINE_SIZE 2048

	char line[VE_LINE_SIZE] = {0};
    char num[10];
	int offset;
#if 1
    va_list vl;
    va_start(vl, fmt);

    strcat(line,TAG);

    offset = strlen(TAG);
    strcat(line,":");
    offset++;
    /*
    strcat(line,"[FUNCTION:");
    strcat(line,__FUNCTION__);
    strcat(line,",LINE:");
    snprintf(num,10,"%d",__LINE__);
    strcat(line,num);
    strcat(line,"]");
    */
    offset = strlen(line);

    vsnprintf(line + offset, VE_LINE_SIZE - offset, fmt, vl);

    strcat(line,"\n");

    if(veLogPrinter){
    	veLogPrinter(level,line);
    }else{
#ifdef __APPLE__
    	printf(line);
#endif
#ifdef __ANDROID__
    	if(level == 1){
    		__android_log_print(ANDROID_LOG_DEBUG, TAG, line);
    	}else if(level == 2){
    		__android_log_print(ANDROID_LOG_INFO, TAG, line);
    	}else if(level == 3){
    		__android_log_print(ANDROID_LOG_WARN, TAG, line);
    	}else if(level == 4){
    		__android_log_print(ANDROID_LOG_ERROR, TAG, line);
    	}
#endif
    }

    va_end(vl);
#endif
}
}
