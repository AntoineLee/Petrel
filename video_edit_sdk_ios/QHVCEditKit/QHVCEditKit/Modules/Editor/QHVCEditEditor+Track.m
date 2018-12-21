//
//  QHVCEditEditor+Track.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEditor+Track.h"
#import "QHVCEditLogger.h"
#import "QHVCEditConfig.h"
#import "QHVCEditEffect.h"
#import "QHVCEditEffectManager.h"
#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditEditor.h"
#import "QHVCEditTrack.h"
#import "QHVCEditTrackManager.h"
#import "QHVCEditTrackClip.h"
#import "QHVCEditTrackClipManager.h"

@implementation QHVCEditEditor(Track)

#pragma mark - 基础方法

- (QHVCEditError)track:(QHVCEditTrack *)track updateClipParams:(QHVCEditTrackClip *)clip
{
    if (!clip)
    {
        LogError(@"track updateClipParam error, clip is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track updateClipParam error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    NSNumber* clipIdNum = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipIdNum];
    if (!clipMgr)
    {
        LogError(@"track updateClipParams error, clip %@ havn't added", clipIdNum);
        return QHVCEditErrorNotExist;
    }

    QHVCEditError err = [trackMgr updateClipParams:clipMgr];
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track deleteClipById:(NSInteger)clipId
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track deleteClipById error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    NSNumber* clipIdNum = [NSNumber numberWithInteger:clipId];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipIdNum];
    if (!clipMgr)
    {
        LogError(@"track deleteClipById error, clip %@ havn't added", clipIdNum);
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr deleteClipById:clipId];
    if (err == QHVCEditErrorNoError)
    {
        [self.clipMgrs removeObjectForKey:clipIdNum];
    }
    
    return err;
}

- (QHVCEditTrackClip *)track:(QHVCEditTrack *)track getClipById:(NSInteger)clipId
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track getClipById error, track not exist");
        return nil;
    }
    
    QHVCEditTrackClipManager* clipMgr = [trackMgr getClipById:clipId];
    if (!clipMgr)
    {
        LogError(@"track getClipById error, clip %ld not exist", (long)clipId);
        return nil;
    }
    
    QHVCEditTrackClip* clip = [clipMgr getClip];
    return clip;
}

- (NSArray<QHVCEditTrackClip *>*)trackGetClips:(QHVCEditTrack *)track
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track trackGetClips error, track not exist");
        return nil;
    }
    
    NSArray<QHVCEditTrackClipManager *>* clipMgrs = [trackMgr getClips];
    NSMutableArray* clips = [[NSMutableArray alloc] initWithCapacity:0];
    [clipMgrs enumerateObjectsUsingBlock:^(QHVCEditTrackClipManager * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
    {
        QHVCEditTrackClip* clip = [obj getClip];
        if (clip)
        {
            [clips addObject:clip];
        }
    }];
    return clips;
}

- (NSInteger)trackDuration:(QHVCEditTrack *)track
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track get duration error, track not exist");
        return 0;
    }
    
    NSInteger duration = [trackMgr duration];
    return duration;
}

- (QHVCEditError)track:(QHVCEditTrack *)track setSpeed:(CGFloat)speed
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track setSpeed error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr setSpeed:speed];
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track setVolume:(NSInteger)volume
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track setVolume error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr setVolume:volume];
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track addEffect:(QHVCEditEffect *)effect
{
    if (!effect)
    {
        LogError(@"track addEffect error, effect is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track addEffect error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr addEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditEffectManager* effectMgr = [self getEffectOfId:effect.effectId];
        [effectMgr setSuperObject:track];
    }
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track deleteEffectById:(NSInteger)effectId
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track deleteEffectById error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr deleteEffectById:effectId];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditEffectManager* effectMgr = [self getEffectOfId:effectId];
        [effectMgr setSuperObject:nil];
    }
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track updateEffect:(QHVCEditEffect *)effect
{
    if (!effect)
    {
        LogError(@"track updateEffect error, effect is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track updateEffect error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr updateEffect:effect];
    return err;
}

- (QHVCEditEffect *)track:(QHVCEditTrack *)track getEffectById:(NSInteger)effectId
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track getEffectById error, track not exist");
        return nil;
    }
    
    QHVCEditEffect* effect = [trackMgr getEffectById:effectId];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)trackGetEffects:(QHVCEditTrack *)track
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track getEffects error, track not exist");
        return nil;
    }
    
    NSArray* effects = [trackMgr getEffects:track];
    return effects;
}

#pragma mark - 顺序轨道

