#ifndef __VE_CONFIG_H__
#define __VE_CONFIG_H__

#include "ve_interface.h"
#include "VECommon.h"

typedef enum{
	VE_VT_MODE_PLAYER = 0,//播放器
	VE_VT_MODE_EXPORT, //合成导出
}VE_VT_MODE;
typedef struct{
	int m_mode{VE_VT_MODE_PLAYER};
}VEVTMode;

typedef struct{
	ve_filter m_filter;
	std::string m_action;
}VEFilterData;
typedef struct{
	ve_clip m_clip;
	std::string m_filename;
	std::string m_originalFilename;
	std::string m_reverseFilename;

	std::map<int,VEFilterData> m_filters;//文件特效


	VE_CLIP_ARRANGEMENT m_clipArrangement{VE_CLIP_ARRANGEMENT_SEQUENCE};


	//下面是合成内部使用

	int m_split{0};

	int m_actualVolume{100};
	float m_actualSpeed{1.0};

	int m_index{-1};
	int m_insertTime{-1};//cilp插入时间点,考虑转场变速

	int m_transitionId{-1};
	int m_transitionDuration{0};
	const char* m_transitionAction{NULL};

	//...
}VEClipData;
typedef struct{
	ve_transition m_transition;
	std::string m_action;
}VETransitionData;
typedef struct{
	ve_track m_track;
	std::deque<VEClipData> m_clips;
	//filterId,ve_filter_info*
	std::map<int,VEFilterData> m_filters;

	std::map<int,VETransitionData> m_transitions;
}VETrackData;

class VEConfig{
public:
	VEConfig(){VEConfig::initialize();}
	VEConfig(ve_timeline *timeline);
	~VEConfig();
	static void enableVELog(bool enable);
	static int m_veLogEnable;

	VEConfig & operator =(VEConfig & other);

	VE_ERR reconfig(ve_timeline *timeline);
	VE_ERR getConfig(ve_timeline *timeline);

	VE_ERR addTrack(ve_track *track);
	VE_ERR delTrack(int trackId);
	VE_ERR modTrack(ve_track *track);
	VE_ERR getTrack(int trackId,ve_track *track);

	VE_ERR insertClip(ve_clip *clip,int index);
	VE_ERR modClip(ve_clip *clip);
	VE_ERR moveClip(int trackId,int from,int to);
	VE_ERR delClip(int clipId);
	VE_ERR delClip(int trackId,int index);
	VE_ERR getClip(int clipId,ve_clip *clip);
	VE_ERR getClip(int trackId,int index,ve_clip *clip);
	VE_ERR getClipDuration(int clipId,int* duration);

	VE_ERR getClipsCount(int trackId,int *count);
	VE_ERR getClips(int trackId,ve_clip*  clips,int len);

	VE_ERR getTrackDuration(int trackId,int actual,int *duration);
	VE_ERR getTrackDuration(int trackId,int index,int actual,int *duration);
	VE_ERR getTimelineDuration(int actual,int *duration);



	VE_ERR addTransition(ve_transition *transition);
	VE_ERR modTransition(ve_transition *transition);
	VE_ERR delTransition(int transitionId);
	VE_ERR getTransition(int transitionId,ve_transition *transition);


	VE_ERR addFilter(ve_filter *filter);
	VE_ERR modFilter(ve_filter *filter);
	VE_ERR delFilter(int filterId);
	VE_ERR getFilter(int filterId,ve_filter *filter);

private:

	VE_ERR getTimelineDurationInternal(int actual,int *duration);
	bool filterExists(std::map<int,VEFilterData> & filterMap,int filterId);
	int transitionDuration(VETrackData* trackData,int indexB);
	bool transitionHasIndexB(VETrackData* trackData,int indexB);
	bool clipOverlay(VETrackData* trackData,ve_clip* clip);//判断clip是否有重叠
	bool invalidVolume(VETrackData* trackData,ve_clip* clip,int *actualVolume);//判断音量是否在不可接受范围
	bool invalidSlvInfo(slv_info* slv);
	bool invalidSpeed(VETrackData* trackData,VEClipData* clip,int index,float *actualSpeed);//判断变速是否在不可接受范围
	bool invalidSpeedForTrack(VETrackData* trackData,float newTrackSpeed);
	void insertClip(std::deque<VEClipData>& clips,VEClipData * clip,int moveIndex);
	void delTransition(VETrackData* trackData,int indexB);
	void reAssignFiltersStrPtr(std::map<int,VEFilterData> & filterMap);
	VE_ERR getTrackDurationInternal(int trackId,int index,int actual,int *duration);
public:
	//内部接口


	//v filter获取相关
	static void getTimelineVFilters(VEConfig* config,int vCurTime,ve_multitrack_callback_param & param);
	static void getTrackVFilters(VEConfig* config,int trackId,int vCurTime,ve_track_callback_param & param);
	static void getClipVFilters(VEConfig* config,int trackId,int clipId,int clipCurTime,ve_v_frame_callback_param & param);

	//转场获取相关
	void getTransitionFilter(VETrackData* trackData,std::map<int,VETransitionData> & transitions,int clipIndexB,VEClipData* clip);

	//获取clip实际时长
	int getClipDuration(VEClipData* clip);

	//调整volume和speed到合适的区间
	static void ajustVolume(VEConfig* config,VETrackData* trackData,ve_clip* clip,int *actualVolume);
	static void ajustSpeed(VEConfig* config,VETrackData* trackData,ve_clip* clip,float *actualSpeed);


	//慢视频相关内部接口
	static float getCurSlvSpeed(slv_info* slv,int curTime);//用于慢视频音频变速

	static int getSlvTime(slv_info* slv,int startTime);
	static int getSlvDuration(slv_info* slv,int startTime,int endTime);//用于慢视频视频变速

	static int getSlvOriginalStartTime(slv_info* slv,int startTime);
public:
    static int m_inited;
	static void initialize();
public:

	void* m_context{NULL};

	int m_id{-1};

	std::string m_filename;//导出文件名
	int m_width{0};//导出宽高
	int m_height{0};
	int m_bitrate{0};

	int m_outputFps{30};//导出帧率
	int m_videoBitrate{0};//导出视频比特率，暂时不用
	int m_audioBitrate{0};//导出音频比特率，暂时不用

	int m_volume{100};//timeline音量，实际处理的整体音量范围0~200，超出范围按边界处理
	float m_speed{1.0};//整体变速，实际处理的整体变速范围1/64 ~ 64,超出范围按边界处理

	int m_hwDecode{0};//for android

	std::map<int,VEFilterData> m_filters;//timeline特效
	//trackId ve_track_info*
	std::map<int,VETrackData> m_tracks;
	int m_multitrackNum{0};


	//接口保护
	comn::CriticalSection m_cs;
};
#endif
