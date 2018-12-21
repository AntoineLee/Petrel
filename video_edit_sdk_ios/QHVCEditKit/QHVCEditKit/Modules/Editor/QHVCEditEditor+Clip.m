//
//  QHVCEditEditor+Clip.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEditor+Clip.h"
#import "QHVCEditLogger.h"
#import "QHVCEditConfig.h"
#import "QHVCEditEffect.h"
#import "QHVCEditEditor.h"
#import "QHVCEditTrack.h"
#import "QHVCEditTrackManager.h"
#import "QHVCEditTrackClip.h"
#import "QHVCEditTrackClipManager.h"
#import "QHVCEditEffectManager.h"

@implementation QHVCEditEditor(TrackClip)

- (QHVCEditTrackClipManager *)clipGetManager:(QHVCEditTrackClip *)clip
{
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    return clipMgr;
}

- (QHVCEditTrack *)clipGetBelongsTrack:(QHVCEditTrackClip *)clip
{
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        return nil;
    }
    
    QHVCEditTrack* track = nil;
    NSNumber* trackId = [NSNumber numberWithInteger:[clipMgr getTrackId]];
    QHVCEditTrackManager* trackMgr = [self.trackMgrs objectForKey:trackId];
    if (trackMgr)
    {
        track = [trackMgr getTrack];
    }
    
    return track;
}

- (NSInteger)clipGetInsertTime:(QHVCEditTrackClip *)clip
{
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        return -1;
    }
    
    NSInteger insertTime = [clipMgr insertTime];
    return insertTime;
}

- (NSInteger)clipDuration:(QHVCEditTrackClip *)clip
{
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        return 0;
    }
    
    NSInteger duration = [clipMgr duration];
    return duration;
}

- (QHVCEditError)clip:(QHVCEditTrackClip *)clip addEffect:(QHVCEditEffect *)effect
{
    if (!effect)
    {
        LogError(@"clip addEffect error, effect is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        LogError(@"clip addEffect error, clip not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [clipMgr addEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditEffectManager* effectMgr = [self getEffectOfId:effect.effectId];
        [effectMgr setSuperObject:clip];
    }
    return err;
}

- (QHVCEditError)clip:(QHVCEditTrackClip *)clip updateEffect:(QHVCEditEffect *)effect
{
    if (!effect)
    {
        LogError(@"clip updateEffect error, effect is nil");
        return QHVCEditErrorParamError;
    }
    
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        LogError(@"clip updateEffect error, clip not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [clipMgr updateEffect:effect];
    return err;
}

- (QHVCEditError)clip:(QHVCEditTrackClip *)clip deleteEffectById:(NSInteger)effectId
{
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        LogError(@"clip deleteEffectById %ld error, clip not exist", (long)effectId);
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [clipMgr deleteEffectById:effectId];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditEffectManager* effectMgr = [self getEffectOfId:effectId];
        [effectMgr setSuperObject:nil];
    }
    return err;
}

- (QHVCEditEffect *)clip:(QHVCEditTrackClip *)clip getEffectById:(NSInteger)effectId
{
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        LogError(@"clip getEffectById %ld error, clip not exist", (long)effectId);
        return nil;
    }
    
    QHVCEditEffect* effect = [clipMgr getEffectById:effectId];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)clipGetEffects:(QHVCEditTrackClip *)clip
{
    NSNumber* clipId = [NSNumber numberWithInteger:[clip clipId]];
    QHVCEditTrackClipManager* clipMgr = [self.clipMgrs objectForKey:clipId];
    if (!clipMgr)
    {
        LogError(@"clip getEffects error, clip not exist");
        return nil;
    }
    
    NSArray<QHVCEditEffect *>* effects = [clipMgr getEffects];
    return effects;
}

@end
