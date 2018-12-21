#include "ve_interface.h"
#include "VEUnitTest.h"
#include "VEUnitTestConfig.h"
#include <string>
#include "VESource.h"

VEConfig getVEConfig1(){
	//单视频,只有视频track
    VEConfig::initialize();

	std::string filename = getVideoPath(0);

	ve_clip_info clipInfo;

    VE_ERR ret;

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	ve_timeline timeline;


	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	timeline.output_width = clipInfo.width;
	timeline.output_height = clipInfo.height;

	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);

	ve_clip clip;

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 1000;
	clip.duration = 1000;
	clip.pitch = 2;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	ve_filter filter,audioFilter;
	filter.filter_id = getId();
	filter.track_id = track.track_id;
	filter.clip_id = clip.clip_id;
	filter.start_time = 0;
	filter.end_time = 1000;
	filter.loc_type = VE_FILTER_LOC_CLIP;
	filter.action = "hi";
	ret = config.addFilter(&filter);
	

	audioFilter.filter_id = getId();
	audioFilter.track_id = track.track_id;
	audioFilter.clip_id = clip.clip_id;
	audioFilter.start_time = 0;
	audioFilter.end_time = 1000;
	audioFilter.type = VE_FILTER_AUDIO;
	audioFilter.loc_type = VE_FILTER_LOC_CLIP;
	audioFilter.af_type = VE_AUDIO_FILTER_FADE_IN;

	ret = config.addFilter(&audioFilter);
	
	return config;
}

VEConfig getVEConfig2(){
    VEConfig::initialize();

	std::string filename = getPicPath(0);

	ve_clip_info clipInfo;

    VE_ERR ret;

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	ve_timeline timeline;


	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	timeline.output_width = clipInfo.width;
	timeline.output_height = clipInfo.height;

	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);

	ve_clip clip;

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 0;
	clip.duration = 1000;
	clip.filename = filename.c_str();
	clip.type = VE_CLIP_PICTURE;

	ret = config.insertClip(&clip,0);

	return config;
}
VEConfig getVEConfig3(){
	//单视频,只有视频track
    VEConfig::initialize();

	std::string filename = getHevcVideoPath(1);

	ve_clip_info clipInfo;

    VE_ERR ret;

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	ve_timeline timeline;


	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	timeline.output_width = clipInfo.width;
	timeline.output_height = clipInfo.height;

	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);

	ve_clip clip;

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 1000;
	clip.duration = 1000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);

	return config;
}

VEConfig getVEConfig4(){
	//单个视频(有音频）+ 音频track，背景音乐
    VEConfig::initialize();

    std::string filename = getVideoPath(1);
    std::string audioFilename = getMusicPath(0);
    
	ve_clip_info clipInfo;

    VE_ERR ret;

	ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);

	ve_timeline timeline;


	std::string outputFilename = getOutputDir();
	outputFilename += "output.mp4";
	timeline.timeline_id = getId();
	timeline.filename = outputFilename.c_str();
	timeline.output_width = clipInfo.width;
	timeline.output_height = clipInfo.height;

	VEConfig config(&timeline);

	int sequnceTrackId;
	ve_track track,audioTrack;
	sequnceTrackId = track.track_id = getId();
	track.type = VE_TRACK_VIDEO;

	ret = config.addTrack(&track);

	audioTrack.track_id = getId();
	audioTrack.type = VE_TRACK_AUDIO;
	ret = config.addTrack(&audioTrack);

	ve_clip clip,audioClip;

    clip.track_id = track.track_id;
	clip.clip_id = getId();
	clip.start_time = 0;
	clip.end_time = 1000;
	clip.duration = 1000;
	clip.filename = filename.c_str();

	ret = config.insertClip(&clip,0);


	audioClip.track_id = audioTrack.track_id;
	audioClip.clip_id = getId();
	audioClip.start_time = 0;
	audioClip.end_time = 1000;
	audioClip.duration = 1000;
	audioClip.type = VE_CLIP_AUDIO;
	audioClip.filename = audioFilename.c_str();

	ret = config.insertClip(&audioClip,0);
	return config;
}

