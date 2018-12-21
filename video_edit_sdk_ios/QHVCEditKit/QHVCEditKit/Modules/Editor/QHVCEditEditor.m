//
//  QHVCEditEditor.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/25.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEditor.h"
#import "QHVCEditConfig.h"
#import "QHVCEditTrack.h"
#import "QHVCEditEffect.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditTimelineManager.h"
#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditEditor+Track.h"

@interface QHVCEditEditor ()

//当前分配的最大id
@property (nonatomic, assign) NSInteger trackIndex;
@property (nonatomic, assign) NSInteger clipIndex;
@property (nonatomic, assign) NSInteger effectIndex;
@property (nonatomic, assign) NSInteger transitionIndex;

//timeline对象
@property (nonatomic,   weak) QHVCEditTimeline* timeline;

@end

@implementation QHVCEditEditor

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        self.timeline = timeline;
        self.timelineMgr = [[QHVCEditTimelineManager alloc] initWithTimeline:timeline];
        if (self.timelineMgr)
        {
            [self initParams];
        }
    }
    
    return self;
}

- (QHVCEditTimeline *)getTimeline
{
    return self.timeline;
}

- (void *)getTimelineHandle
{
    return [self.timelineMgr getTimelineHandle];
}

- (QHVCEditError)free
{
    QHVCEditError err = [self.timelineMgr freeTimeline];
    [self.editorEffects removeAllObjects];
    self.editorEffects = nil;
    
    [self.trackMgrs removeAllObjects];
    self.trackMgrs = nil;
    
    [self.clipMgrs removeAllObjects];
    self.clipMgrs = nil;
    
    return err;
}

- (NSInteger)getTrackIndex
{
    self.trackIndex ++;
    return self.trackIndex;
}

- (NSInteger)getClipIndex
{
    self.clipIndex ++;
    return self.clipIndex;
}

- (NSInteger)getEffectIndex
{
    self.effectIndex ++;
    return self.effectIndex;
}

- (NSInteger)getTransitionIndex
{
    self.transitionIndex ++;
    return self.transitionIndex;
}

- (void)initParams
{
    self.outputPath = [[[QHVCEditConfig sharedInstance] cacheDirectory] stringByAppendingString:@"output.mp4"];
    self.trackMgrs = [[NSMutableDictionary alloc] initWithCapacity:0];
    self.clipMgrs = [[NSMutableDictionary alloc] initWithCapacity:0];
    self.editorEffects = [[NSMutableDictionary alloc] initWithCapacity:0];
}

#pragma mark - effect 查找表

- (void)addEffect:(QHVCEditEffectManager *)effect effectId:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    [self.editorEffects setObject:effect forKey:effectIdNum];
}

- (void)removeEffectOfId:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    [self.editorEffects removeObjectForKey:effectIdNum];
}

- (QHVCEditEffectManager *)getEffectOfId:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    QHVCEditEffectManager* effect = [self.editorEffects objectForKey:effectIdNum];
    return effect;
}

@end
