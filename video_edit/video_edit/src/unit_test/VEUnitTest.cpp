#include "ve_interface.h"
#include "VEUnitTest.h"
#include <string>
#include "VECommon.h"
#include "VESwsscale.h"
#include "VEConfig.h"
#include "VESource.h"
#include "VEMp4Writer.h"
#include "VEExport.h"
#include "VEUnitTestConfig.h"
#ifdef __APPLE__
#include "VTDecoder.h"
#endif
#include "VEQueue.h"
#include "VESemaphore.h"
#include "VELock.h"
#include "VEMp4Writer.h"
#include "VEAudioFilter.h"
#include "VEThumbnail.h"

#ifdef __ANDROID__
#include "ve_webm_wrap.h"
#endif

static int idCount = 0;

#ifdef __APPLE__

CVPixelBufferRef getPixelBuffer(int width,int height){

	CVPixelBufferRef dstPixelbuffer = NULL;
    OSType type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;

	CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);

	CVPixelBufferCreate(kCFAllocatorDefault, width,height , type, attrs, &dstPixelbuffer);

	return dstPixelbuffer;
}

#endif


void testEnableLog(){

	ve_enable_log(1);
}
int getId(){
	return idCount++;
}
void resetIdCount(){
	idCount = 0;
}
int testAddClip(){

	const char* filename = getVideoPath(0);
	const char* output_dir = getOutputDir();
	return 0;
}
int testInterfaceUitls(){

	ve_enable_log(1);

	char info[1024];

	ve_get_err_info(info,1024);

	std::string filename = getVideoPath(0);

	ve_clip_info clipInfo,*ptrClipInfo = NULL;

	VE_ERR ret = ve_get_file_info(filename.c_str(),&clipInfo);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_get_file_info(filename.c_str(),ptrClipInfo);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);
	return 0;
}
int testInterfaceTimeline(){

	HANDLE handle = ve_timeline_create();

	VE_ASSERT_NEQ(handle,NULL);

	ve_timeline_free(handle);
	handle = NULL;
	ve_timeline_free(handle);

	//ve_timeline_free done

	ve_timeline timeline,*ptrTimeline = NULL;

	handle = ve_timeline_create(&timeline);

	VE_ASSERT_EQ(handle,NULL);

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	handle = ve_timeline_create(&timeline);
	VE_ASSERT_NEQ(handle,NULL);

	//ve_timeline_create done

	VE_ERR ret = ve_timeline_reconfig(handle,ptrTimeline);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ptrTimeline = &timeline;
	ret = ve_timeline_reconfig(handle,ptrTimeline);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	//ve_timeline_reconfig done

	ptrTimeline = NULL;
	ret = ve_timeline_get_config(handle,ptrTimeline);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);


	ptrTimeline = &timeline;
	ret = ve_timeline_get_config(handle,ptrTimeline);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	//ve_timeline_get_config done

	int duration;
	ret = ve_timeline_get_duration(handle,1,&duration);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	handle = NULL;

	ret = ve_timeline_get_duration(handle,1,&duration);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ve_timeline_free(handle);

	return 0;
}
int testInterfaceTrack(){

	VE_ERR ret;

	ret = ve_track_add(NULL,NULL);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ve_timeline timeline;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	HANDLE handle = ve_timeline_create(&timeline);
	VE_ASSERT_NEQ(handle,NULL);

	ve_track track;
	track.track_id = getId();
	track.type = VE_TRACK_VIDEO;
	ret = ve_track_add(handle,&track);

	VE_ASSERT_EQ(ret,VE_ERR_OK);


	ret = ve_track_mod(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_track_mod(handle,&track);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_track_get(0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_track_get(handle,track.track_id,&track);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	int duration;
	ret = ve_track_get_overall_duration(0,0,0,&duration);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_track_get_overall_duration(handle,track.track_id,1,&duration);
	VE_ASSERT_EQ(ret,VE_ERR_OK);


	ret = ve_track_get_duration(0,0,0,0,&duration);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_track_get_duration(handle,0,track.track_id,1,&duration);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_track_get_clips_count(0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	int count;
	ret = ve_track_get_clips_count(handle,track.track_id,&count);
	VE_ASSERT_EQ(ret,VE_ERR_OK);


	ret = ve_track_get_clips(0,0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ve_clip clips[2];
	ret = ve_track_get_clips(handle,track.track_id,clips,count);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_track_del(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_track_del(handle,track.track_id);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_timeline_free(handle);
	return 0;
}
int testInterfaceClip(){

	VE_ERR ret;

	ve_timeline timeline;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	HANDLE handle = ve_timeline_create(&timeline);
	VE_ASSERT_NEQ(handle,NULL);

	ve_track track;
	track.track_id = getId();
	track.type = VE_TRACK_VIDEO;
	ret = ve_track_add(handle,&track);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_insert(0,0,0);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	std::string filename = getVideoPath(0);
	ve_clip clip;
    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = ve_clip_insert(handle,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_mod(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_clip_mod(handle,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_move(0,0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_clip_move(handle,clip.track_id,0,0);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_get(0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_clip_get(handle,clip.clip_id,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_get_by_index(0,0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_clip_get_by_index(handle,clip.track_id,0,&clip);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_get_duration(0,0,0);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	int duration;
	ret = ve_clip_get_duration(handle,clip.clip_id,&duration);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_del(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_clip_del(handle,clip.clip_id);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 3000;
    clip.duration = 3000;
    clip.filename = filename.c_str();
	ret = ve_clip_insert(handle,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_clip_del_by_index(0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_clip_del_by_index(handle,clip.track_id,0);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_timeline_free(handle);

	return 0;
}

int testInterfaceTransition(){

	VE_ERR ret;

	ve_timeline timeline;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	HANDLE handle = ve_timeline_create(&timeline);
	VE_ASSERT_NEQ(handle,NULL);

	ve_track track;
	track.track_id = getId();
	track.type = VE_TRACK_VIDEO;
	ret = ve_track_add(handle,&track);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	std::string filename = getVideoPath(0);
	ve_clip clip;
    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = ve_clip_insert(handle,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	clip.clip_id = getId();
	ret = ve_clip_insert(handle,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);


	ret = ve_transition_add(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ve_transition transition;
	transition.transition_id = getId();
	transition.track_id = track.track_id;
	transition.action = "transition filter";
	transition.clip_index_b = 1;
	transition.duration = 1000;
	ret = ve_transition_add(handle,&transition);
	VE_ASSERT_EQ(ret,VE_ERR_OK);


	ret = ve_transition_mod(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_transition_mod(handle,&transition);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_transition_get(0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_transition_get(handle,transition.transition_id,&transition);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_transition_del(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_transition_del(handle,transition.transition_id);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_timeline_free(handle);

	return 0;
}
int testInterfaceFilter(){

	VE_ERR ret;

	ve_timeline timeline;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	HANDLE handle = ve_timeline_create(&timeline);
	VE_ASSERT_NEQ(handle,NULL);

	ve_track track;
	track.track_id = getId();
	track.type = VE_TRACK_VIDEO;
	ret = ve_track_add(handle,&track);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	std::string filename = getVideoPath(0);
	ve_clip clip;
    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = ve_clip_insert(handle,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);


	ve_filter filter;
	filter.track_id = track.track_id;
	filter.clip_id = clip.clip_id;
	filter.filter_id = getId();
	filter.start_time = 0;
	filter.end_time = 3000;
	filter.action = "clip filter";

	ret = ve_filter_add(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);


	ret = ve_filter_add(handle,&filter);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_filter_mod(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_filter_mod(handle,&filter);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_filter_get(0,0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_filter_get(handle,filter.filter_id,&filter);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ret = ve_filter_del(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ret = ve_filter_del(handle,filter.filter_id);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_timeline_free(handle);

	return 0;
}
void testExportVideoFilterCallback(HANDLE client_handle,ve_filter_callback_param* param,void* userExt){

}
void testExportStatusCallback(ve_export_status* status){

}
int testInterfaceExport(){

	VE_ERR ret;

	ve_timeline timeline;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	HANDLE handle = ve_timeline_create(&timeline);
	VE_ASSERT_NEQ(handle,NULL);

	ve_track track;
	track.track_id = getId();
	track.type = VE_TRACK_VIDEO;
	ret = ve_track_add(handle,&track);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	std::string filename = getVideoPath(0);
	ve_clip clip;
    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = ve_clip_insert(handle,&clip);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_export_param exportParam;
	HANDLE exportHandle = ve_export_create(handle,testExportVideoFilterCallback,testExportStatusCallback,exportParam);
	VE_ASSERT_NEQ(0,exportHandle);


	ve_export_start(0);
	ve_export_start(exportHandle);

	ve_export_send_filtered_data_back(0,0);

	ve_filtered_data filteredData;
	ve_export_send_filtered_data_back(exportHandle,&filteredData);

	ve_export_cancel(0);
	ve_export_cancel(exportHandle);

	ve_export_free(exportHandle);

	return 0;
}
void testThumbnailCallback(HANDLE handle,ve_thumbnail_callback_param* param,int status,void* userExt){

}
int testVEThumbnailExit = 0;
void testThumbnailCallback2(HANDLE handle,ve_thumbnail_callback_param* param,int status,void* userExt){
	switch(status){
	case VE_THUMBNAIL_ERR:
	case VE_THUMBNAIL_END:
	case VE_THUMBNAIL_CANCEL:
		testVEThumbnailExit = 1;
		break;
	default:
		break;
	}

}
int testInterfaceThumbnail(){
	HANDLE handle = ve_thumbnail_new();

	int ret = ve_thumbnail_get(0,0);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	ve_thumbnail_param thumbnailParam;
	std::string filename = getVideoPath(0);
	std::string outputDir = getOutputDir();

	thumbnailParam.filename = filename.c_str();
	thumbnailParam.width = 100;
	thumbnailParam.height = 100;
	thumbnailParam.start_time = 0;
	thumbnailParam.end_time = 1000;
	thumbnailParam.count = 2;
	thumbnailParam.path = outputDir.c_str();
	thumbnailParam.callback = testThumbnailCallback;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(0);
	ve_thumbnail_cancel(handle);
	ve_thumbnail_free(handle);
	return 0;
}


int testInterfaceWebm(){

	HANDLE handle = ve_webm_new(0);

	VE_ASSERT_EQ(handle,0);

	ve_webm_config config;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.webm";

	config.output_filename = outputFilename.c_str();
	config.output_fps = 30;
	config.frame_format = VE_WEBM_COLOR_RGBA;

	handle = ve_webm_new(&config);

	VE_ASSERT_NEQ(handle,0);

	VE_WEBM_ERR ret = ve_webm_start(0);

	VE_ASSERT_NEQ(ret,0);

	ret = ve_webm_start(handle);

	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);

	ve_webm_frame frame;
	AVFrame* avFrame = av_frame_alloc();
	avFrame->width = config.output_width;
	avFrame->height = config.output_height;
	avFrame->format = AV_PIX_FMT_RGBA;
    av_frame_get_buffer(avFrame, 16);

    frame.data[0] = avFrame->data[0];

    frame.linesize[0] = avFrame->linesize[0];

    frame.width = config.output_width;
    frame.height = config.output_height;
    frame.pts = 0;


    ret = ve_webm_send_frame(0,0);

    VE_ASSERT_NEQ(ret,VE_WEBM_ERR_OK);

    int i = 0;
    for(;i< 5;i++){

    	ret = ve_webm_send_frame(handle,&frame);

    	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);
    	frame.pts += 30;
    	av_usleep(30000);
    }


	ret = ve_webm_stop(0);
	VE_ASSERT_NEQ(ret,VE_WEBM_ERR_OK);

	ret = ve_webm_stop(handle);
	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);

	ve_webm_free(handle);

	av_frame_free(&avFrame);

	return 0;
}

int testInterfaceWebm2(){


	HANDLE handle = ve_webm_new(0);

	VE_ASSERT_EQ(handle,0);

	ve_webm_config config;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.webm";

	config.output_filename = outputFilename.c_str();
	config.output_fps = 30;
	config.frame_format = VE_WEBM_COLOR_ARGB;

	handle = ve_webm_new(&config);

	VE_ASSERT_NEQ(handle,0);

	VE_WEBM_ERR ret = ve_webm_start(0);

	VE_ASSERT_NEQ(ret,0);

	ret = ve_webm_start(handle);

	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);

	ve_webm_frame frame;
	AVFrame* avFrame = av_frame_alloc();
	avFrame->width = config.output_width;
	avFrame->height = config.output_height;
	avFrame->format = VE_WEBM_COLOR_ARGB;
    av_frame_get_buffer(avFrame, 16);

    frame.data[0] = avFrame->data[0];

    frame.linesize[0] = avFrame->linesize[0];

    frame.width = config.output_width;
    frame.height = config.output_height;
    frame.pts = 0;


    ret = ve_webm_send_frame(0,0);

    VE_ASSERT_NEQ(ret,VE_WEBM_ERR_OK);
	ret = ve_webm_send_frame(handle,&frame);

	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);

	ret = ve_webm_stop(0);
	VE_ASSERT_NEQ(ret,VE_WEBM_ERR_OK);

	ret = ve_webm_stop(handle);
	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);

	ve_webm_free(handle);

	av_frame_free(&avFrame);

	return 0;
}

#ifdef __ANDROID__
int testInterfaceWebm3(){

	ve_webm_config other_config;
	ve_webm_config * config = (ve_webm_config *)malloc(sizeof(ve_webm_config));
	*config = other_config;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.webm";
	char* output_filename = (char*)malloc(outputFilename.size() + 1);

	strcpy(output_filename,outputFilename.c_str());
	config->output_filename = output_filename;
	config->output_fps = 30;
	config->frame_format = VE_WEBM_COLOR_ARGB;



	HANDLE handle = ve_webm_wrap_new(config);


	VE_ASSERT_NEQ(handle,0);


	int ret = ve_webm_wrap_start(handle);

	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);

	ve_webm_frame frame;
	AVFrame* avFrame = av_frame_alloc();
	avFrame->width = config->output_width;
	avFrame->height = config->output_height;
	avFrame->format = VE_WEBM_COLOR_ARGB;
    av_frame_get_buffer(avFrame, 16);

    frame.data[0] = avFrame->data[0];

    frame.linesize[0] = avFrame->linesize[0];

    frame.width = config->output_width;
    frame.height = config->output_height;
    frame.pts = 0;

    ve_webm_frame * framePtr = ve_webm_wrap_get_frame_cache(handle,frame.linesize,1,frame.width,frame.height,frame.pts);

	ret = ve_webm_wrap_send_frame(handle,&frame);



	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);


    framePtr = ve_webm_wrap_get_frame_cache(handle,frame.linesize,1,frame.width,frame.height,frame.pts);

	ret = ve_webm_wrap_send_frame(handle,&frame);


	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);


	ret = ve_webm_wrap_stop(handle);
	VE_ASSERT_EQ(ret,VE_WEBM_ERR_OK);

	ve_webm_wrap_free(handle);

	av_frame_free(&avFrame);

	return 0;
}
#endif
int testVEUtils(){
	va_list vl;
	veLogCallbackFfmpeg(0,0,"",vl);
	return 0;
}
int testVESwsscale(){

	int dstWidth,dstHeight;
	VESwsscale::getOpenGLWH(100,102,&dstWidth,&dstHeight);
    
    VE_ASSERT_EQ(104,dstWidth);
    VE_ASSERT_EQ(102,dstHeight);
    
	VESwsscale::getOpenGLWH(102,100,&dstWidth,&dstHeight);
    
    VE_ASSERT_EQ(104,dstWidth);
    VE_ASSERT_EQ(100,dstHeight);

	VESwsscale::getOpenGLWH(4100,1081,&dstWidth,&dstHeight);
    
    VE_ASSERT_EQ(4096,dstWidth);
    VE_ASSERT_EQ(1080,dstHeight);
	VESwsscale::getOpenGLWH(1080,4100,&dstWidth,&dstHeight);
    
    VE_ASSERT_EQ(1080,dstWidth);
    VE_ASSERT_EQ(4096,dstHeight);


	AVFrame* avFrame = av_frame_alloc(),*dstFrame = NULL;
	avFrame->width = 1920;
	avFrame->height = 1080;
	avFrame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(avFrame, 16);


    VESwsscale* swsScale = new VESwsscale();
    VE_ASSERT_NEQ(0,swsScale);


    swsScale->process(avFrame,&dstFrame,AV_PIX_FMT_YUV420P,1080,720);

    delete swsScale;

	return 0;
}
int testVEBitstream(){

	VEBitstream* bitstream = new VEBitstream("aac_adtstoasc");
	VE_ASSERT_NEQ(0,bitstream);
	delete bitstream;

	bitstream = new VEBitstream("hevc_mp4toannexb");

	VE_ASSERT_NEQ(0,bitstream);
	delete bitstream;


	bitstream = new VEBitstream("h264_mp4toannexb");

	VE_ASSERT_NEQ(0,bitstream);

	delete bitstream;

	bitstream = new VEBitstream("xxx");

	bitstream->applyBitstreamFilter(0,0);

	VE_ASSERT_NEQ(0,bitstream);
	delete bitstream;

	return 0;
}

int testVEConfigBase(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config1,config2(&timeline);


	config1.reconfig(&timeline);
	config1.getConfig(&timeline);
	VEConfig::enableVELog(0);
	VEConfig::enableVELog(1);

	int duration;
	ret = config2.getTimelineDuration(1,&duration);

	VE_ASSERT_EQ(0,ret);

	ve_track track;
	track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config2.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);
	ve_clip clip;
    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config2.insertClip(&clip,0);
	VE_ASSERT_EQ(0,ret);
    clip.clip_id = getId();
	ret = config2.insertClip(&clip,1);

	VE_ASSERT_EQ(0,ret);
	ret = config2.getTimelineDuration(1,&duration);

	VE_ASSERT_EQ(0,ret);

	ve_transition transition;
	transition.transition_id = getId();
	transition.track_id = track.track_id;
	transition.action = "transition filter";
	transition.clip_index_b = 1;
	transition.duration = 1000;

	ret = config2.addTransition(&transition);

	VE_ASSERT_EQ(0,ret);

	config1 = config2;
	return 0;
}
int testVEConfigTrack(){


	ve_timeline timeline;
	VE_ERR ret;

	resetIdCount();

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	ve_track track;
	track.track_id = -1;
	track.type = VE_TRACK_VIDEO;


	ret = config.getTrack(0,&track);
	VE_ASSERT_NEQ(0,ret);
	ret = config.addTrack(&track);
	VE_ASSERT_NEQ(0,ret);
	ret = config.modTrack(&track);
	VE_ASSERT_NEQ(0,ret);

	int i;
	for(i=0;i<VE_MAX_TRACK_NUM;i++){
		track.track_id = getId();
		ret = config.addTrack(&track);
		VE_ASSERT_EQ(0,ret);
        
        ret = config.modTrack(&track);
        VE_ASSERT_EQ(0,ret);

		ret = config.getTrack(track.track_id,&track);
		VE_ASSERT_EQ(0,ret);
	}

	track.track_id = 1000;
	ret = config.addTrack(&track);
	VE_ASSERT_NEQ(0,ret);

    ret = config.delTrack(5);
    VE_ASSERT_EQ(0,ret);
    
    track.track_id = 6;
    track.type = VE_TRACK_AUDIO;
	ret = config.modTrack(&track);
	VE_ASSERT_NEQ(0,ret);

    ret = config.delTrack(1000);
    VE_ASSERT_NEQ(0,ret);




	return 0;
}

int testVEConfigInsertClip1(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);
	ve_clip clip;
	clip.clip_id = -1;

	ret = config.insertClip(&clip,0);

	VE_ASSERT_NEQ(0,ret);

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.insertClip(&clip,0);

	VE_ASSERT_NEQ(0,ret);

	clip.clip_id = getId();

	ret = config.insertClip(&clip,-1);

	VE_ASSERT_EQ(0,ret);
	track.track_id = getId();
	track.clip_arrangement = VE_CLIP_ARRANGEMENT_OVERLAY;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	clip.track_id = track.track_id;
	clip.clip_id = getId();

	ret = config.insertClip(&clip,100);
	VE_ASSERT_NEQ(0,ret);

    clip.insert_time = 0;
	ret = config.insertClip(&clip,-1);
	VE_ASSERT_EQ(0,ret);

	clip.clip_id = getId();
	ret = config.insertClip(&clip,-1);
	VE_ASSERT_NEQ(0,ret);

	clip.track_id = sequnceTrackId;
	clip.filename = NULL;
	ret = config.insertClip(&clip,100);
	VE_ASSERT_NEQ(0,ret);

	clip.clip_id = getId();
	clip.filename = filename.c_str();
	clip.slv.active = 1;
	clip.slv.len = 1;
	clip.slv.start_time[0] = 0;
	clip.slv.end_time[0] = 3000;
	clip.slv.speed[0] = 1;

	ret = config.insertClip(&clip,100);
	VE_ASSERT_EQ(0,ret);


	audioTrack.track_id = getId();
	audioTrack.type = VE_TRACK_AUDIO;

	ret = config.addTrack(&audioTrack);
	VE_ASSERT_EQ(0,ret);

	clip.clip_id = getId();
    clip.track_id = audioTrack.track_id;
	ret = config.insertClip(&clip,100);
	VE_ASSERT_NEQ(0,ret);

	//trackData->m_track.type == VE_TRACK_AUDIO && (!clipInfo.a_codec_id || clip->type != VE_CLIP_AUDIO
	return 0;
}
int testVEConfigInsertClip2(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);
	ve_clip clip;

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.type = VE_CLIP_AUDIO;
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);
	VE_ASSERT_NEQ(0,ret);

	clip.type = VE_CLIP_PICTURE;
	clip.duration = 0;

	ret = config.insertClip(&clip,0);
	VE_ASSERT_NEQ(0,ret);

	clip.type = VE_CLIP_VIDEO;
	clip.duration = 3000;
	clip.start_time = -1;

	ret = config.insertClip(&clip,0);
	VE_ASSERT_NEQ(0,ret);

	clip.start_time = 0;
	clip.volume = 300;

	ret = config.insertClip(&clip,0);
	VE_ASSERT_NEQ(0,ret);

	clip.volume = 100;
	clip.speed = 65;

	ret = config.insertClip(&clip,0);
	VE_ASSERT_NEQ(0,ret);

	clip.speed = 1;
	clip.slv.active = 1;
	clip.slv.len = 1;
	clip.slv.start_time[0] = 1;
	clip.slv.end_time[0] = 3000;
	clip.slv.speed[0] = 1;

	ret = config.insertClip(&clip,0);
	VE_ASSERT_NEQ(0,ret);

	clip.slv.start_time[0] = 0;
	clip.duration = 2000;

	ret = config.insertClip(&clip,0);
	VE_ASSERT_EQ(0,ret);


    clip.clip_id = getId();
	clip.slv.active = 0;
	clip.duration = 3000;
	clip.type = VE_CLIP_PICTURE;
	clip.picture_rotate = VE_ROTATE_90;

	filename = getPicPath(0);
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);
	VE_ASSERT_EQ(0,ret);

	return 0;
}
int testVEConfigModClip1(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int clipId;
	ve_clip clip,clip2;
	ret = config.modClip(&clip);

	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);


    clip.track_id = track.track_id;
    clipId = clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);


	clip2.clip_id = 0;
	ret = config.modClip(&clip2);

	VE_ASSERT_NEQ(0,ret);

	clip.clip_id = clipId;
	ret = config.modClip(&clip);

	VE_ASSERT_EQ(0,ret);

	track.track_id = getId();
	track.clip_arrangement = VE_CLIP_ARRANGEMENT_OVERLAY;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.insert_time = 0;
	ret = config.insertClip(&clip,-1);
	VE_ASSERT_EQ(0,ret);

	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.insert_time = 3000;
	ret = config.insertClip(&clip,-1);
	VE_ASSERT_EQ(0,ret);

	clip.insert_time = 0;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);


	clip.clip_id = clipId;
	clip.track_id = sequnceTrackId;
	clip.filename = NULL;

	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.filename = filename.c_str();
	ret = config.modClip(&clip);
	VE_ASSERT_EQ(0,ret);

	filename = getVideoPath(1);
	clip.filename = filename.c_str();
	ret = config.modClip(&clip);
	VE_ASSERT_EQ(0,ret);


	clip.slv.active = 1;
	clip.slv.len = 1;
	clip.slv.start_time[0] = 0;
	clip.slv.end_time[0] = 3000;
	clip.slv.speed[0] = 1;
	clip.filename = filename.c_str();
	ret = config.modClip(&clip);
	VE_ASSERT_EQ(0,ret);

	return 0;
}

int testVEConfigModClip2(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int clipId;
	ve_clip clip,clip2;

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);
	std::string musicFilename = getMusicPath(0);

    clip.track_id = track.track_id;
    clipId = clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	audioTrack.track_id = getId();
	audioTrack.type = VE_TRACK_AUDIO;

	ret = config.addTrack(&audioTrack);
	VE_ASSERT_EQ(0,ret);

	clip2 = clip;
	clip2.track_id = audioTrack.track_id;
	clip2.clip_id = getId();
	clip2.type = VE_CLIP_AUDIO;
	clip2.filename = musicFilename.c_str();

	ret = config.insertClip(&clip2,0);
	VE_ASSERT_EQ(0,ret);

	clip.type = VE_CLIP_AUDIO;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.type = VE_CLIP_PICTURE;
	clip.duration = 0;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.type = VE_CLIP_PICTURE;
	clip.duration = 3000;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.type = VE_CLIP_VIDEO;
	clip.start_time = -1;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.start_time = 0;
	clip.volume = 300;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);


	clip.volume = 100;
	clip.speed = 65;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.speed = 1;
	clip.slv.active = 1;
	clip.slv.start_time[0] = 1;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.slv.active = 0;
	clip.duration = 2000;
	ret = config.modClip(&clip);
	VE_ASSERT_NEQ(0,ret);

	clip.duration = 4000;
	ret = config.modClip(&clip);
	VE_ASSERT_EQ(0,ret);


	clip.slv.active = 1;
	clip.slv.len = 1;
	clip.slv.start_time[0] = 0;
	clip.slv.end_time[0] = 3000;
	clip.slv.speed[0] = 1;
	ret = config.modClip(&clip);
	VE_ASSERT_EQ(0,ret);

	return 0;
}

int testVEConfigMoveClip(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int clipId;
	ve_clip clip,clip2;

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.moveClip(0,0,0);
	VE_ASSERT_NEQ(0,ret);

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);

    clip.track_id = track.track_id;
    clipId = clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.moveClip(track.track_id,0,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.moveClip(track.track_id,1,1);

	VE_ASSERT_EQ(0,ret);

	return 0;
}
int testVEConfigDelClip1(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);
	ve_clip clip;
    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.delClip(100);
	VE_ASSERT_NEQ(0,ret);

	ret = config.delClip(clip.clip_id);
	VE_ASSERT_EQ(0,ret);

	return 0;
}
int testVEConfigDelClip2(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	ret = config.delClip(0,0);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ret = config.delClip(track.track_id,-1);
	VE_ASSERT_NEQ(0,ret);

	std::string filename = getVideoPath(0);
	ve_clip clip;
    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.delClip(track.track_id,0);
	VE_ASSERT_EQ(0,ret);

	return 0;
}
int testVEConfigGetClip(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	ret = config.getClip(0,0,0);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;
	ret = config.getClip(track.track_id,-1,&clip2);
	VE_ASSERT_NEQ(0,ret);

	std::string filename = getVideoPath(0);

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.getClip(clip.track_id,0,&clip2);
	VE_ASSERT_EQ(0,ret);

	ret = config.getClip(clip.track_id,0,&clip2);
	VE_ASSERT_EQ(0,ret);

	ret = config.getClip(clip.clip_id,&clip2);
	VE_ASSERT_EQ(0,ret);

	return 0;
}
int testVEConfigGetClipDuration(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);


	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	int duration;

	ret = config.getClipDuration(clip.clip_id,&duration);

	VE_ASSERT_EQ(0,ret);
	return 0;
}
int testVEConfigGetClipsCount(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int count;

	ret = config.getClipsCount(0,&count);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.getClipsCount(clip.track_id,&count);
	VE_ASSERT_EQ(0,ret);
	return 0l;
}
int testVEConfigGetClips(){


	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int count;

	ret = config.getClips(0,0,0);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	ret = config.getClips(clip.track_id,0,2);
	VE_ASSERT_NEQ(0,ret);

	ret = config.getClips(clip.track_id,&clip2,1);
	VE_ASSERT_EQ(0,ret);
	return 0;
}
int testVEConfigGetTrackDuration(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int duration;

	ret = config.getTrackDuration(0,0,0,&duration);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	
	ret = config.getTrackDuration(clip.track_id,0,1,&duration);
	VE_ASSERT_EQ(0,ret);

	ret = config.getTrackDuration(clip.track_id,3,1,&duration);
	VE_ASSERT_EQ(0,ret);

	track.track_id = getId();
	track.clip_arrangement = VE_CLIP_ARRANGEMENT_OVERLAY;
	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
    clip.insert_time = 0;

	ret = config.insertClip(&clip,-1);

	VE_ASSERT_EQ(0,ret);

	ret = config.getTrackDuration(clip.track_id,3,1,&duration);
	VE_ASSERT_EQ(0,ret);

	ret = config.getTrackDuration(clip.track_id,1,&duration);
	VE_ASSERT_EQ(0,ret);
	return 0;
}
int testVEConfigAddTransition(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int count;
	ve_transition transition;
	ret = config.addTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	transition.track_id = track.track_id;
	transition.clip_index_b = 2;
	ret = config.addTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	clip.clip_id = getId();
	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

    transition.clip_index_b = 1;
	transition.transition_id = -1;
	ret = config.addTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	transition.transition_id = getId();
	transition.action = NULL;
	ret = config.addTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	transition.action = "transition filter";
	transition.clip_index_b = 0;
	ret = config.addTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	transition.clip_index_b = 1;
	transition.duration = 5000;
	ret = config.addTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	transition.duration = 1000;
	ret = config.addTransition(&transition);
	VE_ASSERT_EQ(0,ret);

	transition.duration = 1000;
	ret = config.addTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	return 0;
}
int testVEConfigModTransition(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int count;
	ve_transition transition,transition2;
	ret = config.modTransition(&transition);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	clip.clip_id = getId();
	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	transition.track_id = track.track_id;
	transition.clip_index_b = 1;
	transition.action = "transition filter";
	transition.transition_id = getId();
	transition.duration = 1000;
	ret = config.addTransition(&transition);
	VE_ASSERT_EQ(0,ret);

	transition2.transition_id = -1;
	ret = config.modTransition(&transition2);
	VE_ASSERT_NEQ(0,ret);

    transition2.track_id = transition.track_id;
	transition2.transition_id = transition.transition_id;
	transition2.clip_index_b = 2;
	ret = config.modTransition(&transition2);
	VE_ASSERT_NEQ(0,ret);


	transition2.clip_index_b = 1;
	transition2.action = NULL;
	ret = config.modTransition(&transition2);
	VE_ASSERT_NEQ(0,ret);

	transition2.clip_index_b = 0;
	transition2.action = "transition filter";
	ret = config.modTransition(&transition2);
	VE_ASSERT_NEQ(0,ret);


	transition2.clip_index_b = 1;
	transition2.duration = 5000;
	ret = config.modTransition(&transition2);
	VE_ASSERT_NEQ(0,ret);


	ret = config.modTransition(&transition);
	VE_ASSERT_EQ(0,ret);

	return 0;
}
int testVEConfigGetTransition(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int count;
	ve_transition transition;


	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);


	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	clip.clip_id = getId();
	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	transition.track_id = track.track_id;
	transition.transition_id = getId();
	transition.clip_index_b = 1;
	transition.duration = 1000;
	transition.action = "";
	ret = config.addTransition(&transition);
	VE_ASSERT_EQ(0,ret);

	ret = config.getTransition(transition.transition_id,&transition);
	VE_ASSERT_EQ(0,ret);

	ret = config.getTransition(0,&transition);
	VE_ASSERT_NEQ(0,ret);

	return 0;
}

int testVEConfigDelTransition(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int count;
	ve_transition transition;



	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	ret = config.delTransition(transition.transition_id);

	VE_ASSERT_NEQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	clip.clip_id = getId();
	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	transition.track_id = track.track_id;
	transition.transition_id = getId();
	transition.clip_index_b = 1;
	transition.duration = 1000;
	transition.action = "";
	ret = config.addTransition(&transition);
	VE_ASSERT_EQ(0,ret);

	ret = config.delTransition(transition.transition_id);

	VE_ASSERT_EQ(0,ret);

	ret = config.delTransition(0);
	VE_ASSERT_NEQ(0,ret);
	return 0;
}
int testVEConfigAddFilter(){


	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	ve_filter filter;
	filter.filter_id = -1;

	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);
	filter.filter_id = getId();
	filter.start_time = 1;
	filter.end_time = 0;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.start_time = 0;
	filter.end_time = 3000;
	filter.loc_type = VE_FILTER_LOC_TIMELINE;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.filter_id = getId();
	filter.loc_type = VE_FILTER_LOC_TRACK;

	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	filter.track_id = track.track_id;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.filter_id = getId();
	filter.type = VE_FILTER_AUDIO;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	audioTrack.track_id = getId();
	audioTrack.type = VE_TRACK_AUDIO;
	ret = config.addTrack(&audioTrack);
	VE_ASSERT_EQ(0,ret);


    filter.track_id = audioTrack.track_id;
	filter.type = VE_FILTER_VIDEO;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	ret = config.delTrack(track.track_id);
	VE_ASSERT_EQ(0,ret);

	ret = config.delTrack(audioTrack.track_id);
	VE_ASSERT_EQ(0,ret);

	filter.loc_type = VE_FILTER_LOC_CLIP;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	track.track_id = getId();
	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	filter.clip_id = -1;
	filter.clip_index = -1;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.clip_index = 1;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.clip_index = 0;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.clip_id = 0;
	filter.clip_index = -1;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

    filter.track_id = track.track_id;
	filter.clip_id = clip.clip_id;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);


	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.clip_id = -1;
	filter.clip_index = 0;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.filter_id = getId();
	filter.type = VE_FILTER_AUDIO;
	filter.af_type = VE_AUDIO_FILTER_FADE_IN;
	filter.gain_min = -1;
	ret = config.addFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	return 0;
}
int testVEConfigModFilter(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	ve_filter filter;
	filter.filter_id = -1;

	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.filter_id = getId();

	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.start_time = 0;
	filter.end_time = 3000;
	filter.loc_type = VE_FILTER_LOC_TIMELINE;

	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.modFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	filter.loc_type = VE_FILTER_LOC_TRACK;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	filter.track_id = track.track_id;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.filter_id = getId();
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.modFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	filter.type = VE_FILTER_AUDIO;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.type = VE_FILTER_VIDEO;
	audioTrack.track_id = getId();
	audioTrack.type = VE_TRACK_AUDIO;

	ret = config.addTrack(&audioTrack);
	VE_ASSERT_EQ(0,ret);

	filter.track_id = audioTrack.track_id;
	filter.loc_type = VE_FILTER_LOC_CLIP;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.track_id = track.track_id;

	ret = config.delTrack(track.track_id);
	VE_ASSERT_EQ(0,ret);

	ret = config.delTrack(audioTrack.track_id);
	VE_ASSERT_EQ(0,ret);

	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	track.track_id = getId();
	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	filter.filter_id = -1;
	filter.clip_index = -1;
	filter.track_id = track.track_id;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);


	filter.clip_index = 0;
	filter.track_id = track.track_id;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	filter.filter_id = 0;
	filter.clip_index = -1;
	filter.track_id = track.track_id;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);


	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	filter.filter_id = getId();
	filter.clip_id = clip.clip_id;
	filter.clip_index = -1;

	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);


	ret = config.modFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	filter.clip_id = -1;
	filter.clip_index = 0;
	ret = config.modFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	filter.type = VE_FILTER_AUDIO;
	filter.af_type = VE_AUDIO_FILTER_FADE_IN;
	filter.gain_min = -1;
	ret = config.modFilter(&filter);
	VE_ASSERT_NEQ(0,ret);

	return 0;
}
int testVEConfigGetFilter(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	ve_filter filter;

	filter.filter_id = getId();
	filter.start_time = 0;
	filter.end_time = 3000;
	filter.loc_type = VE_FILTER_LOC_TIMELINE;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.getFilter(filter.filter_id,&filter);
	VE_ASSERT_EQ(0,ret);


	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	filter.filter_id = getId();
	filter.track_id = track.track_id;
	filter.loc_type = VE_FILTER_LOC_TRACK;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.getFilter(filter.filter_id,&filter);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	filter.filter_id = getId();
	filter.clip_id = clip.clip_id;
	filter.clip_index = -1;
	filter.loc_type = VE_FILTER_LOC_CLIP;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.getFilter(filter.filter_id,&filter);
	VE_ASSERT_EQ(0,ret);


	return 0;
}

