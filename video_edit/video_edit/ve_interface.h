#ifndef __VE_INTERFACE__
#define __VE_INTERFACE__


#include "ve_type.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
	VE_ERR_OK = 0,

	//内部错误码
	VE_ERR_FFMPEG_FAILED = -1199,
	VE_ERR_FFMPEG_TERMINATE,
	VE_ERR_SPEED_PARAM,
	VE_ERR_CODEC_PARAM,
	VE_ERR_SCALE_PARAM,
	VE_ERR_OUTPUT_FILE_PARAM,
	VE_ERR_WIDTH_HEIGHT_PARAM,
	VE_ERR_CHANNEL_PARAM,
	VE_ERR_SAMPLERATE_PARAM,
	VE_ERR_HANDLE_INVALID,
	VE_ERR_NO_MP4_FILE,
	VE_ERR_SCALE_FAILED,
	VE_ERR_NV12ToI420_FAILED,
	VE_ERR_TMPFILE_DIR_PARAM,
	VE_ERR_NO_THUMBNAIL_FILE,
	VE_ERR_CHANGE_VIDEO_PTS_FAILED,
	VE_ERR_TMPNAM_FAILED,
	VE_ERR_RENAME_FAILED,
	VE_ERR_SWR_CONVERT_FAILED,
	VE_ERR_VIDEOTOOLBOX_RETRIEVE_DATA_FAILED,

	//下面是对外错误码，公布给业务使用

	//-900 ~ -999 参数配置、合成公共
	VE_ERR_MALLOC_FAILED = -999,
	VE_ERR_OPEN_FILE_FAILED,
	VE_ERR_FILE_CORRUPT,
	VE_ERR_NO_BACKGOUND_MUSIC,
	VE_ERR_V_FILTER_EXIST,
	VE_ERR_V_FILTER_NOT_EXIST,
	VE_ERR_CREATE_THREAD_FAILED,

	//-800 ~ -899 参数配置相关

	VE_ERR_INPUT_PARAM = -899,

	//-700 ~ -799 合成相关

	VE_ERR_PARAM_CONFIG_LOCKED = -799,
	VE_ERR_EMPTY_PARAM_CONFIG,//参数配置为空就开始合成
	VE_ERR_CANNOT_FIND_VIDEO_STREAM,
	VE_ERR_CANNOT_FIND_AUDIO_STREAM,
	VE_ERR_NEW_STREAM_FAILED,
	VE_ERR_FIND_DECODER_FAILED,
	VE_ERR_OPEN_DECODER_FAILED,
	VE_ERR_OPEN_ENCODER_FAILED,
	VE_ERR_ENCODE_FAILED,
	VE_ERR_DECODE_FAILED,
	VE_ERR_CREATE_FILTER_FAILED,
	VE_ERR_AV_SEEK_FRAME_FAILED,
	VE_ERR_WRITE_FILE_FAILED,
	VE_ERR_FILL_AUDIO_FAILED,
	VE_ERR_PARAM_ERR,

	//-600 ~ -699 播放器相关
    VE_ERR_PREVIEW_PARAM = -699, //一切从参数配置模块获取信息出错
    VE_ERR_PREVIEW_OPENFILE_FAILED,
    VE_ERR_PREVIEW_SEEK_FAILED,
    VE_ERR_PREVIEW_DECODE_FIALED,
    VE_ERR_PREVIEW_CONNECT_FAILED,
}VE_ERR;


//C interface

/**
 * 设置日志输出
 * @param enable 1 输出日志，0不输出日志
 */
EXPORT_API void ve_enable_log(	int enable = 1 );

/*
 * 设置日志回调
 * @level 1 INFO,2 DEBUG,3 WARN 4 ERROR
 */
typedef void (*log_printer)(int level,const char* log);

