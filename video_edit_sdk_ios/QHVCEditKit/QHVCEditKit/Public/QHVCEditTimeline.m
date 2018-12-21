//
//  QHVCEditTimeline.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditTimeline.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditConfig.h"
#import "QHVCEditLogger.h"

#import "QHVCEditTrack.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditEditorManager.h"

@interface QHVCEditTimeline ()
{
    void* _userData;
}

@property (nonatomic, strong) QHVCEditEditor* editor;
@property (nonatomic, assign) NSInteger timelineId;

@end

@implementation QHVCEditTimeline

#pragma mark - 基础方法

- (QHVCEditObjectType)objType
{
    return QHVCEditObjectTypeTimeline;
}

- (instancetype)initTimeline
{
    self = [super init];
    if (self)
    {
        self.editor = [[QHVCEditEditor alloc] initWithTimeline:self];
        if (self.editor)
        {
            self.timelineId = [[QHVCEditConfig sharedInstance] getTimelineIndex];
            [[QHVCEditEditorManager sharedInstance] addEditor:self.editor editorId:self.timelineId];
        }
    }

    LogInfo(@"init timeline[%ld]", (long)self.timelineId);
    return self;
}

- (QHVCEditError)free
{
    LogInfo(@"free timeline[%ld]", (long)self.timelineId);
    [[QHVCEditEditorManager sharedInstance] deleteEditor:self.timelineId];
    return [self.editor free];
}

- (NSInteger)timelineId
{
    return _timelineId;
}

- (NSInteger)duration
{
    NSInteger duration = [self.editor timelineDuration];
    return duration;
}

- (QHVCEditError)setUserData:(void *)userData
{
    _userData = userData;
    return QHVCEditErrorNoError;
}

- (void *)userData
{
    return _userData;
}

#pragma mark - 输出设置

- (QHVCEditError)setOutputWidth:(NSInteger)width height:(NSInteger)height
{
    LogInfo(@"timeline[%ld] set output width[%ld] height[%ld]", (long)self.timelineId, (long)width, (long)height);
    return [self.editor setOutputWidth:width height:height];
}

- (CGSize)outputSize
{
    CGSize size = [self.editor outputSize];
    return size;
}

- (QHVCEditError)setOutputBgColor:(NSString *)bgColor
{
    LogInfo(@"timeline[%ld] set output bgColor[%@]", (long)self.timelineId, bgColor);
    return [self.editor setOutputBgColor:bgColor];
}

- (NSString *)outputBgColor
{
    NSString* color = [self.editor outputBgColor];
    return color;
}

- (QHVCEditError)setOutputFps:(NSInteger)fps
{
    LogInfo(@"timeline[%ld] set output fps[%ld]", (long)self.timelineId, fps);
    return [self.editor setOutputFps:fps];
}

- (NSInteger)outputFps
{
    NSInteger fps = [self.editor outputFps];
    return fps;
}

- (QHVCEditError)setOutputBitrate:(NSInteger)bitrate
{
    LogInfo(@"timeline[%ld] set output bitrate[%ld]", (long)self.timelineId, (long)bitrate);
    return [self.editor setOutputBitrate:bitrate];
}

- (NSInteger)outputBitrate
{
    NSInteger bitrate = [self.editor outputBitrate];
    return bitrate;
}

- (QHVCEditError)setSpeed:(CGFloat)speed
{
    LogInfo(@"timeline[%ld] set speed[%.1f]", (long)self.timelineId, speed);
    return [self.editor setSpeed:speed];
}

- (CGFloat)speed
{
    CGFloat speed = [self.editor speed];
    return speed;
}

- (QHVCEditError)setVolume:(NSInteger)volume
{
    LogInfo(@"timeline[%ld] set volume[%ld]", (long)self.timelineId, volume);
    return [self.editor setVolume:volume];
}

- (NSInteger)volume
{
    NSInteger volume = [self.editor volume];
    return volume;
}

- (QHVCEditError)setOutputPath:(NSString *)filePath
{
    LogInfo(@"timeline[%ld] set output path[%@]", (long)self.timelineId, filePath);
    return [self.editor setOutputPath:filePath];
}

- (NSString *)outputPath
{
    NSString* path = [self.editor outputPath];
    return path;
}

#pragma mark - 轨道相关

- (QHVCEditError)appendTrack:(QHVCEditTrack *)track
{
    QHVCEditError err = [self.editor timelineAppendTrack:track];
    LogInfo(@"timeline[%ld] append track[%ld] ret[%ld]", (long)self.timelineId, (long)track.trackId, (long)err);
    return err;
}

- (QHVCEditError)deleteTrackById:(NSInteger)trackId
{
    QHVCEditError err = [self.editor timelineDeleteTrackById:trackId];
    LogInfo(@"timeline[%ld] delete track[%ld] ret[%ld]", (long)self.timelineId, (long)trackId, (long)err);
    return err;
}

- (QHVCEditTrack *)getTrackById:(NSInteger)trackId
{
    QHVCEditTrack* track = [self.editor timelineGetTrackById:trackId];
    return track;
}

- (NSArray<QHVCEditTrack *>*)getTracks
{
    NSArray* array = [self.editor timelineGetTracks];
    return array;
}

#pragma mark - 时间线特效

- (QHVCEditError)addEffect:(QHVCEditEffect *)effect
{
    QHVCEditError err = [self.editor timelineAddEffect:effect];
    NSString* detail = [[QHVCEditConfig sharedInstance] printEffectDetail:effect];
    LogInfo(@"timeline[%ld] add effect ret[%ld], detail: %@", (long)self.timelineId, (long)err, detail);
    return err;
}

- (QHVCEditError)deleteEffectById:(NSInteger)effectId
{
    QHVCEditError err = [self.editor timelineDeleteEffectById:effectId];
    LogInfo(@"timeline[%ld] delete effec of id[%ld] ret[%ld]", (long)self.timelineId, (long)effectId, (long)err);
    return err;
}

- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect
{
    QHVCEditError err = [self.editor timelineUpdateEffect:effect];
    NSString* detail = [[QHVCEditConfig sharedInstance] printEffectDetail:effect];
    LogInfo(@"timeline[%ld] update effect ret[%ld], detail: %@", (long)self.timelineId, (long)err, detail);
    return err;
}

- (QHVCEditEffect *)getEffectById:(NSInteger)effectId
{
    QHVCEditEffect* effect = [self.editor timelineGetEffectById:effectId];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)getEffects
{
    NSArray* array = [self.editor timelineGetEffects];
    return array;
}

@end
