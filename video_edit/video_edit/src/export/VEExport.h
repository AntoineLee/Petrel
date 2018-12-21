#ifndef __VE_EXPORT_H__
#define __VE_EXPORT_H__

#include "ve_interface.h"
#include "VEConfig.h"
#include "VESource.h"


class VESource;
class VEMp4Writer;
typedef struct{
	std::vector<VESource*> m_sources;
	ve_track	m_track;

	//下面为保留不用字段

	bool m_multitrack{false};
}VETrackSources;

class VEExport:public VESourceListener{
public:
	VEExport(VEConfig * config,ve_filter_callback filterCallback,ve_export_status_callback statusCallback,ve_export_param &param);
	~VEExport();
	void start();
	void cancel();
	void sendData(ve_filtered_data* filteredData);
	void setStatus(int status){m_status = status;}

	friend void * veExportProcess(void* context);
	friend void * veExportVFilterProcess(void* context);
	friend void * veExportAFilterProcess(void* context);

private:

	void getStatus(ve_export_status * exportStatus,int status,int errNo);
	//初始化资源
	int init();
	//退出出所有线程
	void stop();
	//释放对象资源
	void release();

	//进度计算
	void progressing();
	//视频特效处理&mix
	int videoFilterProcess();
	//音频特效处理&mix
	int audioFilterProcess();

	int getFilterCallbackParam(int curTime,ve_filter_callback_param & callbackParam);
    

	comn::CriticalSection m_cs;

	VEConfig* m_config{NULL};
	VEConfig	m_configCopy;

	void*	m_userExt{NULL};

	int m_render{0};//for android
	int m_hwEncode{0};//for android

	ve_filter_callback m_callback{NULL};
	ve_export_status_callback m_statusCallback{NULL};

	float m_progress{0};
	int m_status{0};
	int m_cancel{0};
	int m_done{0};
	int m_exit{1};

	pthread_t m_worker{0};
	pthread_t m_vFilterWorker{0};
	pthread_t m_aFilterWorker{0};

	std::vector<VETrackSources> m_vTrackSources;//视频

	std::vector<VETrackSources> m_aTrackSources;//音频

	ve_filter_callback_param m_callbackParam;

	int m_exportDuration{0};//导出总时长，毫秒
	int m_exportDurationUs{0};//导出总时长，微秒

	int m_curVFramePts{0};//当前视频帧时间，毫秒
	int64_t m_curAFramePts{0};//当前音频帧时间，微秒



	int m_yuvSendCount{0};
	int m_yuvSendBackCount{0};

	//std::vector<ve_track> m_multitracks;

	VEMp4Writer* m_veMp4writer;


	AVRational m_VTimebase{1,1000000};
	AVRational m_ATimebase{1,44100};
	int m_samplerate{44100};
	int m_channels{2};
	int m_channelLayout{AV_CH_LAYOUT_STEREO};
	int m_sampleSizePerFrame{1024};
	int m_outputPcmSize{VE_SOURCE_PCM_SAMPLE_SIZE};
	enum AVSampleFormat m_samplefmt{AV_SAMPLE_FMT_S16};
	int m_aBitrate{64000};

	uint8_t *m_pcmEmptyBuffer{0};
};


#endif /* __VE_EXPORT_H__ */