int testVEConfigDelFilter(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	ve_filter filter;

	filter.filter_id = getId();
	filter.start_time = 0;
	filter.end_time = 3000;
	filter.loc_type = VE_FILTER_LOC_TIMELINE;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.delFilter(filter.filter_id);
	VE_ASSERT_EQ(0,ret);


	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	filter.filter_id = getId();
	filter.track_id = track.track_id;
	filter.loc_type = VE_FILTER_LOC_TRACK;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.delFilter(filter.filter_id);
	VE_ASSERT_EQ(0,ret);

	ve_clip clip,clip2;

	std::string filename = getVideoPath(0);

	clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 3000;
	clip.duration = 3000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);

	filter.filter_id = getId();
	filter.clip_id = clip.clip_id;
	filter.clip_index = -1;
	filter.loc_type = VE_FILTER_LOC_CLIP;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	ret = config.delFilter(filter.filter_id);
	VE_ASSERT_EQ(0,ret);


	return 0;
}

int testVEThumbnail(){

	HANDLE handle = ve_thumbnail_new();

	int ret;

	testVEThumbnailExit = 0;
	ve_thumbnail_param thumbnailParam;
	std::string filename = getVideoPath(0);
	std::string hevcFilename = getHevcVideoPath(0);
	std::string webmFilename = getWebmVideoPath(0);
	std::string outputDir = getOutputDir();


	thumbnailParam.width = 100;
	thumbnailParam.height = 100;
	thumbnailParam.start_time = 0;
	thumbnailParam.end_time = 1000;
	thumbnailParam.count = 0;
	thumbnailParam.path = outputDir.c_str();
	thumbnailParam.callback = testThumbnailCallback2;

	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);
	thumbnailParam.filename = "/ios/file";
	thumbnailParam.count = 2;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);
	thumbnailParam.filename = filename.c_str();

	thumbnailParam.height = 0;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);


	thumbnailParam.height = 100;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);


	thumbnailParam.path = outputDir.c_str();
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);

	thumbnailParam.path = NULL;
	thumbnailParam.height = 100;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);


	thumbnailParam.height = 200;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);

	thumbnailParam.width = 200;
	thumbnailParam.height = 100;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);

	thumbnailParam.width = 101;
	thumbnailParam.height = 101;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);

	thumbnailParam.path = outputDir.c_str();
	thumbnailParam.width = 100;
	thumbnailParam.height = 100;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);

	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);

	ve_thumbnail_cancel(handle);
    thumbnailParam.end_time = 4500;
	thumbnailParam.count = 3;
	testVEThumbnailExit = 0;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);
	while(1){
		av_usleep(20000);
		if(testVEThumbnailExit)break;
	}
	ve_thumbnail_cancel(handle);


	testVEThumbnailExit = 0;
	thumbnailParam.filename = hevcFilename.c_str();
	thumbnailParam.end_time = 1000;
	thumbnailParam.count = 2;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);
	while(1){
		av_usleep(20000);
		if(testVEThumbnailExit)break;
	}
	ve_thumbnail_cancel(handle);


	testVEThumbnailExit = 0;
	thumbnailParam.filename = webmFilename.c_str();
	thumbnailParam.end_time = 1000;
	thumbnailParam.count = 2;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);
	while(1){
		av_usleep(20000);
		if(testVEThumbnailExit)break;
	}
	ve_thumbnail_cancel(handle);


	ve_thumbnail_free(handle);

	return 0;
}


