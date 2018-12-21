#ifndef __VE_TYPE__
#define __VE_TYPE__


#include <stddef.h>
#include <stdint.h>

typedef void* HANDLE;

#define EXPORT_API __attribute__ ((visibility("default")))


//帧率控制
#define VE_MIN_FPS 1
#define VE_MAX_FPS 60
#define VE_DEFAULT_PFS 30

#define VE_MAX_MULTI_TRACK_NUM 1	//保留宏，不用

#define VE_MAX_TRACK_NUM 16	//最大轨道数

#define VE_MAX_FILTER_NUM 64	//同一时点最多filter数

#define BITRATE_720P	4500000	//720P视频的默认码率

#define BITRATE_AAC	(64000<<1)	//aac编码默认码率

typedef enum{
	VE_ROTATE_0,
	VE_ROTATE_90,
	VE_ROTATE_180,
	VE_ROTATE_270,
}VE_ROTATE;

typedef enum{
	VE_CODEC_UNKNOWN,
	VE_CODEC_H264,
	VE_CODEC_HEVC,
	VE_CODEC_AAC
}VE_CODEC;

typedef enum{
	VE_COLOR_YUV420P = 0,
	VE_COLOR_IOS_PIXELBUFFER,
	VE_COLOR_ANDROID_SURFACE_TEXTURE,//for android hw decoder
	VE_COLOR_BGRA,//for webm vp8
}VE_COLOR_TYPE;

typedef struct{
	int filter_id;
	int start_time;//for android,表示该特效开始时间，相对timeline时间
	int end_time;//for android,表示该特效结束时间，相对timeline时间
	const char* action;
}ve_filter_param;

typedef struct{
	int surface_texture_id{-1}; // GP ADDED.
	void* data{0};
	int len{0};
	int format{VE_COLOR_YUV420P};
	int width{0};
	int height{0};
	int rotate{VE_ROTATE_0};

	int clip_id{-1};
	int transition_id{-1};
	int transition_start_time{0};//转场开始时间 for android
	int transition_end_time{0};//转场结束时间 for android
	int transition_frame{0};//是否是转场帧(index_b转场部分）
	const char* transition_action{NULL};//转场特效

	ve_filter_param clip_filters[VE_MAX_FILTER_NUM];//clip特效

	int clip_filters_len{0};
	// void* clip_context{NULL};
	//下面是合成内部使用

	void* source{NULL};

}ve_v_frame_callback_param;



typedef struct{
	ve_v_frame_callback_param frame_data[2];//0 main track,1 transition track
	int track_id{-1};
	ve_filter_param track_filters[VE_MAX_FILTER_NUM];//track特效
	int track_filters_len{0};//track特效个数
	// void* track_context{NULL};
	int z_order{-1};//不用
}ve_track_callback_param;

typedef struct{
	ve_track_callback_param tracks[VE_MAX_TRACK_NUM];
	int tracks_num{0};

	ve_filter_param multitrack_filters[VE_MAX_FILTER_NUM];//目前是timeline特效
	int multitrack_filters_len{0};//目前是timeline特效个数
}ve_multitrack_callback_param;

typedef struct{
	ve_multitrack_callback_param multitracks[VE_MAX_MULTI_TRACK_NUM];//目前就multitracks[0]
	int multitrack_num{1};//目前没有多轨道，固定为1

	int timeline_id{-1};

	int cur_time{0};
	// void* timeline_context{NULL};
	bool end_frame{false};//for android
}ve_filter_callback_param;

typedef void (*ve_filter_callback)(HANDLE client_handle,ve_filter_callback_param* param,void* userExt);

#endif
