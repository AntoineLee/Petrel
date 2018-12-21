#ifndef __VE_THUMBNAIL_H__
#define __VE_THUMBNAIL_H__

#include "VECommon.h"
#include "ve_thumbnail.h"
#include "ve_interface.h"

typedef struct VEThumbnailKey{
public:
	std::string filename;
	int width{-1};
	int height{-1};

	bool operator<(const struct VEThumbnailKey & other) const;   // 两个const是必需的。

}VEThumbnailKey;


typedef struct VEThumbnailCmd{
public:
	std::string filename;
	int width{-1};
	int height{-1};
	int startTime{-1};
	int endTime{-1};
	VE_ROTATE rotate{VE_ROTATE_0};
	int count{0};
	int hwDecode{0};//for android
	std::string path;
	ve_thumbnail_callback callback{NULL};
	void* userExt{0};
	bool operator<(const struct VEThumbnailCmd & other) const;   // 两个const是必需的。

}VEThumbnailCmd;

typedef struct VEThumbnailPicture{
public:
	VEThumbnailPicture(){
	}
	int	m_pts{0};
	uint8_t* m_jpeg{0};
	int m_jpegLength{0};
	std::string m_filename;

}VEThumbnailPicture;


class VEHWAccel;
typedef struct VEThumbnailWorker{


	int workerExit{0};
	int decodingExit{0};
	std::thread * worker{0};

	std::string filename;

	int m_status{0};

	AVFormatContext *fmtCtx{NULL};
    uint8_t* m_yuv{NULL};		//for decode

    uint8_t* m_scaleYuv{NULL};	//for scale

    VEHWAccel* m_hwaccel{NULL};

    int m_format{-1};						//for encode type
    AVCodec* m_decoder{NULL};
    AVCodecContext* m_context{NULL};
    AVFrame*    m_frame{NULL};	//for decode
    AVFrame*		m_frameI420{NULL}; //swscale

    int m_swsCtxWidth{0};
    int m_swsCtxHeight{0};
    SwsContext* m_swsCtx{NULL};

    int64_t m_firstAudioPts{-1};
    int64_t m_firstThumbnailPts{-1};
    int m_videoStreamIndex{-1};
    int m_audioStreamIndex{-1};

    VEBitstream *m_h264Mp4toAnnexbBsfc{NULL};



    int m_lastFrameTs{0};

    AVPacket m_lastVPkt;
    bool m_savedLastVPkt{false};

    int m_firstAPktTsInMs{0};
    int m_firstVPktTsInMs{0};
    
    int m_curPktTsInMs{0};

    uint64_t m_lastOutputTime{0};


}VEThumbnailWorker;

typedef struct VEThumbnailPicList{
	std::list<VEThumbnailPicture*> list;
	comn::CriticalSection m_cs;
	int cacheCount{0};
}VEThumbnailPicList;
class VEThumbnail{
public:
	VEThumbnail(int maxMemCacheNum);
	~VEThumbnail();

	int getThumbnails(ve_thumbnail_param* param);

	void cancel();

	friend void thumbnailProcess(VEThumbnail *context,const VEThumbnailCmd  cmd,ve_clip_info info);


private:

	void clearThumbnailWorker(VEThumbnailWorker & worker);

	void stopWorkers();
	int getThumbnail(const VEThumbnailCmd * const cmd,VEThumbnailWorker & worker,VEThumbnailKey * key,VEThumbnailPicture* pic,int index);
	void insertThumbnail(VEThumbnailKey * key,VEThumbnailPicture* pic);
	int getOutputThumbnail(const VEThumbnailCmd* const cmd,VEThumbnailWorker & worker,VEThumbnailKey * key,ve_thumbnail_callback_param* param,int index);



	//void ManageMem();

	int m_maxMemCacheNum{300};
	int m_maxYuvQueueSize{10};


	std::map<VEThumbnailKey, VEThumbnailPicList> m_map;
	std::map<VEThumbnailCmd,VEThumbnailWorker> m_workers;
	std::map<std::string,ve_clip_info> m_videoInfos;

	int m_cancel{0};

	//接口保护
	comn::CriticalSection m_cs;

	comn::CriticalSection m_csMap;
	void *m_vtMode{NULL};
};

#endif