int testVEThumbnail2(){

	HANDLE handle = ve_thumbnail_new();

	int ret;

	testVEThumbnailExit = 0;
	ve_thumbnail_param thumbnailParam;
	std::string filename = getVideoPath(0);
	std::string hevcFilename = getHevcVideoPath(0);
	std::string webmFilename = getWebmVideoPath(0);
	std::string outputDir = getOutputDir();


	thumbnailParam.width = 100;
	thumbnailParam.height = 100;
	thumbnailParam.start_time = 0;
	thumbnailParam.end_time = 1000;
	thumbnailParam.count = 40;
	thumbnailParam.path = outputDir.c_str();
	thumbnailParam.callback = testThumbnailCallback2;
	thumbnailParam.filename = filename.c_str();

	thumbnailParam.start_time = -1;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	ve_thumbnail_cancel(handle);

	thumbnailParam.start_time = 0;
	testVEThumbnailExit = 0;

	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);
	while(1){
		av_usleep(20000);
		if(testVEThumbnailExit)break;
	}
	ve_thumbnail_cancel(handle);


	testVEThumbnailExit = 0;

	ret = ve_thumbnail_get(handle,&thumbnailParam);
	thumbnailParam.count = 39;
	ret = ve_thumbnail_get(handle,&thumbnailParam);
	VE_ASSERT_EQ(ret,VE_ERR_OK);
	while(1){
		av_usleep(20000);
		if(testVEThumbnailExit)break;
	}
	ve_thumbnail_cancel(handle);


	ve_thumbnail_free(handle);

	return 0;
}


