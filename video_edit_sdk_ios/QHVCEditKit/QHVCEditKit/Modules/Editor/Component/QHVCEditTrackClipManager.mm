//
//  QHVCEditTrackClipManager.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditTrackClipManager.h"
#import "QHVCEditTrackClip.h"
#import "QHVCEditEffect.h"
#import "ve_interface.h"
#import "QHVCEditConfig.h"
#import "QHVCEditLogger.h"

@interface QHVCEditTrackClipManager ()
{
    HANDLE _timelineHandle;
}

@property (nonatomic, retain) QHVCEditTrackClip* clip;
@property (atomic,    strong) NSMutableDictionary* effects;
@property (nonatomic, assign) NSInteger trackId;
@property (nonatomic, assign) NSInteger insertTime;

@end

@implementation QHVCEditTrackClipManager

- (instancetype)initWithClip:(QHVCEditTrackClip *)clip toTrack:(NSInteger)trackId ofTimelineHandle:(void *)timelineHandle
{
    self = [super init];
    if (self)
    {
        self.clip = clip;
        self.trackId = trackId;
        _timelineHandle = timelineHandle;
        self.effects = [[NSMutableDictionary alloc] initWithCapacity:0];
    }
    
    return self;
}

- (NSInteger)getTrackId
{
    return self.trackId;
}

- (QHVCEditTrackClip *)getClip
{
    return self.clip;
}

- (NSInteger)duration
{
    return [self ve_getDuration];
}

- (NSInteger)insertTime
{
    return _insertTime;
}

- (void)updateInsertTime:(NSInteger)time
{
    self.insertTime = time;
}

- (QHVCEditError)addEffect:(QHVCEditEffect *)effect
{
    NSNumber* effectId = [NSNumber numberWithInteger:[effect effectId]];
    QHVCEditError err = [self ve_addEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects setObject:effect forKey:effectId];
    }
    
    return err;
}

- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect
{
    QHVCEditError err = [self ve_updateEffect:effect];
    return err;
}

- (QHVCEditError)deleteEffectById:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    QHVCEditError err = [self ve_deleteEffectById:effectId];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects removeObjectForKey:effectIdNum];
    }
    
    return err;
}

- (QHVCEditEffect *)getEffectById:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    QHVCEditEffect* effect = [self.effects objectForKey:effectIdNum];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)getEffects
{
    NSMutableArray* effects = [[NSMutableArray alloc] initWithCapacity:0];
    [self.effects enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, QHVCEditEffect* obj, BOOL * _Nonnull stop)
     {
         [effects addObject:obj];
    }];
    return effects;
}

- (void)addVideoTransition:(NSInteger)transitionId
                  duration:(NSInteger)durationMs
            transitionName:(NSString *)transitionName
        easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    self.transitionId = transitionId;
    self.transitionDuration = durationMs;
    self.transitionName = transitionName;
    self.transitionEasingFunctionType = easingFunctionType;
    self.haveTransition = YES;
}

- (void)updateVideoTransition:(NSInteger)durationMs
               transitionName:(NSString *)transitionName
           easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    if (self.haveTransition)
    {
        self.transitionEasingFunctionType = easingFunctionType;
        self.transitionDuration = durationMs;
        self.transitionName = transitionName;
    }
}

- (void)deleteVideoTransition
{
    self.haveTransition = NO;
}

#pragma mark - video_edit Methods

- (NSInteger)ve_getDuration
{
    int duration;
    VE_ERR err = ve_clip_get_duration(_timelineHandle, (int)self.clip.clipId, &duration);
    if (err != VE_ERR_OK)
    {
        LogError(@"clip get duration error errorInfo[%@]",[[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return 0;
    }
    
    return (NSInteger)duration;
}

- (QHVCEditError)ve_addEffect:(QHVCEditEffect *)effect
{
    ve_filter veFilter;
    [self getVEFiterByEffect:effect veFilter:&veFilter];
    VE_ERR err = ve_filter_add(_timelineHandle, &veFilter);
    if (err != VE_ERR_OK)
    {
        LogError(@"clip add effect error effectId[%d] errorInfo[%@]", (int)effect.effectId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_updateEffect:(QHVCEditEffect *)effect
{
    ve_filter veFilter;
    [self getVEFiterByEffect:effect veFilter:&veFilter];
    VE_ERR err = ve_filter_mod(_timelineHandle, &veFilter);
    if (err != VE_ERR_OK)
    {
        LogError(@"clip update effect mod filter error effectId[%d] errorInfo[%@]", (int)effect.effectId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_deleteEffectById:(NSInteger)effectId
{
    VE_ERR err = ve_filter_del(_timelineHandle, (int)effectId);
    if (err != VE_ERR_OK)
    {
        LogError(@"clip delete effect error effectId[%d] errorInfo[%@]", (int)effectId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}


- (void)getVEFiterByEffect:(QHVCEditEffect *)effect veFilter:(ve_filter *)veFilter
{
    veFilter->track_id = (int)self.trackId;
    veFilter->clip_id = (int)self.clip.clipId;
    veFilter->filter_id = (int)effect.effectId;
    veFilter->type = (VE_FILTER)[[QHVCEditConfig sharedInstance] getVEFilterType:effect];
    veFilter->loc_type = VE_FILTER_LOC_CLIP;
    veFilter->start_time = (int)effect.startTime;
    veFilter->end_time = (int)effect.endTime;
    if (veFilter->type == VE_FILTER_VIDEO)
    {
        veFilter->action = "";
    }
    
    if (effect.effectType == QHVCEditEffectTypeAudio)
    {
        QHVCEditAudioTransferEffect *audioTransfer = (QHVCEditAudioTransferEffect *)effect;
        veFilter->af_type = (VE_AUDIO_FILTER_TYPE)audioTransfer.transferType;
        veFilter->fade_curve = (VE_AF_FADE_CURVE)audioTransfer.transferCurveType;
        veFilter->gain_min = (int)audioTransfer.gainMin;
        veFilter->gain_max = (int)audioTransfer.gainMax;
    }
}

@end
