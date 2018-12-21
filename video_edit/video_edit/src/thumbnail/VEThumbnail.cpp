#include "ve_interface.h"
#include "VEThumbnail.h"
#include "VESource.h"
#include "VEConfig.h"
#ifdef __APPLE__
#include "ios+ver.h"
#endif

#define VE_MODULE_TAG "[VEThumbnail]"
bool VEThumbnailKey::operator<(const struct VEThumbnailKey & other) const   // 两个const是必需的。
	{
		int result = filename.compare(other.filename);
		if(result < 0)return true;
        else if(result > 0)return false;
		else{
			if(width < other.width) return true;
			if(width > other.width) return false;

			return height < other.height;
		}
	}
bool VEThumbnailCmd::operator<(const struct VEThumbnailCmd & other) const   // 两个const是必需的。
    {

    	int result = filename.compare(other.filename);
    	if(result < 0)return true;
    	else if(result > 0)return false;
    	else{
    		if(width < other.width)return true;
    		else if(width > other.width)return false;
    		else{
    			if(height < other.height)return true;
    			else if(height > other.height)return false;
    			else{
        			if(startTime < other.startTime)return true;
        			else if(startTime > other.startTime)return false;
        			else{
            			if(endTime < other.endTime)return true;
            			else if(endTime > other.endTime)return false;
            			else{
                			if(count < other.count)return true;
                			else if(count > other.count)return false;
                			else{
                    			if(rotate < other.rotate)return true;
                    			else if(rotate > other.rotate)return false;
                    			else{
									if(callback < other.callback)return true;
									else if(callback > other.callback)return false;
									else{
										if(userExt < other.userExt)return true;
										else if(userExt > other.userExt)return false;
										else{
											result = path.compare(other.path);
                                            return result;
										}
									}
                    			}
                			}
            			}
        			}
    			}
    		}
    	}

    }
int saveI420ImageToFile(void* yuv,int length,int width,int height,const char* outputFile);
VEThumbnail::VEThumbnail(int maxMemCacheNum){

	/*
	if(maxMemCacheNum < 100){
		m_maxMemCacheNum = 100;
	}else if(maxMemCacheNum > 500){
		m_maxMemCacheNum = 500;
	}else{
		m_maxMemCacheNum = maxMemCacheNum;
	}
	*/
	m_maxMemCacheNum = 100;
	VEConfig::initialize();
	m_vtMode = (void*)new VEVTMode();
	((VEVTMode*)m_vtMode)->m_mode = VE_VT_MODE_EXPORT;
}
void VEThumbnail::clearThumbnailWorker(VEThumbnailWorker & worker){

    if(worker.m_context)
    {

        avcodec_close(worker.m_context);
        worker.m_context = NULL;
    }
    
    if(worker.m_h264Mp4toAnnexbBsfc){
    	delete worker.m_h264Mp4toAnnexbBsfc;
    	worker.m_h264Mp4toAnnexbBsfc = NULL;
    }

	if(worker.fmtCtx){
		avformat_close_input(&worker.fmtCtx);
	}
    

	if(worker.m_swsCtx){
		sws_freeContext(worker.m_swsCtx);
		worker.m_swsCtx = NULL;
	}
	if(worker.m_frame){
		av_frame_free(&worker.m_frame);
		worker.m_frame = NULL;
	}
	if(worker.m_frameI420){
		av_frame_free(&worker.m_frameI420);
		worker.m_frameI420 = NULL;
	}


	if(worker.m_yuv){
		delete [] worker.m_yuv;
		worker.m_yuv = NULL;
	}
	if(worker.m_scaleYuv){
		delete [] worker.m_scaleYuv;
		worker.m_scaleYuv = NULL;
	}

}
VEThumbnail::~VEThumbnail(){

	comn::AutoCritSec lock(m_cs);

	stopWorkers();


	std::map<VEThumbnailKey, VEThumbnailPicList>::iterator it2 = m_map.begin();

	for(;it2 != m_map.end();it2++){

		VEThumbnailPicList & picList = it2->second;
		std::list<VEThumbnailPicture*> & list = picList.list;

		//std::list<VEThumbnailPicture*>::iterator picIt = list.begin();

		while(!list.empty()){
			VEThumbnailPicture* picture = list.front();
            /*
			if(picture->m_jpeg){
				delete [] picture->m_jpeg;
				picture->m_jpeg = NULL;
			}
             */
			picture->m_jpegLength  = 0;
			if(picture->m_filename.size()){
				unlink(picture->m_filename.c_str());
			}
			//VE_LOG_INFO("VEThumbnail::~VEThumbnail picture->m_pts=%d",picture->m_pts);
			list.pop_front();
			delete picture;
		}
	}

	if(m_vtMode){
		delete m_vtMode;
		m_vtMode = NULL;
	}
}