VEConfig getVEConfig5()
{
    //单个视频(无音频）+ 音频track，背景音乐,变速，调节音量
    VEConfig::initialize();
    
    std::string filename = getVideoPath(0);
    std::string audioFilename = getMusicPath(0);
    
    ve_clip_info clipInfo;
    
    VE_ERR ret;
    
    ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);
    
    ve_timeline timeline;
    
    
    std::string outputFilename = getOutputDir();
    outputFilename += "output.mp4";
    timeline.timeline_id = getId();
    timeline.filename = outputFilename.c_str();
    timeline.output_width = clipInfo.width;
    timeline.output_height = clipInfo.height;
    
    VEConfig config(&timeline);
    
    int sequnceTrackId;
    ve_track track,audioTrack;
    sequnceTrackId = track.track_id = getId();
    track.type = VE_TRACK_VIDEO;
    
    ret = config.addTrack(&track);
    
    audioTrack.track_id = getId();
    audioTrack.type = VE_TRACK_AUDIO;
    ret = config.addTrack(&audioTrack);
    
    ve_clip clip,audioClip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 3000;
    clip.duration = 3000;
    clip.filename = filename.c_str();
    
    ret = config.insertClip(&clip,0);
    
    
    audioClip.track_id = audioTrack.track_id;
    audioClip.clip_id = getId();
    audioClip.start_time = 1000;
    audioClip.end_time = 4000;
    audioClip.duration = 3000;
    audioClip.type = VE_CLIP_AUDIO;
    audioClip.filename = audioFilename.c_str();
    audioClip.volume = 50;
    audioClip.speed = 2.0;
    audioClip.pitch = 2.0;
    
    ret = config.insertClip(&audioClip,0);
    
    ve_filter filter;
    filter.track_id = audioTrack.track_id;
    filter.clip_id = audioClip.clip_id;
    filter.af_type = VE_AUDIO_FILTER_FADE_IN;
    filter.filter_id = getId();
    filter.type = VE_FILTER_AUDIO;
    filter.start_time = 2000;
    filter.end_time = 3000;
    config.addFilter(&filter);
    


	ve_filter trackFilter,timelineFilter;
	trackFilter.filter_id = getId();
	trackFilter.track_id = track.track_id;
	trackFilter.start_time = 0;
	trackFilter.end_time = 3000;
	trackFilter.loc_type = VE_FILTER_LOC_TRACK;
	trackFilter.action = "hi";
	ret = config.addFilter(&trackFilter);



	timelineFilter.filter_id = getId();
	timelineFilter.start_time = 0;
	timelineFilter.end_time = 3000;
	timelineFilter.loc_type = VE_FILTER_LOC_TIMELINE;
	timelineFilter.action = "hi";
	ret = config.addFilter(&timelineFilter);

    return config;
}


VEConfig getVESFConfig(const char *file,int start,int end,int pic){
    VEConfig::initialize();
    
    std::string filename = file;
    
    ve_clip_info clipInfo;
    
    VE_ERR ret;
    
    ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);
    
    ve_timeline timeline;
    
    
    std::string outputFilename = getOutputDir();
    outputFilename += "output.mp4";
    timeline.timeline_id = getId();
    timeline.filename = outputFilename.c_str();
    timeline.output_width = clipInfo.width;
    timeline.output_height = clipInfo.height;
    
    VEConfig config(&timeline);
    
    int sequnceTrackId;
    ve_track track,audioTrack;
    sequnceTrackId = track.track_id = getId();
    track.type = VE_TRACK_VIDEO;
    
    ret = config.addTrack(&track);
    
    ve_clip clip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    if(pic){
        clip.start_time = 0;
        clip.end_time = 0;
        clip.type = VE_CLIP_PICTURE;
    }else{
        clip.start_time = start;
        clip.end_time = end;
    }
    clip.duration = end - start;
    clip.volume = 50;
    clip.filename = filename.c_str();
    
    ret = config.insertClip(&clip,0);
    
    return config;
}