EXPORT_API void ve_set_log_printer(	log_printer printer);
/**
 * ve开头的接口函数返回失败后，可以通过这个接口获取具体失败信息，用于定位具体是什么类型失败
 * @param info 输出的字符串数组
 * @param info_size 的长度，给1024就行
 */
EXPORT_API void ve_get_err_info(char* info,int info_size);

typedef struct{
	int timeline_id{-1};

	const char* filename{NULL};//导出文件名
	int output_width{720};//导出视频宽
	int output_height{1280};//导出视频高
	void* context{NULL};//上层透传字段

	int output_fps{30};//导出帧率
	int video_bitrate{BITRATE_720P};//导出视频比特率
	int audio_bitrate{BITRATE_AAC};//导出音频比特

	float speed{1.0};
	int volume{100};//0~200

}ve_timeline;

/**
 * 创建timeline（参数配置对象），后续可以传递给合成对象和预览播放器对象
 * @param timeline 剪辑参数，参考ve_timeline
 * @return	成功返回timeline的句柄，失败返回NULL
 */
EXPORT_API HANDLE ve_timeline_create(ve_timeline *timeline = NULL);
/**
 * 释放timeline（参数配置对象）
 * @param timeline_handle ve_timeline_create返回的句柄
 */
EXPORT_API void ve_timeline_free(HANDLE timeline_handle);