- (QHVCEditError)track:(QHVCEditTrack *)track appendClip:(QHVCEditTrackClip *)clip
{
    if (!clip)
    {
        LogError(@"track appendClip error, clip is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track appendClip error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (clipMgr)
    {
        LogError(@"track appeendClip error, clip aready added to track of trackId %ld", (long)[clipMgr getTrackId]);
        return QHVCEditErrorAlreayExist;
    }
    
    void* timelineHandle = [self getTimelineHandle];
    clipMgr = [[QHVCEditTrackClipManager alloc] initWithClip:clip toTrack:[track trackId] ofTimelineHandle:timelineHandle];
    QHVCEditError err = [trackMgr appendClip:clipMgr];
    if (err == QHVCEditErrorNoError)
    {
        [self.clipMgrs setObject:clipMgr forKey:clipId];
    }
    
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track insertClip:(QHVCEditTrackClip *)clip atIndex:(NSInteger)index
{
    if (!clip)
    {
        LogError(@"track insertClip error, clip is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track insertClip error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (clipMgr)
    {
        LogError(@"track insertClip error, clip aready added to track of trackId %ld", (long)[clipMgr getTrackId]);
        return QHVCEditErrorAlreayExist;
    }
    
    void* timelineHandle = [self getTimelineHandle];
    clipMgr = [[QHVCEditTrackClipManager alloc] initWithClip:clip toTrack:[track trackId] ofTimelineHandle:timelineHandle];
    QHVCEditError err = [trackMgr insertClip:clipMgr atIndex:index];
    if (err == QHVCEditErrorNoError)
    {
        [self.clipMgrs setObject:clipMgr forKey:clipId];
    }

    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track moveClip:(NSInteger)fromIndex toIndex:(NSInteger)toIndex
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager *trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track insertClip error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr moveClip:fromIndex toIndex:toIndex];
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track deleteClipAtIndex:(NSInteger)index
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track deleteClipAtIndex error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditTrackClipManager* clipMgr = [trackMgr getClipAtIndex:index];
    if (!clipMgr)
    {
        LogError(@"track deleteClipAtIndex error, clip of index %ld havn't added", (long)index);
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr deleteClipAtIndex:index];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditTrackClip* clip = [clipMgr getClip];
        NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
        [self.clipMgrs removeObjectForKey:clipId];
    }
    
    return err;
}

- (QHVCEditTrackClip *)track:(QHVCEditTrack *)track getClipAtIndex:(NSInteger)index
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track getClipById error, track not exist");
        return nil;
    }
    
    QHVCEditTrackClipManager* clipMgr = [trackMgr getClipAtIndex:index];
    if (!clipMgr)
    {
        LogError(@"track getClipById error, clip of index %ld not exist", (long)index);
        return nil;
    }
    
    QHVCEditTrackClip* clip = [clipMgr getClip];
    return clip;
}

- (QHVCEditError)track:(QHVCEditTrack *)track
addVideoTransitionToIndex:(NSInteger)clipIndex
              duration:(NSInteger)durationMs
   videoTransitionName:(NSString *)transitionName
    easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track setVideoTransition error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    NSInteger transitionId = [self getTransitionIndex];
    QHVCEditError err = [trackMgr addVideoTransitionToIndex:clipIndex duration:durationMs videoTransitionName:transitionName transitionId:transitionId easingFunctionType:easingFunctionType];
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track
updateVideoTransitionAtIndex:(NSInteger)clipIndex
              duration:(NSInteger)durationMs
   videoTransitionName:(NSString *)transitionName
    easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track updateVideoTransition error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    if (durationMs <= 0)
    {
        LogError(@"track updateVideoTransition error, duration <= 0");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [trackMgr updateVideoTransitionAtIndex:clipIndex duration:durationMs videoTransitionName:transitionName easingFunctionType:easingFunctionType];
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track deleteVideoTransition:(NSInteger)clipIndex
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track setVideoTransition error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr deleteVideoTransition:clipIndex];
    return err;
}

#pragma mark - 画中画轨道

- (QHVCEditError)track:(QHVCEditTrack *)track addClip:(QHVCEditTrackClip *)clip atTime:(NSInteger)timeMs
{
    if (!clip)
    {
        LogError(@"track addClip error, clip is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track insertClip error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (clipMgr)
    {
        LogError(@"track insertClip error, clip aready added to track of trackId %ld", (long)[clipMgr getTrackId]);
        return QHVCEditErrorAlreayExist;
    }
    
    void* timelineHandle = [self getTimelineHandle];
    clipMgr = [[QHVCEditTrackClipManager alloc] initWithClip:clip toTrack:[track trackId] ofTimelineHandle:timelineHandle];
    QHVCEditError err = [trackMgr addClip:clipMgr atTime:timeMs];
    if (err == QHVCEditErrorNoError)
    {
        [self.clipMgrs setObject:clipMgr forKey:clipId];
    }
    
    return err;
}

- (QHVCEditError)track:(QHVCEditTrack *)track changeClipInsertTime:(NSInteger)timeMs clipId:(NSInteger)clipId
{
    NSNumber* trackId = [NSNumber numberWithInteger:[track trackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (!trackMgr)
    {
        LogError(@"track changeClipInsertTime error, track not exist");
        return QHVCEditErrorNotExist;
    }
    
    NSNumber* clipIdNum = [NSNumber numberWithInteger:clipId];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipIdNum];
    if (!clipMgr)
    {
        LogError(@"track changeClipInsertTime error, clip %@ havn't added", clipIdNum);
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [trackMgr changeClipInsertTime:timeMs clipId:clipId];
    return err;
}

@end