VEConfig getTransitionConfig(){
    VEConfig::initialize();
    
    std::string filename_v1 = getVideoPath(0);
    std::string filename_v2 = getVideoPath(1);
    
    ve_clip_info clipInfo;
    
    VE_ERR ret;
    
    ret = VESource::getSourceInfo(filename_v1.c_str(),&clipInfo);
    
    ve_timeline timeline;
    
    
    std::string outputFilename = getOutputDir();
    outputFilename += "output.mp4";
    timeline.timeline_id = getId();
    timeline.filename = outputFilename.c_str();
    timeline.output_width = clipInfo.width;
    timeline.output_height = clipInfo.height;
    
    VEConfig config(&timeline);
    
    int sequnceTrackId;
    ve_track track,audioTrack;
    sequnceTrackId = track.track_id = getId();
    track.type = VE_TRACK_VIDEO;
    
    ret = config.addTrack(&track);
    
    ve_clip clip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 3000;
    clip.duration = 3000;
    clip.filename = filename_v1.c_str();
    
    ret = config.insertClip(&clip,0);
    
    ve_clip clip2;
    
    clip2.track_id = track.track_id;
    clip2.clip_id = getId();
    clip2.start_time = 0;
    clip2.end_time = 3000;
    clip2.duration = 3000;
    clip2.filename = filename_v2.c_str();
    
    ret = config.insertClip(&clip2,1);
    
    ve_transition transition;
    transition.action = "transition";
    transition.clip_index_b = 1;
    transition.track_id = track.track_id;
    transition.duration = 2000;
    transition.transition_id = getId();
    config.addTransition(&transition);
    
    //overlay
    ve_track track2;
    track2.clip_arrangement = VE_CLIP_ARRANGEMENT_OVERLAY;
    track2.track_id = getId();
    track2.type = VE_TRACK_VIDEO;
    
    ret = config.addTrack(&track2);
    
    ve_clip clip3;
    clip3.track_id = track2.track_id;
    clip3.clip_id = getId();
    clip3.start_time = 0;
    clip3.end_time = 1000;
    clip3.duration = 1000;
    clip3.filename = filename_v1.c_str();
    clip3.insert_time = 4000;
    
    ret = config.insertClip(&clip3,-1);
    
    return config;
}

VEConfig getSLVConfig(){
    VEConfig::initialize();
    
    std::string filename = getVideoPath(1);
    
    ve_clip_info clipInfo;
    
    VE_ERR ret;
    
    ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);
    
    ve_timeline timeline;
    
    
    std::string outputFilename = getOutputDir();
    outputFilename += "output.mp4";
    timeline.timeline_id = getId();
    timeline.filename = outputFilename.c_str();
    timeline.output_width = clipInfo.width;
    timeline.output_height = clipInfo.height;
    
    VEConfig config(&timeline);
    
    int sequnceTrackId;
    ve_track track,audioTrack;
    sequnceTrackId = track.track_id = getId();
    track.type = VE_TRACK_VIDEO;
    
    ret = config.addTrack(&track);
    
    ve_clip clip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 3000;
    clip.duration = 3000;
    clip.volume = 200;
    clip.filename = filename.c_str();
    clip.slv.active = true;
    clip.slv.len = 1;
    clip.slv.start_time[0] = 0;
    clip.slv.end_time[0] = 3000;
    clip.slv.speed[0] = 0.5;
    
    
    ret = config.insertClip(&clip,0);
    
    return config;
}


VEConfig getVEConfig11(){
    //单视频,只有视频track
    VEConfig::initialize();
    
    std::string filename = getVideoPath(0);
    
    ve_clip_info clipInfo;
    
    VE_ERR ret;
    
    ret = VESource::getSourceInfo(filename.c_str(),&clipInfo);
    
    ve_timeline timeline;
    
    
    std::string outputFilename = getOutputDir();
    outputFilename += "output.mp4";
    timeline.timeline_id = getId();
    timeline.filename = outputFilename.c_str();
    timeline.output_width = clipInfo.width;
    timeline.output_height = clipInfo.height;
    
    VEConfig config(&timeline);
    
    int sequnceTrackId;
    ve_track track,audioTrack;
    sequnceTrackId = track.track_id = getId();
    track.type = VE_TRACK_VIDEO;
    
    ret = config.addTrack(&track);
    
    ve_clip clip;
    
    clip.track_id = track.track_id;
    clip.clip_id = getId();
    clip.start_time = 0;
    clip.end_time = 3000;
    clip.duration = 3000;
    clip.filename = filename.c_str();
    
    ret = config.insertClip(&clip,0);
    
    return config;
}