int testVESourceOpenSource(){

	VEConfig::initialize();

	std::string filename = getVideoPath(0);

	VE_ERR ret = VESource::openSource(NULL,NULL);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	AVFormatContext *fmtCtx = NULL;

	ret = VESource::openSource(&fmtCtx,"hello");

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	fmtCtx = NULL;
	VESource::setUnitTest(true);
	ret = VESource::openSource(&fmtCtx,filename.c_str());

	VESource::setUnitTest(false);
	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	return 0;
}
int testVESourceGetAudioFilterString(){

	ve_audio_filter_param audioFilterParam;

	audioFilterParam.m_volume = 0;
	audioFilterParam.m_samplerateForSpeed = 44100;
	audioFilterParam.m_fade = VE_AUDIO_FILTER_FADE_IN;
	char outStr[1024];


	VESource::getAudioFilterString(&audioFilterParam,outStr);

	audioFilterParam.m_volume = 100;
	audioFilterParam.m_fade = VE_AUDIO_FILTER_FADE_OUT;
	VESource::getAudioFilterString(&audioFilterParam,outStr);

	audioFilterParam.m_volume = 200;
	VESource::getAudioFilterString(&audioFilterParam,outStr);
	return 0;
}
int testVESourceGetSourceInfo(){

	ve_clip_info clipInfo;

	VEConfig::initialize();

	std::string filename = getVideoPath(0);

	VE_ERR ret = VESource::getSourceInfo(0,0);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	VESource::setUnitTest(true);
	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	filename = getHevcVideoPath(0);

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);


	filename = getMusicPath(0);

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	VE_ASSERT_NEQ(ret,VE_ERR_OK);

	VESource::setUnitTest(false);

	filename = getVideoPath(0);

	slv_info slv;
	slv.active = true;
	slv.len = 1;
	slv.start_time[0] = 0;
	slv.end_time[0] = 1000;
	slv.speed[0] = 1;

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo,&slv);

	VE_ASSERT_EQ(ret,VE_ERR_OK);


	filename = getMusicPath(0);

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	VE_ASSERT_EQ(ret,VE_ERR_OK);

	filename = getPicPath(0);

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	VE_ASSERT_EQ(ret,VE_ERR_OK);
	return 0;
}
class VETestVESourceListener:public VESourceListener{
	void setStatus(int status){}
};
int testVESourceH264Video(){

	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getVideoPath(0);
	ve_clip clip;

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 200;
	clip.duration = 200;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);
	VETestVESourceListener listener;
	VETrackData & trackData = config.m_tracks[track.track_id];
	VEClipData & clipData = trackData.m_clips[0];
	clipData.m_index = 0;
	VESource* source = new VESource(VE_SOURCE_TYPE_VIDEO,&config,&clipData,&listener);

	source->start();
	bool bRet = source->isStart();

	VE_ASSERT_EQ(true,bRet);
	bRet = source->isStop();
	VE_ASSERT_EQ(false,bRet);

	int clipId = source->getClipId();

	VEClipData* ptrClipData = source->getClipData();

	ve_dec_yuv_buffer yuv;
	int gotPicture;
	int eof;
	int curTime = 0;

	for(;;){
		ret = source->getYuvBuffer(&yuv,&gotPicture,&eof,curTime);


#ifdef __APPLE__
		CVPixelBufferRelease((CVPixelBufferRef)yuv.m_data);
#endif
		source->returnYuv();
		curTime += 33;

		if(ret || eof)break;
	}


	source->stop();

	ve_dec_pcm_buffer pcm;
	source->m_pcmQueue.push(pcm);
	source->m_pcmPool.push(pcm);
	delete source;

	return 0;
}
int testVESourcePic(){
	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

	std::string filename = getPicPath(0);
	ve_clip clip;

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.type = VE_CLIP_PICTURE;
	clip.duration = 200;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	VE_ASSERT_EQ(0,ret);
	VETestVESourceListener listener;
	VETrackData & trackData = config.m_tracks[track.track_id];
	VEClipData & clipData = trackData.m_clips[0];
	VESource* source = new VESource(VE_SOURCE_TYPE_VIDEO,&config,&clipData,&listener);

	source->start();

	ve_dec_yuv_buffer yuv;
	int gotPicture;
	int eof;
	int curTime = 0;

	for(;;){
		ret = source->getYuvBuffer(&yuv,&gotPicture,&eof,curTime);
#ifdef __APPLE__
		CVPixelBufferRelease((CVPixelBufferRef)yuv.m_data);
#endif
		source->returnYuv();
		curTime += 33;

		if(ret || eof)break;
	}


	source->stop();
	delete source;

	return 0;
}
int testVESourceAudio(){
	ve_timeline timeline;
	VE_ERR ret;

	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();


	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
    
    audioTrack.track_id = getId();
	audioTrack.type = VE_TRACK_AUDIO;

	ret = config.addTrack(&track);
	VE_ASSERT_EQ(0,ret);

    ret = config.addTrack(&audioTrack);
    VE_ASSERT_EQ(0,ret);
    
    std::string filename = getVideoPath(0);
	std::string musicFilename = getMusicPath(0);
	ve_clip clip,audioClip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 200;
    clip.duration = 200;
    clip.speed = 1.2;
    clip.filename = filename.c_str();
    
    ret = config.insertClip(&clip,0);
    VE_ASSERT_EQ(0,ret);

    audioClip.track_id = audioTrack.track_id;
	audioClip.clip_id = getId();
	audioClip.start_time = 0;
	audioClip.end_time = 200;
	audioClip.duration = 200;
    audioClip.type = VE_CLIP_AUDIO;
    audioClip.speed = 1.2;
	audioClip.filename = musicFilename.c_str();

	ret = config.insertClip(&audioClip,0);
    VE_ASSERT_EQ(0,ret);

	ve_filter filter;
	filter.filter_id = getId();
	filter.start_time = 0;
	filter.end_time = 200;
	filter.action = "audio filter";
	filter.af_type = VE_AUDIO_FILTER_FADE_IN;
	filter.type = VE_FILTER_AUDIO;
	filter.track_id = audioClip.track_id;
	filter.clip_id = audioClip.clip_id;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	VE_ASSERT_EQ(0,ret);
	VETestVESourceListener listener;
	VETrackData & trackData = config.m_tracks[audioTrack.track_id];
	VEClipData & clipData = trackData.m_clips[0];
    clipData.m_index = 0;
	VESource* source = new VESource(VE_SOURCE_TYPE_AUDIO,&config,&clipData,&listener);

	source->start();


	ve_dec_pcm_buffer pcm;
	int gotPcm;
	int eof;
	int curTime = 0;

	for(;;){
		ret = source->getPcmBuffer(&pcm,&gotPcm,&eof,curTime);
        if(gotPcm){
            source->returnPcm();
            curTime += pcm.m_sampleSize * 1000 / pcm.m_samplerate;
        }
		if(ret || eof)break;
	}


	source->stop();
	delete source;

	return 0;
}
int testVESourceAudioNoFilter(){
    ve_timeline timeline;
    VE_ERR ret;
    
    std::string outputFilename = getOutputDir();
    outputFilename += "output.mp4";
    timeline.timeline_id = getId();
    timeline.filename = outputFilename.c_str();
    
    
    VEConfig config(&timeline);
    
    int sequnceTrackId;
    ve_track track,audioTrack;
    sequnceTrackId = track.track_id = getId();
    
    audioTrack.track_id = getId();
    audioTrack.type = VE_TRACK_AUDIO;
    
    ret = config.addTrack(&track);
    VE_ASSERT_EQ(0,ret);
    
    ret = config.addTrack(&audioTrack);
    VE_ASSERT_EQ(0,ret);
    
    std::string filename = getVideoPath(0);
    std::string musicFilename = getMusicPath(0);
    ve_clip clip,audioClip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 200;
    clip.duration = 200;
    clip.speed = 1.2;
    clip.filename = filename.c_str();
    
    ret = config.insertClip(&clip,0);
    VE_ASSERT_EQ(0,ret);
    
    audioClip.track_id = audioTrack.track_id;
    audioClip.clip_id = getId();
    audioClip.start_time = 0;
    audioClip.end_time = 200;
    audioClip.duration = 200;
    audioClip.type = VE_CLIP_AUDIO;
    audioClip.speed = 1.2;
    audioClip.filename = musicFilename.c_str();
    
    ret = config.insertClip(&audioClip,0);
    VE_ASSERT_EQ(0,ret);

	VETestVESourceListener listener;
	VETrackData & trackData = config.m_tracks[audioTrack.track_id];
	VEClipData & clipData = trackData.m_clips[0];
    clipData.m_index = 0;
	VESource* source = new VESource(VE_SOURCE_TYPE_AUDIO,&config,&clipData,&listener);

	source->start();


	ve_dec_pcm_buffer pcm;
	int gotPcm;
	int eof;
	int curTime = 0;

	for(;;){
		ret = source->getPcmBuffer(&pcm,&gotPcm,&eof,curTime);
        if(gotPcm){
            source->returnPcm();
            curTime += pcm.m_sampleSize * 1000 / pcm.m_samplerate;
        }
		if(ret || eof)break;
	}


	source->stop();
	delete source;

	return 0;
}
int testVESourceAudioSlv(){
    
    ve_timeline timeline;
    VE_ERR ret;
    
    std::string outputFilename = getOutputDir();
    outputFilename += "output.mp4";
    timeline.timeline_id = getId();
    timeline.filename = outputFilename.c_str();
    
    
    VEConfig config(&timeline);
    
    int sequnceTrackId;
    ve_track track,audioTrack;
    sequnceTrackId = track.track_id = getId();
    
    audioTrack.track_id = getId();
    audioTrack.type = VE_TRACK_AUDIO;
    
    ret = config.addTrack(&track);
    VE_ASSERT_EQ(0,ret);
    
    ret = config.addTrack(&audioTrack);
    VE_ASSERT_EQ(0,ret);
    
    std::string filename = getVideoPath(0);
    std::string musicFilename = getMusicPath(0);
    ve_clip clip,audioClip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 200;
    clip.duration = 200;
    clip.speed = 1.2;
    clip.filename = filename.c_str();
    
    ret = config.insertClip(&clip,0);
    VE_ASSERT_EQ(0,ret);
    
    audioClip.track_id = audioTrack.track_id;
    audioClip.clip_id = getId();
    audioClip.start_time = 0;
    audioClip.end_time = 200;
    audioClip.duration = 200;
    audioClip.type = VE_CLIP_AUDIO;
    audioClip.speed = 1.2;
    audioClip.filename = musicFilename.c_str();
    audioClip.slv.active = 1;
    audioClip.slv.len = 1;
    audioClip.slv.start_time[0] = 0;
    audioClip.slv.end_time[0] = 200;
    audioClip.slv.speed[0] = 1;
    
    ret = config.insertClip(&audioClip,0);
    VE_ASSERT_EQ(0,ret);

	ve_filter filter;
	filter.filter_id = getId();
	filter.start_time = 0;
	filter.end_time = 200;
	filter.action = "audio filter";
	filter.af_type = VE_AUDIO_FILTER_FADE_IN;
	filter.type = VE_FILTER_AUDIO;
	filter.track_id = clip.track_id;
	filter.clip_id = clip.clip_id;
	ret = config.addFilter(&filter);
	VE_ASSERT_EQ(0,ret);

	VE_ASSERT_EQ(0,ret);
	VETestVESourceListener listener;
	VETrackData & trackData = config.m_tracks[audioTrack.track_id];
	VEClipData & clipData = trackData.m_clips[0];
    clipData.m_index = 0;
	VESource* source = new VESource(VE_SOURCE_TYPE_AUDIO,&config,&clipData,&listener);

	source->start();


	ve_dec_pcm_buffer pcm;
	int gotPcm;
	int eof;
	int curTime = 0;

	for(;;){
		ret = source->getPcmBuffer(&pcm,&gotPcm,&eof,curTime);
        if(gotPcm){
            source->returnPcm();
            curTime += pcm.m_sampleSize * 1000 / pcm.m_samplerate;
        }
		if(ret || eof)break;
	}


	source->stop();
	delete source;

	return 0;
}
int testVESourceGetDurationForPicture(){

	AVFormatContext* fmtCtx = NULL;
	std::string filename = getVideoPath(0);

	VE_ERR ret = VESource::openSource(&fmtCtx,filename.c_str());

	VE_ASSERT_EQ(0,ret);

	int duration = VESource::getDurationForPicture(fmtCtx,0);

	return 0;
}
bool veIntComp(const int &a,const int &b){
	return a < b;
}
VESemaphore sema;
void* semeThread(void* context){
	av_usleep(1000000);
	sema.post();
	av_usleep(1000000);
	sema.post();
    return 0;
}
int testVEQueue(){
	VEQueue<int> queue;
	queue.push(1);
	queue.push_back(2);
	queue.push_front(3);
	queue.post();
	queue.post();
	queue.post();

	VE_ASSERT_EQ(queue.empty(),false);
	VE_ASSERT_EQ(queue.size(),3);

	queue.sort(veIntComp);
	queue.wait();
	int num = queue.front();
	queue.pop();

	queue.wait();
	queue.pop_front();
	queue.pop();


	sema.post();
	sema.post();
	sema.tryWait();
	sema.tryWait();
	pthread_t semaThreadT;
	pthread_create(&semaThreadT, NULL, semeThread, 0);
	sema.tryWait();
	sema.wait();

	VELock lock(VE_LOCK_TYPE_NORMAL);
	lock.getLock();
	lock.trylock();
	lock.unlock();

	return 0;
}
int exitExport = 0;
uint8_t* rgbaData = 0;
int rgbaDataLen = 0;
void veFilterCallback(HANDLE client_handle,ve_filter_callback_param* param,void* userExt){
	VEExport *veExport = (VEExport*)client_handle;
	ve_filtered_data filteredData;

	filteredData.width = param->multitracks[0].tracks[0].frame_data[0].width;
	filteredData.height = param->multitracks[0].tracks[0].frame_data[0].height;
	filteredData.cur_time = param->cur_time;

#ifdef __APPLE__
	filteredData.data = param->multitracks[0].tracks[0].frame_data[0].data;
	filteredData.len = param->multitracks[0].tracks[0].frame_data[0].len;
#else
	int len = filteredData.width * filteredData.height * 4;

	if(rgbaDataLen < len){
		delete[] rgbaData;
		rgbaDataLen = len;
		rgbaData = new uint8_t[len];
	}
	filteredData.data = rgbaData;
	filteredData.len = rgbaDataLen;
	filteredData.frame_type = ANDROID_RGBA;
#endif


	veExport->sendData(&filteredData);
}
void veExportStatusCallback(ve_export_status* status){
	if(status->status){
		exitExport = status->status;
	}
}
int testVEExport1(){

    
	VEConfig config = getVEConfig1();

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);
	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}
	veExport.setStatus(0);

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEExport2(){

	VEConfig config = getVEConfig2();

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEExport3(){


	VEConfig config = getVEConfig3();

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEExport4(){


	VEConfig config = getVEConfig4();

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEExport5(){


	VEConfig config = getTransitionConfig();

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}


int testVEExport6(){


	VEConfig config = getSLVConfig();

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEExport7(){


	VEConfig config = getVESFConfig(getWebmVideoPath(0), 0, 1000,0);

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEExport8(){



    VEConfig config = getVESFConfig(getRgbaPicPath(0), 0, 1000,1);

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEExport9(){


	VEConfig config = getVEConfig5();

	ve_export_param param;

	VEExport veExport(&config,veFilterCallback,veExportStatusCallback,param);

	exitExport = 0;
	veExport.start();

	while(!exitExport){
		av_usleep(50000);
	}

	VE_ASSERT_EQ(1,exitExport);

	return 0;
}

int testVEMp4Writer(){

	ve_mp4_writer config;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	config.m_filename = outputFilename;
	config.m_vConfig.m_width = 1080;
	config.m_vConfig.m_height = 720;
	config.m_vConfig.m_fps = 30;
	config.m_vConfig.m_gop = 30;
	config.m_vConfig.m_vTimebase = AVRational{1,1000000};
	config.m_vConfig.m_yuvQueueLen = 7 + 1;
#ifdef __APPLE__
	config.m_vConfig.m_type = VE_ENC_VT_H264;
#else

	config.m_vConfig.m_type = VE_ENC_H264;

#endif

	config.m_fmt = "mp4";
	config.m_duration = 1000;
	config.m_aConfig.m_aTimebase = AVRational{1,44100};
	config.m_aConfig.m_channels = 2;
	config.m_aConfig.m_samplefmt = AV_SAMPLE_FMT_S16;
	config.m_aConfig.m_bytesPerSample = av_get_bytes_per_sample(config.m_aConfig.m_samplefmt) * config.m_aConfig.m_channels;
	config.m_aConfig.m_channelLayout = AV_CH_LAYOUT_STEREO;
	config.m_aConfig.m_samplerate = 44100;
	config.m_aConfig.m_type = VE_ENC_FDK_AAC;

	config.m_startTime = av_gettime();
	VEMp4Writer writer(&config);

	writer.start();

	ve_enc_yuv_buffer yuv;

	writer.m_status = VE_ERR_MALLOC_FAILED;
	writer.write(&yuv);
	writer.m_status = VE_ERR_OK;
	writer.m_stop = 1;
	writer.write(&yuv);
	writer.m_stop = 0;
	writer.write(&yuv);

	char packet[4096];
	yuv.m_data = (uint8_t*)packet;
	yuv.m_len = 265;
	yuv.m_format = VE_YUV_VT_PIXELBUFFER;
	writer.write(&yuv);
	writer.m_filteredYuvQueue.pop();

	yuv.m_format = VE_H264_PACKET;
	yuv.m_frameType = ANDROID_MEDIACODEC_PPS_SPS;;
	writer.write(&yuv);

	yuv.m_frameType = ANDROID_MEDIACODEC_KEY_FRAME;;
	writer.write(&yuv);

	yuv.m_format = VE_RGBA;
	writer.write(&yuv);
	writer.m_filteredYuvQueue.pop();

	ve_enc_pcm_buffer pcm;
	writer.m_status = VE_ERR_MALLOC_FAILED;
	writer.writeAndEnc(&pcm);
	writer.m_status = VE_ERR_OK;
	writer.m_stop = 1;
	writer.writeAndEnc(&pcm);
	writer.m_stop = 0;
	writer.writeAndEnc(&pcm);
	pcm.m_data = (uint8_t*)packet;
	pcm.m_len = 4097;

	writer.writeAndEnc(&pcm);
	pcm.m_len = 4096;

	writer.writeAndEnc(&pcm);

	writer.writeExtradata((uint8_t*)packet,4096);

	yuv.m_data = 0;
	yuv.m_len = 0;
	writer.m_filteredYuvSlotQueue.push(yuv);

    yuv.m_len = 1080 * 720 * 3 /2;
#ifdef __APPLE__
	yuv.m_data = (uint8_t*)getPixelBuffer(1080,720);
#else
    yuv.m_data = new uint8_t[yuv.m_len];
#endif
	
	writer.m_filteredYuvQueue.push(yuv);
	AVPacket pkt;
	writer.m_aMuxPktQueue.push(pkt);
	writer.m_vMuxPktQueue.push(pkt);

	writer.stop();

    return 0;
}
int testVEMp4Writer2(){


	VEConfig::initialize();
	ve_mp4_writer config;
	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	config.m_filename = outputFilename;
	config.m_vConfig.m_width = 1080;
	config.m_vConfig.m_height = 720;
	config.m_vConfig.m_fps = 30;
	config.m_vConfig.m_gop = 30;
	config.m_vConfig.m_vTimebase = AVRational{1,1000000};
	config.m_vConfig.m_yuvQueueLen = 7 + 1;
#ifdef __APPLE__
	config.m_vConfig.m_type = VE_ENC_VT_H264;
#else

	config.m_vConfig.m_type = VE_ENC_H264;

#endif

	config.m_fmt = "mp4";
	config.m_duration = 1000;
	config.m_aConfig.m_aTimebase = AVRational{1,44100};
	config.m_aConfig.m_channels = 2;
	config.m_aConfig.m_samplefmt = AV_SAMPLE_FMT_S16;
	config.m_aConfig.m_bytesPerSample = av_get_bytes_per_sample(config.m_aConfig.m_samplefmt) * config.m_aConfig.m_channels;
	config.m_aConfig.m_channelLayout = AV_CH_LAYOUT_STEREO;
	config.m_aConfig.m_samplerate = 44100;
	config.m_aConfig.m_type = VE_ENC_FDK_AAC;

	config.m_startTime = av_gettime();
	VEMp4Writer * writer = new VEMp4Writer(&config);


	ve_enc_yuv_buffer yuv,yuv2;

	writer->m_status = VE_ERR_MALLOC_FAILED;
	writer->write(&yuv);
	writer->m_status = VE_ERR_OK;
	writer->m_stop = 1;
	writer->write(&yuv);
	writer->m_stop = 0;
	writer->write(&yuv);


	char packet[4096];
	yuv.m_data = (uint8_t*)packet;
	yuv.m_len = 265;

	yuv.m_format = VE_H264_PACKET;
	yuv.m_frameType = ANDROID_MEDIACODEC_PPS_SPS;;
	writer->write(&yuv);

	yuv.m_frameType = ANDROID_MEDIACODEC_KEY_FRAME;;
	writer->write(&yuv);


	ve_enc_pcm_buffer pcm;
	writer->m_status = VE_ERR_MALLOC_FAILED;
	writer->writeAndEnc(&pcm);
	writer->m_status = VE_ERR_OK;
	writer->m_stop = 1;
	writer->writeAndEnc(&pcm);
	writer->m_stop = 0;
	writer->writeAndEnc(&pcm);

	pcm.m_data = (uint8_t*)packet;
	pcm.m_len = 65537;

	writer->writeAndEnc(&pcm);
	pcm.m_len = 4096;

	writer->writeAndEnc(&pcm);

	writer->stop();

	delete writer;

	writer = new VEMp4Writer(&config);

	writer->start();

	writer->writeExtradata((uint8_t*)packet,32);

	writer->stop();
	delete writer;

	writer = new VEMp4Writer(&config);


	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	pkt.data = 0;
	pkt.size = 0;

	writer->m_aMuxPktQueue.push(pkt);
	writer->m_vMuxPktQueue.push(pkt);
	writer->m_filteredYuvQueue.push(yuv2);
	writer->m_filteredYuvSlotQueue.push(yuv2);

	delete writer;

    return 0;
}
int testVEAudioFilter(){

	VEAudioFilter filter;

	filter.init(0);
	filter.addFilters("volume=volume=1.0");
	

	AVCodecContext ctx;
	ctx.channel_layout = 0;
	ctx.channels = 2;
	filter.init(&ctx);

	ctx.time_base = AVRational{1,44100};

	filter.init(&ctx);
    
    filter.releaseMem();

    filter.addFilters("volume=volume=1.0");
    ctx.channels = 1;
    ctx.channel_layout = 0;
	filter.init(&ctx);

	filter.init(&ctx);

	return 0;
}
int testVEThumbnail1(){

	VEThumbnailKey key1,key2;

	key1.width = 100;
	key1.height = 100;
	key1.filename = "a";

	key2.width = 98;
	key2.height = 100;
	key2.filename = "b";

	key1 < key2;

	key2.filename = "a";


	key1 < key2;

	key2.width = 102;
	key1 < key2;

	key2.width = 100;
	key2.height = 102;

	key1 < key2;


	VEThumbnailCmd cmd1,cmd2;

	cmd1.filename = "a";
	cmd2.filename = "b";

	cmd1 < cmd2;

	cmd1.filename = "c";

	cmd1 < cmd2;

	cmd1.filename = "b";

	cmd1.width = 101;
	cmd2.width = 100;
	cmd1 < cmd2;

	cmd1.width = 98;
	cmd1 < cmd2;

	cmd1.width = 100;

	cmd1.height = 101;
	cmd2.height = 100;

	cmd1 < cmd2;

	cmd1.height = 98;

	cmd1 < cmd2;

	cmd1.height = 100;

	cmd1.startTime = 101;
	cmd2.startTime = 100;

	cmd1 < cmd2;

	cmd1.startTime = 98;
	cmd1 < cmd2;

	cmd1.startTime = 100;

	cmd1.endTime = 102;
	cmd2.endTime = 100;

	cmd1 < cmd2;

	cmd1.endTime = 98;
	cmd1 < cmd2;

	cmd1.endTime = 100;

	cmd1.count = 12;
	cmd2.count = 10;

	cmd1 < cmd2;

	cmd1.count = 8;
	cmd1 < cmd2;

	cmd1.count = 10;

	cmd1.rotate = (VE_ROTATE)1;
	cmd2.rotate = (VE_ROTATE)2;

	cmd1 < cmd2;
	cmd1.rotate = (VE_ROTATE)3;

	cmd1 < cmd2;

	cmd1.rotate = (VE_ROTATE)4;

	cmd1.callback = (ve_thumbnail_callback)1;
	cmd2.callback = (ve_thumbnail_callback)2;

	cmd1 < cmd2;

	cmd1.callback = (ve_thumbnail_callback)3;

	cmd1 < cmd2;

	cmd1.callback = (ve_thumbnail_callback)2;

	cmd1.userExt = (void*)1;
	cmd2.userExt = (void*)2;

	cmd1 < cmd2;

	cmd1.userExt = (void*)3;
	cmd1 < cmd2;
	cmd1.userExt = (void*)2;

	cmd1.path = "a";

	cmd2.path = "b";

	cmd1 < cmd2;

	cmd1.path = "c";
	cmd1 < cmd2;

	cmd1.path = "b";
	cmd1 < cmd2;

	return 0;
}

int testAll(){

    VE_ASSERT_EQ(0,testInterfaceUitls());

    VE_ASSERT_EQ(0,testInterfaceTimeline());

    VE_ASSERT_EQ(0,testInterfaceTrack());

    VE_ASSERT_EQ(0,testInterfaceClip());

    VE_ASSERT_EQ(0,testInterfaceTransition());

    VE_ASSERT_EQ(0,testInterfaceFilter());

    VE_ASSERT_EQ(0,testInterfaceExport());

    VE_ASSERT_EQ(0,testInterfaceThumbnail());


    VE_ASSERT_EQ(0,testInterfaceWebm());
    VE_ASSERT_EQ(0,testInterfaceWebm2());

    VE_ASSERT_EQ(0,testVEUtils());

    VE_ASSERT_EQ(0,testVESwsscale());

    VE_ASSERT_EQ(0,testVEBitstream());

    VE_ASSERT_EQ(0,testVEConfigBase());


    VE_ASSERT_EQ(0,testVEConfigTrack());

    VE_ASSERT_EQ(0,testVEConfigInsertClip1());

    VE_ASSERT_EQ(0,testVEConfigInsertClip2());

    VE_ASSERT_EQ(0,testVEConfigModClip1());

    VE_ASSERT_EQ(0,testVEConfigModClip2());

    VE_ASSERT_EQ(0,testVEConfigMoveClip());

    VE_ASSERT_EQ(0,testVEConfigDelClip1());

    VE_ASSERT_EQ(0,testVEConfigDelClip2());

    VE_ASSERT_EQ(0,testVEConfigGetClip());

    VE_ASSERT_EQ(0,testVEConfigGetClipDuration());


    VE_ASSERT_EQ(0,testVEConfigGetClipsCount());


    VE_ASSERT_EQ(0,testVEConfigGetClips());

    VE_ASSERT_EQ(0,testVEConfigGetTrackDuration());

    VE_ASSERT_EQ(0,testVEConfigAddTransition());

    VE_ASSERT_EQ(0,testVEConfigModTransition());


    VE_ASSERT_EQ(0,testVEConfigGetTransition());

    VE_ASSERT_EQ(0,testVEConfigDelTransition());

    VE_ASSERT_EQ(0,testVEConfigAddFilter());

    VE_ASSERT_EQ(0,testVEConfigModFilter());

    VE_ASSERT_EQ(0,testVEConfigGetFilter());

    VE_ASSERT_EQ(0,testVEConfigDelFilter());


    VE_ASSERT_EQ(0,testVEThumbnail());

    VE_ASSERT_EQ(0,testVESourceOpenSource());


    VE_ASSERT_EQ(0,testVESourceGetAudioFilterString());

    VE_ASSERT_EQ(0,testVESourceGetSourceInfo());


    VE_ASSERT_EQ(0,testVESourceH264Video());

    VE_ASSERT_EQ(0,testVESourcePic());


    VE_ASSERT_EQ(0,testVESourceAudio());

    VE_ASSERT_EQ(0,testVESourceAudioNoFilter());

    VE_ASSERT_EQ(0,testVESourceAudioSlv());


    VE_ASSERT_EQ(0,testVESourceGetDurationForPicture());

    VE_ASSERT_EQ(0,testVEExport1());

    VE_ASSERT_EQ(0,testVEExport2());

    VE_ASSERT_EQ(0,testVEExport3());

    VE_ASSERT_EQ(0,testVEExport4());

    VE_ASSERT_EQ(0,testVEExport5());

    VE_ASSERT_EQ(0,testVEExport6());

    VE_ASSERT_EQ(0,testVEExport7());

    VE_ASSERT_EQ(0,testVEExport8());

    VE_ASSERT_EQ(0,testVEExport9());


    VE_ASSERT_EQ(0,testVEQueue());

    VE_ASSERT_EQ(0,testVEMp4Writer());

    VE_ASSERT_EQ(0,testVEAudioFilter());


	VE_ASSERT_EQ(0,testVEThumbnail1());


	return 0;
}