void thumbnailProcess(VEThumbnail *context,const VEThumbnailCmd cmd,ve_clip_info info){

	VEThumbnailPicture pic;
	VEThumbnailWorker & worker = context->m_workers[cmd];
	VEThumbnailKey key;

	int ret;
	int index = 0;
	int rotate = cmd.rotate;


    VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d",cmd.startTime);

	key.filename = cmd.filename;
	key.width = cmd.width;
	key.height = cmd.height;

	/*
	if(!cmd.callback){
		worker.m_status = VE_ERR_INPUT_PARAM;
		worker.workerExit = 1;
		VE_LOG_ERROR("VEThumbnail::getThumbnails callback == NULL");
		return;
	}

	if(cmd.width <= 0 || cmd.height <= 0 || cmd.count <= 0){
		VE_LOG_ERROR("VEThumbnail::getThumbnails width <= 0 || height <= 0 || ");
		worker.m_status = VE_ERR_INPUT_PARAM;
		cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
		worker.workerExit = 1;
		return;
	}

	if(cmd.rotate < VE_ROTATE_0 || cmd.rotate > VE_ROTATE_270){
		VE_LOG_ERROR("VEThumbnail::getThumbnails cmd.rotate < VE_ROTATE_0 || cmd.rotate > VE_ROTATE_270");
		worker.m_status = VE_ERR_INPUT_PARAM;
		cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
		worker.workerExit = 1;
		return;
	}
*/

	if(context->m_videoInfos.find(cmd.filename) != context->m_videoInfos.end()){
		info = context->m_videoInfos[cmd.filename];
	}else{
		int ret = VESource::getSourceInfo(cmd.filename.c_str(),&info);

		if(ret){
			cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"GetVideoInfo failed");
			worker.m_status = VE_ERR_OPEN_FILE_FAILED;
			worker.workerExit = 1;
			return;
		}
		comn::AutoCritSec lock(context->m_csMap);
		context->m_videoInfos[cmd.filename] = info;
	}

	ve_clip_info & originalInfo = context->m_videoInfos[cmd.filename];
	if(info.a_codec_format == -1 && info.duration < 100){
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"picture thumbnail");
        info.rotate = originalInfo.rotate = (VE_ROTATE)cmd.rotate;
    }else{

    	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"info.a_codec_format=%d,info.duration=%d,originalInfo.rotate=%d",info.a_codec_format,info.duration,originalInfo.rotate);
        if(cmd.endTime > info.duration){
            cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
            worker.m_status = VE_ERR_INPUT_PARAM;
            VE_LOG_TAG_ERROR(VE_MODULE_TAG,"cmd.endTime > info.duration");
            worker.workerExit = 1;
        }
    }
	if(cmd.startTime < 0 || (cmd.startTime > info.duration && !info.picture) || cmd.endTime <= cmd.startTime){
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"GetVideoInfo failed startTime < 0 || startTime >= info.duration || endTime <= startTime");
		cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
		worker.m_status = VE_ERR_INPUT_PARAM;
		worker.workerExit = 1;
		return;
	}
	worker.decodingExit = 0;


    VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 2",cmd.startTime);
	while(!worker.workerExit){
		ve_thumbnail_callback_param param;
        VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 2.1",cmd.startTime);
		if(worker.m_status < 0)break;

        VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 2.2",cmd.startTime);
		ret = context->getOutputThumbnail(&cmd,worker,&key,&param,index);

		if(ret > 0){

            VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 2.2.1",cmd.startTime);
			if(!worker.workerExit){

                
				if(index == cmd.count -1){
                    VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 2.2.1.1",cmd.startTime);
					cmd.callback(context,&param,VE_THUMBNAIL_END,cmd.userExt);
					index++;
					delete param.output_jpegs;
					break;
				}else{
                    VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 2.2.1.1",cmd.startTime);
					cmd.callback(context,&param,VE_THUMBNAIL_PROCESSING,cmd.userExt);
					delete param.output_jpegs;
				}
				index++;

				continue;
            }else{
                VE_LOG_TAG_INFO(VE_MODULE_TAG,"!workerExit 1");
                //VE_LOG_INFO("thumbnailProcess debug startTime=%d 2.2.2",cmd.startTime);
            }

		}else if(ret < 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"exit 1:filename=%s,ret=%d,index=%d",cmd.filename.c_str(),ret,index);

            //VE_LOG_INFO("thumbnailProcess debug startTime=%d 2.2.3",cmd.startTime);
			worker.m_status = VE_ERR_NO_THUMBNAIL_FILE;
			if(!worker.workerExit){
				cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
			}

			break;
		}else{

            VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 2.2.4",cmd.startTime);

			ret = context->getThumbnail(&cmd,worker,&key,&pic,index);


			if(ret){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"exit 2:filename=%s,ret=%d,index=%d",cmd.filename.c_str(),ret,index);

				worker.m_status = ret;

				if(!worker.workerExit){
					cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
				}

				break;
			}

            VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 3",cmd.startTime);
			if(pic.m_filename.size()){
				//context->ManageMem();
                VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 4",cmd.startTime);
				context->insertThumbnail(&key,&pic);
			}

            VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 5",cmd.startTime);
            
			ret = context->getOutputThumbnail(&cmd,worker,&key,&param,index);

			if(ret > 0){

                
				if(!worker.workerExit){
                    
                    VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 6",cmd.startTime);

					VE_LOG_TAG_INFO(VE_MODULE_TAG,"pic.m_pts2=%d",pic.m_pts);
					if(index == cmd.count -1){
						cmd.callback(context,&param,VE_THUMBNAIL_END,cmd.userExt);
						index++;
						delete param.output_jpegs;
						break;
					}else{
						cmd.callback(context,&param,VE_THUMBNAIL_PROCESSING,cmd.userExt);


						delete param.output_jpegs;
					}
                }else{
                    
                    VE_LOG_TAG_INFO(VE_MODULE_TAG,"!workerExit 2");
                }

			}else{
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"exit 3:filename=%s,ret=%d,index=%d,pic.m_pts3=%d",cmd.filename.c_str(),ret,index,pic.m_pts);

				worker.m_status = VE_ERR_NO_THUMBNAIL_FILE;
				if(!worker.workerExit){
					cmd.callback(context,0,VE_THUMBNAIL_ERR,cmd.userExt);
				}

				break;
			}
		}
		index++;

	}

    VE_LOG_TAG_INFO(VE_MODULE_TAG,"debug startTime=%d 7",cmd.startTime);

	if(context->m_cancel){
		cmd.callback(context,0,VE_THUMBNAIL_CANCEL,cmd.userExt);
	}
	context->clearThumbnailWorker(worker);

	worker.workerExit = 1;
	VE_LOG_TAG_INFO(VE_MODULE_TAG,"exit:filename=%s,index=%d,width=%d,height=%d,startTime=%d,endTime=%d,count=%d,path=%s\n",cmd.filename.c_str(),index,cmd.width,cmd.height,cmd.startTime,cmd.endTime,cmd.count,cmd.path.c_str());
}
/*
void VEThumbnail::ManageMem(){

	VEThumbnailKey key;
	int count = 0,count2 = 0;
	bool need_clear_cache = false,need_clear_cache2 = false;
	std::map<VEThumbnailCmd,VEThumbnailWorker>::iterator it = m_workers.begin();


	for(;it != m_workers.end();it++){
		const VEThumbnailCmd & cmd = it->first;
		VEThumbnailWorker & woker = it->second;
		key.filename = cmd.filename;
		key.width = cmd.width;
		key.height = cmd.height;

        if(m_map.find(key) == m_map.end()){
            continue;
        }


		VEThumbnailPicList & list = m_map[key];

		count += list.cacheCount;
		if(!woker.workerExit){
			count2 +=  list.cacheCount;
		}
		if(count >= m_maxMemCacheNum){
			need_clear_cache = true;
			break;
		}
		if(count2 >= m_maxMemCacheNum){
			need_clear_cache2 = true;
			break;
		}
	}

	if(need_clear_cache){
		it = m_workers.begin();
		for(;it != m_workers.end();it++){
			const VEThumbnailCmd & cmd = it->first;
			VEThumbnailWorker & woker = it->second;
			key.filename = cmd.filename;
			key.width = cmd.width;
			key.height = cmd.height;

            if(m_map.find(key) == m_map.end()){
                continue;
            }

			VEThumbnailPicList & list = m_map[key];

			if(woker.workerExit){
				comn::AutoCritSec lock(list.m_cs);

				std::list<VEThumbnailPicture*>::iterator it2 = list.list.begin();

				for(;it2 != list.list.end();it2++){
					VEThumbnailPicture* pic = (*it2);
					if(pic->m_jpeg){
						delete [] pic->m_jpeg;
						pic->m_jpeg = 0;
						pic->m_jpegLength = 0;
						list.cacheCount--;
					}
				}
			}
		}
	}
#if 1
	if(need_clear_cache2){

		it = m_workers.begin();
		for(;it != m_workers.end();it++){
			const VEThumbnailCmd & cmd = it->first;
			VEThumbnailWorker & woker = it->second;
			key.filename = cmd.filename;
			key.width = cmd.width;
			key.height = cmd.height;

            if(m_map.find(key) == m_map.end()){
                continue;
            }

			VEThumbnailPicList & list = m_map[key];

			if(!woker.workerExit){
				comn::AutoCritSec lock(list.m_cs);
				int delete_count = 0;
				std::list<VEThumbnailPicture*>::iterator it2 = list.list.begin();

				for(;it2 != list.list.end();it2++){
					VEThumbnailPicture* pic = (*it2);
					if(pic->m_jpeg){
						delete [] pic->m_jpeg;
						pic->m_jpeg = 0;
						pic->m_jpegLength = 0;
						list.cacheCount--;
						delete_count++;
						if(delete_count >= 2)break;
					}
				}
			}
		}
	}
#endif

}
*/
int VEThumbnail::getThumbnail(const VEThumbnailCmd *const cmd,VEThumbnailWorker & worker,VEThumbnailKey * key,VEThumbnailPicture* pic,int index){


	ve_clip_info & info = m_videoInfos[cmd->filename];
    char err[256] = {0};
	int interval = (cmd->endTime - cmd->startTime) / cmd->count;
	int fpsInterval =  (info.fps > 5 && info.fps < 50)?1000 / info.fps:50;

	int findPts = (!info.picture) ? (cmd->startTime + interval * index):0,curPts;
    bool seekNeed = !index && findPts;
    int ret,count = 0,decodedFrameCount = 0,gotPicture = 0,yLength,dstY,eof = 0,flushBuffer = 1;
    int tryOpenDecoder = 1;
    int gopTime = 0;
    uint8_t* yuvI420;
    int yuvI420Length;
	char *strTmpFile = NULL;
	std::string jpegFilename;

	int64_t  thumbnailColorStart = 0;
    int64_t seekToVideoTime;
    AVPacket pkt;
	AVFrame frame;

	//VEThumbnailWorker & worker = m_workers[*cmd];

	if(!seekNeed && info.gop_size > 0 && info.fps > 5){
		gopTime = info.gop_size * 1000 / info.fps * 1.5;

		if(findPts - worker.m_lastFrameTs > gopTime){
			seekNeed = true;
		}
	}

	//打开文件
	if(!worker.fmtCtx){

		ret = VESource::openSource(&worker.fmtCtx,cmd->filename.c_str());

		if(ret || !worker.fmtCtx){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"openSource failed! :%s",cmd->filename.c_str());
			return ret;
		}
		//读帧,解码，获得一帧解码yuv

		worker.m_videoStreamIndex = av_find_best_stream(worker.fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		worker.m_audioStreamIndex = av_find_best_stream(worker.fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

		if(worker.m_videoStreamIndex < 0){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"cannot find video steam for file:%s",cmd->filename.c_str());
			return VE_ERR_CANNOT_FIND_VIDEO_STREAM;
		}
		if(!worker.m_decoder){

#ifdef __APPLE__
			if(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id ==AV_CODEC_ID_H264){
				worker.m_decoder = &h264_videotoolbox_decoder;
				worker.m_h264Mp4toAnnexbBsfc = new VEBitstream("h264_mp4toannexb");
				//worker.m_decoder = avcodec_find_decoder(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
			}else if(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id == AV_CODEC_ID_HEVC && isIOSVerAbove(11.0)){
				worker.m_decoder = &hevc_videotoolbox_decoder;
				worker.m_h264Mp4toAnnexbBsfc = new VEBitstream("hevc_mp4toannexb");
                //worker.m_decoder = avcodec_find_decoder(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
			}else{
				worker.m_decoder = avcodec_find_decoder(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
			}
#endif
#ifdef __ANDROID__
			worker.m_decoder = avcodec_find_decoder(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
#endif

#ifdef __ANDROID__
			VE_LOG_TAG_INFO(VE_MODULE_TAG,"hwDecode=%d",cmd->hwDecode);
			if(AV_CODEC_ID_H264 == worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id){
#ifdef _VE_PUBLISH_
				worker.m_decoder = &ff_h264_decoder;
#else
				if(cmd->hwDecode && GetAndroidVersion() >= 19){
					VE_LOG_TAG_INFO(VE_MODULE_TAG,"h264_mediacodec");
					//worker.m_decoder = &ff_h264_decoder;
					worker.m_decoder = &ff_h264_mediacodec_decoder;
					worker.m_h264Mp4toAnnexbBsfc = new VEBitstream("h264_mp4toannexb");
				}else{
					worker.m_decoder = &ff_h264_decoder;
				}
#endif
			}else if(AV_CODEC_ID_HEVC == worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id){
#ifdef _VE_PUBLISH_
				worker.m_decoder = avcodec_find_decoder(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
#else
				if(cmd->hwDecode && GetAndroidVersion() >= 19){
					VE_LOG_TAG_INFO(VE_MODULE_TAG,"hevc_mediacodec");
					//worker.m_decoder = &ff_h264_decoder;
					worker.m_decoder = &ff_hevc_mediacodec_decoder;
					worker.m_h264Mp4toAnnexbBsfc = new VEBitstream("hevc_mp4toannexb");
				}else{
					worker.m_decoder = avcodec_find_decoder(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
				}
#endif
			}else{
				worker.m_decoder = avcodec_find_decoder(worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
			}
#endif
            
			if(!worker.m_decoder){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_find_decoder failed,video_codec_id=%d",worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->codec_id);
				return VE_ERR_FIND_DECODER_FAILED;
			}
		}



		if(!worker.m_context){
			worker.m_context = worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec;

			if(!worker.m_context){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"worker.m_context == NULL");
				return VE_ERR_MALLOC_FAILED;
			}
#ifdef __APPLE__
			if(worker.m_decoder == &h264_videotoolbox_decoder || worker.m_decoder == &hevc_videotoolbox_decoder){
				worker.m_context->pix_fmt = AV_PIX_FMT_VIDEOTOOLBOX;
				worker.m_context->opaque = m_vtMode;
			}

#endif
			do{
				if ((ret = avcodec_open2(worker.m_context, worker.m_decoder, NULL)) < 0) {
					if(tryOpenDecoder){
						tryOpenDecoder = 0;
						continue;
					}
					VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_open2 failed,video_codec_id=%d",worker.fmtCtx->video_codec_id);
					return VE_ERR_OPEN_DECODER_FAILED;
				}
				break;
			}while(1);

		}
		if(!worker.m_frame){
			worker.m_frame = av_frame_alloc();
		}
	}

	av_init_packet(&pkt);



	do{
		/* read frames from the file */
		while (1) {
			pkt.data = NULL;
			pkt.size = 0;
			gotPicture = 0;
			if(!eof){
				/*
				if(worker.m_savedLastVPkt){
					pkt = worker.m_lastVPkt;
					worker.m_savedLastVPkt = false;
					ret = 0;
				}else
				*/{
					ret = av_read_frame(worker.fmtCtx, &pkt);
				}
				if( ret < 0){
					av_strerror(ret, err, 256);
					VE_LOG_TAG_ERROR(VE_MODULE_TAG,"av_read_frame < 0 startTime=%d,endTime=%d,file:%s,ret=%d,err info:%s",cmd->startTime,cmd->endTime,cmd->filename.c_str(),ret,err);

					eof = 1;
					av_packet_unref(&pkt);
					pkt.data = NULL;
					pkt.size = 0;
					pkt.stream_index = worker.m_videoStreamIndex;
				}

 
				if(pkt.stream_index == worker.m_audioStreamIndex && worker.m_firstAudioPts < 0){

					if(pkt.pts < 0){
						pkt.pts = 0;
					}
					worker.m_firstAudioPts = pkt.pts;
					worker.m_firstAPktTsInMs = av_rescale_q(pkt.pts,worker.fmtCtx->streams[pkt.stream_index]->time_base,(AVRational){ 1, 1000 });

                    /*
					av_packet_unref(&pkt);
                    
                    seekToVideoTime = av_rescale_q(0,(AVRational){ 1, 1000 },worker.fmtCtx->streams[worker.m_videoStreamIndex]->time_base);
					ret = av_seek_frame(worker.fmtCtx,worker.m_videoStreamIndex,seekToVideoTime,AVSEEK_FLAG_BYTE);

					if(ret < 0){
						VE_LOG_ERROR("VEThumbnail::getThumbnail av_seek_frame failed,filename=%s",cmd->filename.c_str());
						return VE_ERR_AV_SEEK_FRAME_FAILED;
					}
					continue;
                     */
                    
				}
				if(pkt.stream_index != worker.m_videoStreamIndex){
					av_packet_unref(&pkt);
					continue;
				}
			}
            if(worker.m_firstAudioPts < 0){
                worker.m_firstAudioPts = 0;
                worker.m_firstAPktTsInMs = 0;
            }
			worker.m_lastVPkt = pkt;
            worker.m_curPktTsInMs = av_rescale_q(pkt.pts,worker.fmtCtx->streams[worker.m_videoStreamIndex]->time_base,(AVRational){ 1, 1000 });
			if(worker.m_firstThumbnailPts < 0){
				worker.m_firstVPktTsInMs = av_rescale_q(pkt.pts,worker.fmtCtx->streams[pkt.stream_index]->time_base,(AVRational){ 1, 1000 });
				worker.m_firstThumbnailPts = pkt.pts;
			}
			//seek

			if(seekNeed){
                seekToVideoTime = av_rescale_q(findPts,(AVRational){ 1, 1000 },worker.fmtCtx->streams[worker.m_videoStreamIndex]->time_base);

				av_packet_unref(&pkt);

                if(worker.fmtCtx->streams[worker.m_videoStreamIndex]->index_entries){
                	seekToVideoTime += worker.fmtCtx->streams[worker.m_videoStreamIndex]->index_entries[0].timestamp;
                }

				ret = av_seek_frame(worker.fmtCtx,worker.m_videoStreamIndex,seekToVideoTime,AVSEEK_FLAG_BACKWARD);

				if(ret < 0){
					VE_LOG_TAG_ERROR(VE_MODULE_TAG,"av_seek_frame failed,filename=%s",cmd->filename.c_str());
					return VE_ERR_AV_SEEK_FRAME_FAILED;
				}
				seekNeed = false;
				continue;
			}

			//如果是有音频的视频文件，前面有音频没视频，按黑帧补齐
			/*
			if(!info.picture && worker.m_audioStreamIndex >= 0 &&  worker.m_firstVPktTsInMs - findPts > fpsInterval){
				worker.m_savedLastVPkt = true;
				frame.width = worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->width;
				frame.height = worker.fmtCtx->streams[worker.m_videoStreamIndex]->codec->height;
				yLength = frame.width * frame.height;
				yuvI420Length = yLength * 3/2;
				curPts = findPts;
				gotPicture = 1;
				break;
			}
			*/
			//解码
			if(worker.m_h264Mp4toAnnexbBsfc){
				worker.m_h264Mp4toAnnexbBsfc->applyBitstreamFilter(worker.m_context,&pkt);
			}
			if(!worker.m_lastOutputTime){
				worker.m_lastOutputTime = av_gettime();
			}
			avcodec_decode_video2(worker.m_context, worker.m_frame, &gotPicture, &pkt);
			if(gotPicture){


				worker.m_lastOutputTime = av_gettime();
				decodedFrameCount++;
				av_packet_unref(&pkt);

				curPts = av_rescale_q((worker.m_frame->pkt_pts - worker.m_firstThumbnailPts),worker.fmtCtx->streams[worker.m_videoStreamIndex]->time_base,(AVRational){ 1, 1000 });
				curPts += worker.m_firstVPktTsInMs - worker.m_firstAPktTsInMs;
				if(curPts < 0){
					curPts = 0;
				}
				VE_LOG_TAG_INFO(VE_MODULE_TAG,"startTime=%d,endTime=%d,decode diff=%llu,curPts=%d",cmd->startTime,cmd->endTime,av_gettime() - worker.m_lastOutputTime,curPts);
				if(info.picture)break;

				if( fabs(curPts - findPts) < fpsInterval || curPts > findPts){
					worker.m_lastFrameTs = curPts;
					break;
				}else{
					continue;
				}

			}else{
				if(eof){
					flushBuffer = 0;
					break;
				}
			}
			//释放包
			av_packet_unref(&pkt);
		}
		/*
		if(worker.m_savedLastVPkt){
			break;
		}
		*/
		if(gotPicture){
			thumbnailColorStart = av_gettime();

			//颜色空间
			if(worker.m_frame->format != AV_PIX_FMT_YUV420P){
				VE_LOG_TAG_INFO(VE_MODULE_TAG,"need sws_scale worker.m_context->pix_fmt=%d",worker.m_context->pix_fmt);
				if(worker.m_swsCtxWidth != worker.m_frame->width || worker.m_swsCtxHeight != worker.m_frame->height){
					sws_freeContext(worker.m_swsCtx);
					worker.m_swsCtx = NULL;
					worker.m_swsCtxWidth = worker.m_frame->width;
					worker.m_swsCtxHeight = worker.m_frame->height;

					if(worker.m_yuv){
						delete[] worker.m_yuv;
						worker.m_yuv = NULL;
					}

				}
				if(!worker.m_swsCtx){
					worker.m_swsCtx = sws_getContext(worker.m_frame->width, worker.m_frame->height,
							(enum AVPixelFormat)worker.m_frame->format,
							worker.m_frame->width, worker.m_frame->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, 0, 0, 0);
					if(!worker.m_swsCtx){
						VE_LOG_TAG_ERROR(VE_MODULE_TAG,"sws_getContext failed!");
						return VE_ERR_MALLOC_FAILED;
					}
				}
				yLength = worker.m_swsCtxWidth  * worker.m_swsCtxHeight;
				if(!worker.m_yuv){
					worker.m_yuv = new uint8_t[yLength * 3/2];
					if(!worker.m_yuv){
						VE_LOG_TAG_ERROR(VE_MODULE_TAG,"worker.m_yuv = new uint8_t failed!");
						return VE_ERR_MALLOC_FAILED;
					}
				}

				if(!worker.m_frameI420 || worker.m_frameI420->width != worker.m_frame->width || worker.m_frameI420->height != worker.m_frame->height){

					av_frame_free(&worker.m_frameI420);
					worker.m_frameI420 = NULL;

					worker.m_frameI420 = av_frame_alloc();
					if(!worker.m_frameI420){
						VE_LOG_TAG_ERROR(VE_MODULE_TAG,"worker.m_frameI420 av_frame_alloc failed!");
						return VE_ERR_MALLOC_FAILED;
					}

					worker.m_frameI420->width = worker.m_frame->width;
					worker.m_frameI420->height = worker.m_frame->height;
					worker.m_frameI420->format = AV_PIX_FMT_YUV420P;
					av_frame_get_buffer(worker.m_frameI420, 1);

				}


				sws_scale(worker.m_swsCtx, worker.m_frame->data, worker.m_frame->linesize, 0, worker.m_frame->height,worker.m_frameI420->data, worker.m_frameI420->linesize);

				frame = *worker.m_frameI420;

			}else{
				frame = *worker.m_frame;
			}


			yLength = frame.width * frame.height;
			yuvI420Length = yLength * 3/2;
			if(!worker.m_yuv){
				worker.m_yuv = new uint8_t[yuvI420Length];
				/*
				if(!worker.m_yuv){
					VE_LOG_ERROR("VEThumbnail::getThumbnail worker.m_yuv = new uint8_t failed!");
					return VE_ERR_MALLOC_FAILED;
				}
				*/
			}

			uint8_t* i420_out = (uint8_t*)worker.m_yuv;

			libyuv::I420Copy((uint8_t*)frame.data[0],frame.linesize[0],(uint8_t*)frame.data[1],frame.linesize[1],(uint8_t*)frame.data[2],frame.linesize[2],i420_out, frame.width,
					i420_out + yLength, frame.width / 2,
					i420_out + yLength * 5 / 4, frame.width / 2,
					frame.width, frame.height);

		}

	}while(!gotPicture && flushBuffer);

	if(thumbnailColorStart){
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"thumbnail_color_cost=%lld",(av_gettime() - thumbnailColorStart)/ 1000);
		thumbnailColorStart = av_gettime();
	}

	if(!gotPicture){

    	pic->m_pts = 0;
    	pic->m_filename = "";
    	pic->m_jpeg = NULL;
    	pic->m_jpegLength = 0;

    	return VE_ERR_OK;
    }
	/*
	if(worker.m_savedLastVPkt){
		yLength = frame.width * frame.height;
		yuvI420Length = yLength * 3/2;
		if(!worker.m_yuv){
			worker.m_yuv = new uint8_t[yuvI420Length];
			if(!worker.m_yuv){
				VE_LOG_ERROR("VEThumbnail::getThumbnail worker.m_yuv = new uint8_t failed!");
				return VE_ERR_MALLOC_FAILED;
			}
		}
    	memset(worker.m_yuv,0,yuvI420Length);
    	memset(worker.m_yuv + yLength,128,yuvI420Length - yLength);
	}
	*/
    thumbnailColorStart = av_gettime();
	//缩放
    dstY = cmd->width * cmd->height;
    yuvI420Length = dstY * 3/2;
	if(frame.width != cmd->width || frame.height != cmd->height){

		uint8_t* src;
		src = worker.m_yuv;

		yLength = frame.width * frame.height;

		if(!worker.m_scaleYuv){
			worker.m_scaleYuv = new uint8_t[dstY * 3/2];
			if(!worker.m_scaleYuv){
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"worker.m_scaleYuv = new uint8_t failed!");
				return VE_ERR_MALLOC_FAILED;
			}
		}
		/*
        if(worker.m_savedLastVPkt){
            ret = I420Scale(src, frame.width,
                            src + yLength, frame.width / 2,
                            src + yLength * 5 / 4, frame.width / 2,
                            frame.width, frame.height,
                            worker.m_scaleYuv, cmd->width,
                            worker.m_scaleYuv + dstY, cmd->width / 2,
                            worker.m_scaleYuv + dstY * 5 / 4, cmd->width / 2,
                            cmd->width, cmd->height,
                            libyuv::kFilterNone);
        }else
        */{
            ret = I420Scale(frame.data[0], frame.linesize[0],
                            frame.data[1], frame.linesize[1],
                            frame.data[2], frame.linesize[2],
                            frame.width, frame.height,
                            worker.m_scaleYuv, cmd->width,
                            worker.m_scaleYuv + dstY, cmd->width / 2,
                            worker.m_scaleYuv + dstY * 5 / 4, cmd->width / 2,
                            cmd->width, cmd->height,
                            libyuv::kFilterNone);
        }
		if(ret){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"I420Scale failed!");
			return VE_ERR_SCALE_FAILED;
		}
		yuvI420 = worker.m_scaleYuv;

	}else{
		yuvI420 = worker.m_yuv;
	}
    if(thumbnailColorStart){
    	VE_LOG_TAG_INFO(VE_MODULE_TAG,"thumbnail_color_cost2=%lld",(av_gettime() - thumbnailColorStart)/ 1000);
    	thumbnailColorStart = av_gettime();
    }
	//保存jpeg文件

	if(! (strTmpFile = tempnam(cmd->path.c_str(),NULL))){
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"tmpnam failed");
		return VE_ERR_TMPNAM_FAILED;
	}
	jpegFilename = strTmpFile;
	jpegFilename += ".jpeg";
	free(strTmpFile);
	if(ret = saveI420ImageToFile(yuvI420,yuvI420Length,cmd->width,cmd->height,jpegFilename.c_str())){

		VE_LOG_TAG_INFO(VE_MODULE_TAG,"saveI420ImageToFile failed");
		return ret;
	}

	pic->m_filename = jpegFilename;
	pic->m_jpeg = NULL;
	pic->m_jpegLength = 0;
    pic->m_pts = curPts;

    if(thumbnailColorStart){
    	VE_LOG_TAG_INFO(VE_MODULE_TAG,"thumbnail_color_cost3=%lld",(av_gettime() - thumbnailColorStart)/ 1000);
    }

	return VE_ERR_OK;
}
void VEThumbnail::insertThumbnail(VEThumbnailKey * key,VEThumbnailPicture* pic){

	{
		comn::AutoCritSec lock(m_csMap);

		if(m_map.find(*key) == m_map.end()){
			m_map[*key] = VEThumbnailPicList();
		}
	}
	VEThumbnailPicList & picList = m_map[*key];

    int size = m_map.size();
	std::list<VEThumbnailPicture*> & list = picList.list;
	comn::AutoCritSec lock(picList.m_cs);

	std::list<VEThumbnailPicture*>::iterator it = list.begin();

	VEThumbnailPicture* insertPic = new VEThumbnailPicture();


	*insertPic = *pic;
	//VE_LOG_INFO("VEThumbnail::insertThumbnail insertPic->m_pts=%d",insertPic->m_pts);

	VEThumbnailPicture* curPic;
	for(;it != list.end();it++){
		curPic = (*it);
		if(pic->m_pts < curPic->m_pts){
			list.insert(it,insertPic);
			break;
		}
	}
	if(it == list.end()){
		list.push_back(insertPic);
	}
}