/**
 * 配置timeline剪辑参数
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param timeline 剪辑参数，参考ve_timeline
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_timeline_reconfig(HANDLE timeline_handle,ve_timeline *timeline);

/**
 * 获取timeline剪辑参数
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param timeline 输出参数，参考ve_timeline
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_timeline_get_config(HANDLE timeline_handle,ve_timeline *timeline);


/**
 * 获取timeline时长
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param actual 1 获取实际该轨道播放的时长，会受变速转场音响，actual 0 不受变速转场影响
 * @param duration 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_timeline_get_duration(HANDLE timeline_handle,int actual,int * duration);

typedef enum{
	VE_TRACK_VIDEO,
	VE_TRACK_AUDIO,
	//VE_MULTITRACK
}VE_TRACK;

typedef enum{
	VE_CLIP_ARRANGEMENT_SEQUENCE,//顺序模式，视频列表按顺序排列，通过index索引,一般用于视频主轴
	VE_CLIP_ARRANGEMENT_OVERLAY,//视频随意摆放，一般用于画中画
}VE_CLIP_ARRANGEMENT;

typedef struct{

	int track_id{-1};//调用方生成，用于唯一标识轨道，非负整数
	VE_TRACK type;//轨道类型（音频、视频）
	VE_CLIP_ARRANGEMENT clip_arrangement{VE_CLIP_ARRANGEMENT_SEQUENCE};

	float speed{1.0};
	int volume{100};

	void* context{NULL};//上层透传字段
	int z_order{0};//先保留

	//下面用于multitrack，先保留，不用

	typedef struct{
		int track_id{-1};
		int track_z_order{0};
	}track;
	track tracks[VE_MAX_MULTI_TRACK_NUM];//相互关联的轨道ids
	int track_num;//track_ids数组长度，元素个数
}ve_track;

/**
 * 添加轨道
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track 添加的轨道，参考ve_track
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_add(HANDLE timeline_handle,ve_track *track);
/**
 * 修改轨道相关参数，不能修改type（轨道类型）、clip_arrangement（clip排列方式）、track_id轨道id
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track 修改后的轨道参数，参考ve_track
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_mod(HANDLE timeline_handle,ve_track *track);
/**
 * 删除轨道
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id 轨道id
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_del(HANDLE timeline_handle,int track_id);
/**
 * 获取轨道信息
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track 输出参数，参考ve_track
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_get(HANDLE timeline_handle,int track_id,ve_track *track);
/**
 * 获取整个轨道时长
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id 轨道id
 * @param actual 1 获取实际该轨道播放的时长，会受变速转场音响，actual 0 不受变速转场音响
 * @param duration 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_get_overall_duration(HANDLE timeline_handle,int track_id,int actual,int * duration);

/**
 * 获取轨道时长
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id 轨道id
 * @param index 对于sequence轨道，表示获取index及之前的clips的总体时长，对于overlay轨道，忽略该参数
 * @param actual 1 获取实际该轨道播放的时长，会受变速转场音响，actual 0 不受变速转场音响
 * @param duration 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_get_duration(HANDLE timeline_handle,int index,int track_id,int actual,int * duration);



typedef struct{

	//编码
	int v_codec_format{-1};//视频编码格式
	int a_codec_format{-1};//音频编码格式

	int v_bitrate{0};//视频码率
	int a_bitrate{0};//音频码率
	int bitrate{0};//视频文件总的码率

	//视频参数
	//分辨率
	int width{0};//宽
	int height{0};//高
	int fps{1};//帧率
    int gop_size{-1};//gop

	//音频参数
	int channels{0};//声道数
	int samplerate{0};//采样率

	int sample_fmt{0};//格式
	int channelLayout{0};//layout

	//时长
	int duration{0};//视频时长

	int a_duration{0};//音频stream时长

	int v_duration{0};//视频stream时长

	const char* filename{NULL};

	VE_ROTATE rotate{VE_ROTATE_0};
	int picture{0};//图片

	int v_codec_id{0};
	int a_codec_id{0};

}ve_clip_info;


#define MAX_SLV_SEGMENT 64
typedef struct{

	//业务传入参数
	bool active{false};//是否生效，慢视频生效，普通视频不生效
	int start_time[MAX_SLV_SEGMENT];//每个片段开始时间，慢视频原始时间单位
	int end_time[MAX_SLV_SEGMENT];//每个片段结束时间，慢视频原始时间单位
	float speed[MAX_SLV_SEGMENT];//每个片段变慢后的速度值
	int len{5};//分多少段

	//内部计算出的字段
	int clip_start_time{0};
	int clip_end_time{0};
	float clip_speed{1.0};
}slv_info;

typedef enum{
	VE_CLIP_VIDEO,//视频
	VE_CLIP_PICTURE,//图片
	VE_CLIP_AUDIO	//音频
}VE_CLIP;

typedef struct{
	int track_id{-1};//该素材所在轨道id
	int clip_id{-1};//素材id，调用方生成，用于标识素材，非负整数
	void* context{NULL};//上层透传字段

	//输入参数

	VE_CLIP type{VE_CLIP_VIDEO};

	int insert_time{-1};//track的clip_arrangement为overlay时生效，表示要插入时间点，用于画中画
	int insert_index{-1}; // GP ADDED.

	/*
	 * VE_CLIP_VIDEO	等于end_time - start_time
	 * VE_CLIP_AUDIO	同上
	 * VE_CLIP_PICTURE	图片持续时长
	 */
	int duration{0};//片段时长，毫秒,

	const char* filename{NULL};//素材全路径(音频，视频，图片）
	const char* original_filename{NULL};//HEIC,HEIF文件原始文件名，高分辨率视频压缩到合规尺寸前的原始文件名
	int reverse{0};//1，实时倒放，0，不倒放



	int start_time{0};//素材(音频，视频）相对于该视频文件的开始时间，毫秒
	int end_time{0};//素材(音频，视频）相对于该视频文件的结束时间，毫秒


	VE_ROTATE picture_rotate{VE_ROTATE_0};//图片旋转角度

	int volume{100};//音量0~200，volume为0表示舍弃音频，做音视频分离

	float speed{1.0};//变速1/8 ~ 8

	slv_info slv;

	int pitch{0};//变声 -12~12

	//输出参数
	ve_clip_info info;//切片文件信息
}ve_clip;


/**
 * 获取视频信息
 *
 * 参数
 * filename				文件名
 * info					参考ve_clip_info结构体
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 *
 */
EXPORT_API VE_ERR ve_get_file_info(const char* filename,ve_clip_info* info);

