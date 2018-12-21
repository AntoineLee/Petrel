//
//  QHVCEditTimelineManager.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditTimelineManager.h"
#import "QHVCEditLogger.h"
#import "QHVCEditConfig.h"
#import "QHVCEditUtilsSet.h"

#import "QHVCEditCommonDef.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditTrack.h"
#import "QHVCEditEffect.h"
#import "ve_interface.h"

@interface QHVCEditTimelineManager ()
{
    HANDLE _timelineHandle;
}

@property (nonatomic, retain) QHVCEditTimeline* timeline;

//属性
@property (nonatomic, assign) CGSize outputSize;
@property (nonatomic, assign) NSInteger outputBitrate;
//@property (nonatomic, assign) QHVCEditFillMode defaultFillMode;
//@property (nonatomic, strong) QHVCEditBgParams* defaultBgParams;
@property (nonatomic, retain) NSString* outputBgColor;
@property (nonatomic, assign) NSInteger outputFps;
@property (nonatomic, assign) CGFloat speed;
@property (nonatomic, assign) NSInteger volume;
@property (nonatomic, strong) NSString* outputPath;

//特效
@property (atomic,    strong) NSMutableDictionary* effects;

@end

@implementation QHVCEditTimelineManager

#pragma mark - 初始化、释放

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        QHVCEditError err = [self ve_createTimelineHandle];
        if (err == QHVCEditErrorNoError)
        {
            self.timeline = timeline;
            self.effects = [[NSMutableDictionary alloc] initWithCapacity:0];
            _outputBgColor = @"FF000000";
        }
    }
    
    return self;
}

- (QHVCEditError)freeTimeline
{
    QHVCEditError err = [self ve_freeTimelineHandle];
    return err;
}

- (void *)getTimelineHandle
{
    return _timelineHandle;
}

#pragma mark - 属性

- (QHVCEditError)setOutputWidth:(NSInteger)width height:(NSInteger)height
{
    if (width <= 0)
    {
        LogError(@"timeline setOutputWidth: height: error, width <= 0");
        return QHVCEditErrorParamError;
    }
    
    if (height <= 0)
    {
        LogError(@"timeline setOutputWidth: height: error, height <= 0");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self ve_timelineEditOutputSize:CGSizeMake(width, height)];
    if (err == QHVCEditErrorNoError)
    {
        self.outputSize = CGSizeMake(width, height);
    }
    
    return err;
}

- (CGSize)outputSize
{
    return _outputSize;
}

- (QHVCEditError)setTimelineOutputBgColor:(NSString *)bgColor
{
    _outputBgColor = bgColor;
    return QHVCEditErrorNoError;
}

- (NSString *)outputBgColor
{
    return _outputBgColor;
}

- (QHVCEditError)setTimelineOutputFps:(NSInteger)fps
{
    if (fps <= 0 || fps > 60)
    {
        LogError(@"timeline set output fps error, fps need between 1 and 60");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self ve_timelineEditFps:fps];
    if (err == QHVCEditErrorNoError)
    {
        self.outputFps = fps;
    }
    
    return err;
}

- (NSInteger)outputFps
{
    return _outputFps;
}

- (QHVCEditError)setTimelineOutputBitrate:(NSInteger)bitrate
{
    if (bitrate <= 0)
    {
        LogError(@"timeline set output bitrate error, bitrate <= 0");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self ve_timelineEditBitrate:bitrate];
    if (err == QHVCEditErrorNoError)
    {
        self.outputBitrate = bitrate;
    }
    
    return err;
}

- (NSInteger)outputBitrate
{
    return _outputBitrate;
}

- (QHVCEditError)setTimelineSpeed:(CGFloat)speed
{
    if (speed <= 0)
    {
        LogError(@"timeline set speed error, speed <= 0");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self ve_timelineEditSpeed:speed];
    if (err == QHVCEditErrorNoError)
    {
        self.speed = speed;
    }
    
    return err;
}

- (CGFloat)speed
{
    return _speed;
}

- (QHVCEditError)setTimelineVolume:(NSInteger)volume
{
    QHVCEditError err = [self ve_timelineEditVolume:volume];
    if (err == QHVCEditErrorNoError)
    {
        self.volume = volume;
    }
    
    return err;
}

- (NSInteger)volume
{
    return _volume;
}

- (QHVCEditError)setTimelineOutputPath:(NSString *)filePath
{
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        LogError(@"timeline set output path error, path is nil");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self ve_timelineEditOutputPath:filePath];
    if (err == QHVCEditErrorNoError)
    {
        self.outputPath = filePath;
    }
    
    return err;
}

- (NSString *)outputPath
{
    return _outputPath;
}