int VEThumbnail::getOutputThumbnail(const VEThumbnailCmd* const cmd,VEThumbnailWorker & worker,VEThumbnailKey * key,ve_thumbnail_callback_param* param,int index){
	FILE* jpegFile  = NULL;
	ve_clip_info & info = m_videoInfos[cmd->filename];
	int fpsInterval =  (info.fps > 5 && info.fps < 50)?1000 / info.fps:50;
	int interval = (cmd->endTime - cmd->startTime) / cmd->count;
	int findPts = cmd->startTime + interval * index;
	int found = 0;
	int temp;
	int ret;

#ifdef __ANDROID__
	fpsInterval = fpsInterval * 6/5;
#endif
    if(m_map.find(*key) == m_map.end()){
        return VE_ERR_OK;
    }
	VEThumbnailPicList & picList = m_map[*key];
	std::list<VEThumbnailPicture*> & list = picList.list;
	VEThumbnailPicture* curPic,*prevPic;
	//VEThumbnailWorker & worker = m_workers[*cmd];
	do{
		comn::AutoCritSec lock(picList.m_cs);
		if(info.picture){

			std::list<VEThumbnailPicture*>::iterator it = list.begin();
			if(list.size () && it != list.end()){
				curPic = (*it);
				found = 1;
			}else{
				return VE_ERR_OK;
			}
		}
		do{
			std::list<VEThumbnailPicture*>::iterator it = list.begin();

            int size = list.size();


            if(size){
    			for ( ; it !=  list.end(); ++it){
    				curPic = (*it);
    				if( fabs(curPic->m_pts - findPts) < fpsInterval || curPic->m_pts > findPts){
    					//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 1 curPic->m_pts=%d",curPic->m_pts);
    					break;
    				}
    			}
				if(it == list.end()){
					if(curPic->m_pts >findPts)break;

					if(findPts - curPic->m_pts > fpsInterval * 2){
						//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 2 curPic->m_pts=%d",curPic->m_pts);
					}else{
						found = 1;
						//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 3 curPic->m_pts=%d",curPic->m_pts);
					}
					break;
				}else if(it == list.begin()){
					if(curPic->m_pts - findPts > fpsInterval * 2){
						//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 4 curPic->m_pts=%d",curPic->m_pts);
					}else if(fabs(curPic->m_pts - findPts) <= fpsInterval || curPic->m_pts > findPts){
						//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 5 curPic->m_pts=%d",curPic->m_pts);
						found = 1;
					}
					break;
				}else{
					it--;
					prevPic = (*it);
					if(fabs(curPic->m_pts - findPts)  < fpsInterval * 2 || curPic->m_pts > findPts){
						//if(findPts - prevPic->m_pts > curPic->m_pts - findPts){
						if(findPts * 2  > prevPic->m_pts + curPic->m_pts){
							//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 6 curPic->m_pts=%d",curPic->m_pts);
						}else{
							curPic = prevPic;
							//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 7 curPic->m_pts=%d",curPic->m_pts);
						}
						found = 1;
					}else{
						//VE_LOG_INFO("VEThumbnail::getOutputThumbnail step 8 curPic->m_pts=%d",curPic->m_pts);
					}
					break;
				}
            }
		}while(0);

		if(found){
			param->width = cmd->width;
			param->height = cmd->height;
			param->mp4_file = cmd->filename.c_str();
			param->index = index;
			param->rotate = info.rotate;

			
			if(curPic->m_jpeg){
				param->output_jpegs = curPic->m_jpeg;
				param->output_jpegs_len = curPic->m_jpegLength;
			}else{

				struct stat buf;
				if (stat(curPic->m_filename.c_str(), &buf) < 0){
					ret = VE_ERR_OPEN_FILE_FAILED;
					goto CacheToMem_exit;
				}
				{
					param->output_jpegs = new uint8_t[buf.st_size];
					if(param->output_jpegs)param->output_jpegs_len = buf.st_size;

				}
				if(!param->output_jpegs){
					ret = VE_ERR_MALLOC_FAILED;
					goto CacheToMem_exit;
				}



				jpegFile = fopen(curPic->m_filename.c_str(),"r");
				if(!jpegFile){
					ret = VE_ERR_OPEN_FILE_FAILED;
					goto CacheToMem_exit;
				}

				if(!fread(param->output_jpegs,1,param->output_jpegs_len,jpegFile)){
					ret = VE_ERR_OPEN_FILE_FAILED;
					goto CacheToMem_exit;
				}

				if(jpegFile){
					fclose(jpegFile);
					jpegFile = NULL;
				}
			}
		}
	}while(0);

	if(found){
#ifdef __ANDROID__
		VE_LOG_TAG_INFO(VE_MODULE_TAG,"info.rotate=%d",info.rotate);
		if(info.picture && (info.rotate == VE_ROTATE_90 || info.rotate == VE_ROTATE_270)){

			temp = param->width;
			param->width = param->height;
			param->height = temp;
			VE_LOG_TAG_INFO(VE_MODULE_TAG,"info.picture=%d,info.rotate=%d,param->width=%d,param->height=%d",info.picture,info.rotate,param->width,param->height);
		}
#endif
		return 1;
	}
	return VE_ERR_OK;

CacheToMem_exit:

	if(param->output_jpegs){
		delete [] param->output_jpegs;
		param->output_jpegs_len = 0;
	}
	if(jpegFile){
		fclose(jpegFile);
	}
	return ret;
}
int VEThumbnail::getThumbnails(ve_thumbnail_param *param){
	comn::AutoCritSec lock(m_cs);


	do{
		VEThumbnailCmd cmd;

		ve_clip_info info;

		if(!param || !param->filename || !param->count || !param->width || !param->height || !param->callback){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"!param || !param->filename || !param->count || !param->width || param->height || !param->callback");
		    return VE_ERR_INPUT_PARAM;
		}
		cmd.filename = param->filename;
		cmd.width = param->width;
		cmd.height = param->height;
		cmd.hwDecode = param->hw_decode;


		VE_LOG_TAG_INFO(VE_MODULE_TAG,"cmd.width=%d,cmd.height=%d",cmd.width,cmd.height);
		int ret = VESource::getSourceInfo(cmd.filename.c_str(),&info);

		if(ret){
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"GetVideoInfo failed");
            return VE_ERR_OPEN_FILE_FAILED;
			
		}

        int newHeight = info.height * cmd.width / info.width;
        
		if(newHeight > cmd.height){
			cmd.width =  info.width * cmd.height / info.height;
		}else{
			cmd.height = newHeight;
		}

        if(cmd.width % 2){
            cmd.width += 1;
        }
        if(cmd.height % 2){
            cmd.height += 1;
        }


		cmd.startTime = param->start_time;
		cmd.endTime = param->end_time;
		cmd.count = param->count;
		cmd.rotate = (VE_ROTATE)param->rotate;
		cmd.callback = param->callback;
		cmd.userExt = param->userExt;
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 1,width=%d,height=%d,cmd.rotate=%d,startTime=%d,endTime=%d,count=%d",cmd.width,cmd.height,cmd.rotate,cmd.startTime,cmd.endTime,cmd.count);
		if(param->path){
			if( '/' != param->path[strlen(param->path) - 1] ){
				cmd.path = param->path;
				cmd.path += "/";
			}else{
				cmd.path = param->path;
			}
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 1.1");
		}else{
			cmd.path = cmd.filename;
			cmd.path[cmd.path.rfind('/') + 1] = '\0';
		}
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 2");
		VEThumbnailWorker worker;
		worker.m_swsCtxWidth = cmd.width;
		worker.m_swsCtxHeight = cmd.height;
		worker.filename = cmd.filename;
		std::map<VEThumbnailCmd,VEThumbnailWorker>::iterator it = m_workers.find(cmd);
		VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 3");
		m_cancel = 0;
		if(it != m_workers.end()){
			VEThumbnailWorker & oldWorker = it->second;
			if(!oldWorker.workerExit){
				break;
			}else{
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 3.1,oldWorker.worker=%p",oldWorker.worker);
				if(oldWorker.worker){
					oldWorker.worker->join();
					VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 3.2");
					delete oldWorker.worker;
				}
				VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 3.3,width=%d,height=%d,startTime=%d,endTime=%d,count=%d",cmd.width,cmd.height,cmd.startTime,cmd.endTime,cmd.count);
				m_workers[cmd] = worker;

				VEThumbnailWorker & worker2 = m_workers[cmd];
				worker2.worker = new std::thread(thumbnailProcess,this,cmd,info);


			}

		}else{
			m_workers[cmd] = worker;

			VEThumbnailWorker & worker2 = m_workers[cmd];
			worker2.worker = new std::thread(thumbnailProcess,this,cmd,info);
			VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 4,width=%d,height=%d,startTime=%d,endTime=%d,count=%d",cmd.width,cmd.height,cmd.startTime,cmd.endTime,cmd.count);

		}

	}while(0);
	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"step 5");

    return VE_ERR_OK;
}
void VEThumbnail::stopWorkers(){
	m_cancel = 1;


	std::map<VEThumbnailCmd,VEThumbnailWorker>::iterator it= m_workers.begin();

	if(m_workers.size()){
		for(;it != m_workers.end();it++){

			//VEThumbnailCmd const & cmd = it->first;
			VEThumbnailWorker & worker = it->second;
			worker.workerExit = 1;
		}
	}

	std::map<VEThumbnailCmd,VEThumbnailWorker>::iterator worker_it= m_workers.begin();

	if(m_workers.size()){
		for(;worker_it != m_workers.end();worker_it++){

			//VEThumbnailCmd const & cmd = it->first;
			VEThumbnailWorker & worker = worker_it->second;
			if(worker.worker){
				worker.worker->join();
				delete worker.worker;
				worker.worker = NULL;
				VE_LOG_TAG_INFO(VE_MODULE_TAG,"join");
			}

		}
	}
	m_workers.clear();

}
void VEThumbnail::cancel(){
	comn::AutoCritSec lock(m_cs);

	stopWorkers();
}
int saveI420ImageToFile(void* yuv,int length,int width,int height,const char* outputFile){

    AVFormatContext* pFormatCtx;
    AVOutputFormat* fmt;
    AVStream* videoSt;
    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;

    uint8_t* pictureBuf;
    AVFrame* picture;
    AVPacket pkt;
    int ySize;
    int gotPicture=0;
    int size;

    int ret=0;

    int inW=width,inH=height;                           //YUV's width and height
    const char* outFile = outputFile;    //Output file

    char err[256] = {0};

    av_register_all();

    //Method 1
    /*
    pFormatCtx = avformat_alloc_context();
    //Guess format
    fmt = av_guess_format("mjpeg", NULL, NULL);
    pFormatCtx->oformat = fmt;


    //Output URL

    if ( ret = avio_open(&pFormatCtx->pb,outFile, AVIO_FLAG_READ_WRITE) < 0){
    	av_strerror(ret, err, 256);
    	VE_LOG_ERROR("VideoThumbnail::saveI420ImageToFile avio_open failed:filename=%s,,ret=%d,err info:%s",outputFile,ret,err);
        return VE_ERR_OPEN_FILE_FAILED;
    }
     */
    //Method 2. More simple

    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, outputFile);
    if (!pFormatCtx) {
        avformat_alloc_output_context2(&pFormatCtx, NULL, "mjpeg", outputFile);
    }
    fmt = pFormatCtx->oformat;


    videoSt = avformat_new_stream(pFormatCtx, 0);
    /*
    if (videoSt==NULL){
    	av_strerror(ret, err, 256);
    	VE_LOG_ERROR("VideoThumbnail::saveI420ImageToFile avformat_new_stream failed:filename=%s",outputFile);
        return VE_ERR_NEW_STREAM_FAILED;
    }
     */
    pCodecCtx = videoSt->codec;
    pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->thread_count = 1;

    pCodecCtx->width = inW;
    pCodecCtx->height = inH;

    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    //Output some information
    //av_dump_format(pFormatCtx, 0, outFile, 1);

    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec){
    	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_find_encoder failed !");
        return VE_ERR_FIND_DECODER_FAILED;
    }
    if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0){
    	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_open2 failed !");
        return VE_ERR_OPEN_DECODER_FAILED;
    }
    picture = av_frame_alloc();
    size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    pictureBuf = (uint8_t*)yuv;

    avpicture_fill((AVPicture *)picture, pictureBuf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

    //Write Header
    avformat_write_header(pFormatCtx,NULL);

    ySize = pCodecCtx->width * pCodecCtx->height;
    av_new_packet(&pkt,ySize*3);


    picture->data[0] = pictureBuf;              // Y
    picture->data[1] = pictureBuf+ ySize;      // U
    picture->data[2] = pictureBuf+ ySize*5/4;  // V

    //Encode
    ret = avcodec_encode_video2(pCodecCtx, &pkt,picture, &gotPicture);
    if(ret < 0){
    	VE_LOG_TAG_ERROR(VE_MODULE_TAG,"avcodec_encode_video2 failed !");
        return VE_ERR_ENCODE_FAILED;
    }
    if (gotPicture==1){
        pkt.stream_index = videoSt->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    av_free_packet(&pkt);
    //Write Trailer
    av_write_trailer(pFormatCtx);

    //printf("Encode Successful.\n");

    if (videoSt){
        avcodec_close(videoSt->codec);
        av_free(picture);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);


    return VE_ERR_OK;
}