/**
 * 插入文件clip到轨道
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param clip 文件切片，参考ve_clip
 * @param index track的clip_arrangement字段为sequence类型生效：
 * 								插入的位置，<=0 插到开头， >= 轨道文件个数 插到末尾
 * 							track的clip_arrangement字段为overlay类型时，参考ve_clip的insert_time字段来插入，index用默认值
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_insert(HANDLE timeline_handle,ve_clip *clip,int index = -1);
/**
 * 修改文件clip
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param clip 文件切片，参考ve_clip
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_mod(HANDLE timeline_handle,ve_clip *clip);

/**
 * 移动clip，对于sequence轨道起作用
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id 轨道id
 * @param from 要移动的clip index
 * @param to 要移动到的clip index
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_move(HANDLE timeline_handle,int track_id,int from,int to);

/**
 * 删除文件clip
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param clip_id 文件切片id
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_del(HANDLE timeline_handle,int  clip_id);
/**
 * 通过index删除文件clip，track的clip_arrangement字段为sequence类型生效
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id track id
 * @param index clip索引
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_del_by_index(HANDLE timeline_handle,int  track_id,int index);
/**
 * 获取文件clip信息
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param clip_id clip id
 * @param clip 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_get(HANDLE timeline_handle,int  clip_id,ve_clip* clip);
/**
 * 通过index获取文件clip信息，track的clip_arrangement字段为sequence类型生效
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id track id
 * @param index clip索引
 * @param clip 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_get_by_index(HANDLE timeline_handle,int  track_id,int index,ve_clip* clip);

/**
 * 通过clip_id获取clip时长（受变速影响）
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param clip_id clip id
 * @param duration 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_clip_get_duration(HANDLE timeline_handle,int  clip_id,int* duration);

/**
 * 获取轨道上的clip个数
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id track id
 * @param count 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_get_clips_count(HANDLE timeline_handle,int  track_id,int *count);
/**
 * 获取轨道上的clip数组
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param track_id track id
 * @param clips 输出参数，clip数组
 * @param len clip数组的长度
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_track_get_clips(HANDLE timeline_handle,int track_id,ve_clip*  clips,int len);

typedef enum{
	VE_FILTER_VIDEO,//视频特效
	VE_FILTER_AUDIO,//音频特效
}VE_FILTER;

typedef enum{
	VE_FILTER_LOC_TIMELINE,//timeline特效
	VE_FILTER_LOC_TRACK,//track特效
	VE_FILTER_LOC_CLIP,//文件特效
}VE_FILTER_LOC;

typedef enum{
	VE_AF_FADE_TRI,//线性
	VE_AF_FADE_QSIN,//正弦波
	VE_AF_FADE_ESIN,//指数正弦
	VE_AF_FADE_HSIN,//正弦波的一半
	VE_AF_FADE_LOG,//对数
	VE_AF_FADE_IPAR,//倒抛物线
	VE_AF_FADE_QUA,//二次方
	VE_AF_FADE_CUB,//立方
	VE_AF_FADE_SQU,//平方根
	VE_AF_FADE_CBR,//立方根
	VE_AF_FADE_PAR,//抛物线
	VE_AF_FADE_EXP,//指数
	VE_AF_FADE_IQSIN,//正弦波反季
	VE_AF_FADE_IHSIN,//倒一半的正弦波
	VE_AF_FADE_DESE,//双指数差值
	VE_AF_FADE_DESI,//双指数S弯曲

}VE_AF_FADE_CURVE;
typedef enum{
	VE_AUDIO_FILTER_NONE,
	VE_AUDIO_FILTER_FADE_IN,//淡入
	VE_AUDIO_FILTER_FADE_OUT,//淡出
	VE_AUDIO_FILTER_MAX,
}VE_AUDIO_FILTER_TYPE;

typedef struct{
	int track_id{-1};//track id,标识该特效绑定于track上
	int filter_id{-1};//调用方生成，用于唯一标识特效，非负整数


	VE_FILTER type{VE_FILTER_VIDEO};//音频特效，视频特效
	VE_FILTER_LOC loc_type{VE_FILTER_LOC_CLIP};//特效位置，基于timeline，track还是文件

	int clip_id{-1};//文件特效loc_type为VE_FILTER_LOC_FILE时启用，clip_id,clip_index可自由选用,只能设置1个,并且需要参考track_id
	int clip_index{-1};//文件特效loc_type为VE_FILTER_LOC_FILE时启用，clip_id,clip_index可自由选用，只能设置1个,并且需要参考track_id

	int start_time{-1};//开始时间，对于timeline和track特效，是基于timeline的时间；对于文件特效，是基于clip文件的偏移时间
	int end_time{-1};//结束时间，对于timeline和track特效，是基于timeline的时间；对于文件特效，是基于clip文件的偏移时间
	//视频特效
	const char* action{NULL};//视频特效模板

	//音频特效

	VE_AUDIO_FILTER_TYPE af_type{VE_AUDIO_FILTER_NONE};
	VE_AF_FADE_CURVE fade_curve{VE_AF_FADE_TRI};//淡入淡出激励函数，VE_AUDIO_FILTER_FADE_IN，VE_AUDIO_FILTER_FADE_OUT时启用

	int gain_min{0};//激励百分比，0~100,VE_AUDIO_FILTER_FADE_IN，VE_AUDIO_FILTER_FADE_OUT时启用
	int gain_max{100};//激励百分比，0~100,VE_AUDIO_FILTER_FADE_IN，VE_AUDIO_FILTER_FADE_OUT时启用
}ve_filter;

/**
 * 添加特效
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param filter 特效参数，参考ve_filter
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_filter_add(HANDLE timeline_handle,ve_filter *filter);
/**
 * 修改特效 type loc_type不能修改
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param filter 特效参数，参考ve_filter
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_filter_mod(HANDLE timeline_handle,ve_filter *filter);
/**
 * 删除特效
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param filter_id filter id
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_filter_del(HANDLE timeline_handle,int filter_id);
/**
 * 删除特效
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param filter_id filter id
 * @param filter 输出参数
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_filter_get(HANDLE timeline_handle,int filter_id,ve_filter *filter);



typedef struct{
	int transition_id{-1};//转场id
	int track_id{-1};//轨道id,该轨道clip_arrangement必须是VE_CLIP_ARRANGEMENT_SEQUENCE

	int clip_index_b{-1};//右边切片索引号（从0开始），比方在0，1之间加转场就填1
	int duration{1000};//转场时间默认1秒
	const char* action;//视频转场特效
}ve_transition;

/**
 * 添加转场
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param transition 转场参数，参考ve_transition
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_transition_add(HANDLE timeline_handle,ve_transition *transition);
/**
 * 修改转场
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param transition 转场参数，参考ve_transition
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_transition_mod(HANDLE timeline_handle,ve_transition *transition);
/**
 * 删除转场
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param transition_id transistion id
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_transition_del(HANDLE timeline_handle,int transition_id);

/**
 * 获取转场
 * @param timeline_handle ve_timeline_create返回的句柄
 * @param transition_id transistion id
 * @param transition 转场参数，参考ve_transition
 * @return	成功返回VE_ERR_OK，失败返回VE_ERR枚举，可通过ve_get_err_info获取具体失败信息
 */
EXPORT_API VE_ERR ve_transition_get(HANDLE timeline_handle,int transition_id,ve_transition *transition);

#ifdef __cplusplus
}
#endif

#include "ve_export.h"
#include "ve_thumbnail.h"
#ifdef __APPLE__
#include "ve_webm.h"
#else
#include "ve_webm_wrap.h"
#endif
#include "ve_player.h"
#include "ve_audio_producer.h"

#endif