- (NSInteger)duration
{
    NSInteger duration = [self ve_timelineDuration];
    return duration;
}

#pragma mark - 轨道相关

- (QHVCEditError)appendTrack:(QHVCEditTrack *)track
{
    QHVCEditError err = [self ve_addTrack:track];
    return err;
}

- (QHVCEditError)deleteTrackById:(NSInteger)trackId
{
    QHVCEditError err = [self ve_deleteTrack:trackId];
    return err;
}

- (QHVCEditError)addEffect:(QHVCEditEffect *)effect
{
    NSNumber* effectId = [NSNumber numberWithInteger:[effect effectId]];
    QHVCEditEffect* obj = [self.effects objectForKey:effectId];
    if (obj)
    {
        LogError(@"timeline addEffect error, effect already exist");
        return QHVCEditErrorAlreayExist;
    }
    
    QHVCEditError err = [self ve_addEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects setObject:effect forKey:effectId];
    }
    
    return err;
}

- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect
{
    NSNumber* effectId = [NSNumber numberWithInteger:[effect effectId]];
    QHVCEditEffect* obj = [self.effects objectForKey:effectId];
    if (!obj)
    {
        LogError(@"timeline updateEffect error, effect not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [self ve_updateEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects setObject:effect forKey:effectId];
    }
    
    return err;
}

- (QHVCEditError)deleteEffectById:(NSInteger)effectId
{
    QHVCEditError err = [self ve_deleteEffect:effectId];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects removeObjectForKey:[NSNumber numberWithInteger:effectId]];
    }
    
    return err;
}

- (QHVCEditEffect *)getEffectById:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    QHVCEditEffect* obj = [self.effects objectForKey:effectIdNum];
    return obj;
}

- (NSArray<QHVCEditEffect *>*)getEffects
{
    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:0];
    [self.effects enumerateKeysAndObjectsUsingBlock:^(NSNumber* key, QHVCEditEffect* obj, BOOL * _Nonnull stop)
    {
        if (obj)
        {
            [array addObject:obj];
        }
    }];
    
    return array;
}

#pragma mark - video_edit Methods

