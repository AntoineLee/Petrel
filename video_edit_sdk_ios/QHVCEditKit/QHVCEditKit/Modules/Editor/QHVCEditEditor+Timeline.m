//
//  QHVCEditEditor+Timeline.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditLogger.h"
#import "QHVCEditConfig.h"
#import "QHVCEditEffect.h"
#import "QHVCEditEffectManager.h"
#import "QHVCEditTrack.h"
#import "QHVCEditTrackManager.h"
#import "QHVCEditTimelineManager.h"

@implementation QHVCEditEditor(Timeline)

#pragma mark - timeline属性

- (QHVCEditError)setOutputWidth:(NSInteger)width height:(NSInteger)height
{
    return [self.timelineMgr setOutputWidth:width height:height];
}

- (CGSize)outputSize
{
    return [self.timelineMgr outputSize];
}

- (QHVCEditError)setOutputBgColor:(NSString *)bgColor
{
    return [self.timelineMgr setTimelineOutputBgColor:bgColor];
}

- (NSString *)outputBgColor
{
    return [self.timelineMgr outputBgColor];
}

- (QHVCEditError)setOutputFps:(NSInteger)fps
{
    return [self.timelineMgr setTimelineOutputFps:fps];
}

- (NSInteger)outputFps
{
    return [self.timelineMgr outputFps];
}

- (QHVCEditError)setOutputBitrate:(NSInteger)bitrate
{
    return [self.timelineMgr setTimelineOutputBitrate:bitrate];
}

- (NSInteger)outputBitrate
{
    return [self.timelineMgr outputBitrate];
}

- (QHVCEditError)setSpeed:(CGFloat)speed
{
    return [self.timelineMgr setTimelineSpeed:speed];
}

- (CGFloat)speed
{
    return [self.timelineMgr speed];
}

- (QHVCEditError)setVolume:(NSInteger)volume
{
    return [self.timelineMgr setTimelineVolume:volume];
}

- (NSInteger)volume
{
    return [self.timelineMgr volume];
}

- (QHVCEditError)setOutputPath:(NSString *)filePath
{
    return [self.timelineMgr setTimelineOutputPath:filePath];
}

- (NSString *)outputPath
{
    return [self.timelineMgr outputPath];
}

- (NSInteger)timelineDuration
{
    NSInteger duration = [self.timelineMgr duration];
    return duration;
}

#pragma mark - 轨道相关

- (QHVCEditError)timelineAppendTrack:(QHVCEditTrack *)track
{
    if (!track)
    {
        LogError(@"timeline appendTrack error, track is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* obj = [self.trackMgrs objectForKey:trackId];
    if (obj)
    {
        LogError(@"timeline appendTrack error, track already exist");
        return QHVCEditErrorAlreayExist;
    }
    
    QHVCEditError err = [self.timelineMgr appendTrack:track];
    if (err == QHVCEditErrorNoError)
    {
        void* timelineHandle = [self getTimelineHandle];
        QHVCEditTrackManager* trackMgr = [[QHVCEditTrackManager alloc] initWithTrack:track timelineHandle:timelineHandle];
        [self.trackMgrs setObject:trackMgr forKey:trackId];
    }

    return err;
}

- (QHVCEditError)timelineDeleteTrackById:(NSInteger)trackId
{
    NSNumber* trackIdNum = [NSNumber numberWithInteger:trackId];
    QHVCEditTrackManager* obj = [self.trackMgrs objectForKey:trackIdNum];
    if (!obj)
    {
        LogError(@"timeline deleteTrack error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [self.timelineMgr deleteTrackById:trackId];
    if (err == QHVCEditErrorNoError)
    {
        [self.trackMgrs removeObjectForKey:trackIdNum];
    }

    return QHVCEditErrorNoError;
}

- (QHVCEditTrack *)timelineGetTrackById:(NSInteger)trackId
{
    NSNumber* trackIdNum = [NSNumber numberWithInteger:trackId];
    QHVCEditTrackManager* obj = [self.trackMgrs objectForKey:trackIdNum];
    QHVCEditTrack* track = [obj getTrack];
    return track;
}

- (NSArray<QHVCEditTrack *>*)timelineGetTracks
{
    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:0];
    [self.trackMgrs enumerateKeysAndObjectsUsingBlock:^(NSNumber* key, QHVCEditTrackManager* obj, BOOL * _Nonnull stop)
    {
        QHVCEditTrack* track = [obj getTrack];
        [array addObject:track];
    }];
    return array;
}

#pragma mark - 特效相关

- (QHVCEditError)timelineAddEffect:(QHVCEditEffect *)effect
{
    if (!effect)
    {
        LogError(@"timeline addEffect error, effect is nil");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self.timelineMgr addEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditEffectManager* effectMgr = [self getEffectOfId:effect.effectId];
        [effectMgr setSuperObject:(QHVCEditObject *)[self getTimeline]];
    }
    
    return err;
}

- (QHVCEditError)timelineDeleteEffectById:(NSInteger)effectId
{
    QHVCEditError err = [self.timelineMgr deleteEffectById:effectId];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditEffectManager* effectMgr = [self getEffectOfId:effectId];
        [effectMgr setSuperObject:nil];
    }
    return err;
}

- (QHVCEditError)timelineUpdateEffect:(QHVCEditEffect *)effect
{
    if (!effect)
    {
        LogError(@"timeline addEffect error, effect is nil");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self.timelineMgr updateEffect:effect];
    return err;
}

- (QHVCEditEffect *)timelineGetEffectById:(NSInteger)effectId
{
    QHVCEditEffect* effect = [self.timelineMgr getEffectById:effectId];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)timelineGetEffects
{
    NSArray* array = [self.timelineMgr getEffects];
    return array;
}

@end