- (QHVCEditError)ve_createTimelineHandle
{
    HANDLE handle = ve_timeline_create();
    if (!handle)
    {
        LogError(@"create timeline handle error");
        return QHVCEditErrorParamError;
    }
    
    _timelineHandle = handle;
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_freeTimelineHandle
{
    if (_timelineHandle)
    {
        ve_timeline_free(_timelineHandle);
        _timelineHandle = nil;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_timelineEditOutputPath:(NSString *)path
{
    ve_timeline timeline;
    QHVCEditError err = [self ve_timelineGetConfig:&timeline];
    if (err != QHVCEditErrorNoError)
    {
        return QHVCEditErrorParamError;
    }
        
    timeline.filename = [path UTF8String];
    return [self ve_timelineEditParams:timeline];
}

- (QHVCEditError)ve_timelineEditOutputSize:(CGSize)size
{
    ve_timeline timeline;
    QHVCEditError err = [self ve_timelineGetConfig:&timeline];
    if (err != QHVCEditErrorNoError) {
        return QHVCEditErrorParamError;
    }
    
    timeline.output_width = size.width;
    timeline.output_height = size.height;
    return [self ve_timelineEditParams:timeline];
}

- (QHVCEditError)ve_timelineEditFps:(NSInteger)fps
{
    ve_timeline timeline;
    QHVCEditError err = [self ve_timelineGetConfig:&timeline];
    if (err != QHVCEditErrorNoError) {
        return QHVCEditErrorParamError;
    }
        
    timeline.output_fps = (int)fps;
    return [self ve_timelineEditParams:timeline];
}

- (QHVCEditError)ve_timelineEditBitrate:(NSInteger)bitrate
{
    ve_timeline timeline;
    QHVCEditError err = [self ve_timelineGetConfig:&timeline];
    if (err != QHVCEditErrorNoError) {
        return QHVCEditErrorParamError;
    }
        
    timeline.video_bitrate = (int)bitrate;
    return [self ve_timelineEditParams:timeline];
}

- (QHVCEditError)ve_timelineEditSpeed:(CGFloat)speed
{
    ve_timeline timeline;
    QHVCEditError err = [self ve_timelineGetConfig:&timeline];
    if (err != QHVCEditErrorNoError) {
        return QHVCEditErrorParamError;
    }
        
    timeline.speed = speed;
    return [self ve_timelineEditParams:timeline];
}

- (QHVCEditError)ve_timelineEditVolume:(NSInteger)volume
{
    ve_timeline timeline;
    QHVCEditError err = [self ve_timelineGetConfig:&timeline];
    if (err != QHVCEditErrorNoError) {
        return QHVCEditErrorParamError;
    }
    
    timeline.volume = (int)volume;
    return [self ve_timelineEditParams:timeline];
}

- (QHVCEditError)ve_timelineEditParams:(ve_timeline)timeline
{
    VE_ERR ve_err = ve_timeline_reconfig(_timelineHandle, &timeline);
    if (ve_err != VE_ERR_OK)
    {
        NSString* info = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:ve_err];
        LogError(@"timeline edit param error, %@", info);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_timelineGetConfig:(ve_timeline *)timeline
{
    VE_ERR ve_err = ve_timeline_get_config(_timelineHandle, timeline);
    if (ve_err != VE_ERR_OK)
    {
        NSString* info = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:ve_err];
        LogError(@"timeline get timeline config param error, %@", info);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_addTrack:(QHVCEditTrack *)track
{
    VE_TRACK type = VE_TRACK_VIDEO;
    if ([track trackType] == QHVCEditTrackTypeAudio)
    {
        type = VE_TRACK_AUDIO;
    }
    
    VE_CLIP_ARRANGEMENT arrangement = VE_CLIP_ARRANGEMENT_OVERLAY;
    if ([track trackArrangement] == QHVCEditTrackArrangementSequence)
    {
        arrangement = VE_CLIP_ARRANGEMENT_SEQUENCE;
    }
    
    ve_track params;
    params.track_id = (int)[track trackId];
    params.type     = type;
    params.speed    = [track speed];
    params.volume   = (int)[track volume];
    params.clip_arrangement = arrangement;
    
    VE_ERR ve_err = ve_track_add(_timelineHandle, &params);
    if (ve_err != VE_ERR_OK)
    {
        NSString* info = [self printTrackError:track err:ve_err];
        LogError(@"timeline add track error, %@", info);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_deleteTrack:(NSInteger)trackId
{
    VE_ERR ve_err = ve_track_del(_timelineHandle, (int)trackId);
    if (ve_err != VE_ERR_OK)
    {
        NSString* info = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:ve_err];
        LogError(@"timeline delete track %ld error, %@", (long)trackId, info);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (NSInteger)ve_timelineDuration
{
    int duration = 0;
    VE_ERR err = ve_timeline_get_duration(_timelineHandle, 1, &duration);
    if (err != VE_ERR_OK)
    {
        NSString* info = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err];
        LogError(@"timeline get duration error, %@", info);
    }
    return duration;
}

- (QHVCEditError)ve_addEffect:(QHVCEditEffect *)effect
{
    ve_filter params;
    params.loc_type   = VE_FILTER_LOC_TIMELINE;
    params.type       = (VE_FILTER)[[QHVCEditConfig sharedInstance] getVEFilterType:effect];
    params.filter_id  = (int)effect.effectId;
    params.start_time = (int)effect.startTime;
    params.end_time   = (int)effect.endTime;
    
    VE_ERR err = ve_filter_add(_timelineHandle, &params);
    if (err != VE_ERR_OK)
    {
        NSString* info = [self printEffectError:effect err:err];
        LogError(@"timeline addEffect error, %@", info);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_updateEffect:(QHVCEditEffect *)effect
{
    ve_filter params;
    params.loc_type   = VE_FILTER_LOC_TIMELINE;
    params.type       = (VE_FILTER)[[QHVCEditConfig sharedInstance] getVEFilterType:effect];
    params.filter_id  = (int)effect.effectId;
    params.start_time = (int)effect.startTime;
    params.end_time   = (int)effect.endTime;
    
    VE_ERR err = ve_filter_mod(_timelineHandle, &params);
    if (err != VE_ERR_OK)
    {
        NSString* info = [self printEffectError:effect err:err];
        LogError(@"timeline updateEffect error, %@", info);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_deleteEffect:(NSInteger)effectId
{
    VE_ERR err = ve_filter_del(_timelineHandle, (int)effectId);
    if (err != VE_ERR_OK)
    {
        LogError(@"timeline delete effect error, %@", [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (NSString *)printTrackError:(QHVCEditTrack *)track err:(VE_ERR)err
{
    NSString* errInfo = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err];
    NSString* info = [NSString stringWithFormat:@"trackId = %ld, type = %lu, speed = %f, volume = %ld, %@", (long)[track trackId], (long)[track trackType], [track speed], (long)[track volume], errInfo];
    return info;
}

- (NSString *)printEffectError:(QHVCEditEffect *)effect err:(VE_ERR)err
{
    NSString* errInfo = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err];
    NSString* info = [NSString stringWithFormat:@"effectId = %ld, startTime = %ld, endTime = %ld, %@", (long)[effect effectId], (long)[effect startTime], (long)[effect endTime], errInfo];
    return info;
}

@end
