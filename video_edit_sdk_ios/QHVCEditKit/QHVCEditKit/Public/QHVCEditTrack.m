//
//  QHVCEditTrack.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditTrack.h"
#import "QHVCEditConfig.h"
#import "QHVCEditLogger.h"

#import "QHVCEditEditorManager.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Track.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditTrackClip.h"

@interface QHVCEditTrack ()
{
    void* _userData;
}

@property (nonatomic, strong) QHVCEditEditor* editor;
@property (nonatomic, assign) QHVCEditTrackType trackType;
@property (nonatomic, assign) NSInteger trackId;
@property (nonatomic, assign) NSInteger trackZOrder;
@property (nonatomic, assign) CGFloat trackSpeed;
@property (nonatomic, assign) NSInteger trackVolume;

@property (nonatomic, assign) CGRect trackRenderRect;
@property (nonatomic, assign) CGFloat trackRenderRadian;
@property (nonatomic, assign) CGFloat trackFillMode;
@property (nonatomic, strong) QHVCEditBgParams* trackBgParams;
@end

@implementation QHVCEditTrack

#pragma mark - 基础方法

- (QHVCEditObjectType)objType
{
    return QHVCEditObjectTypeTrack;
}

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline type:(QHVCEditTrackType)trackType
{
    self = [super init];
    if (self)
    {
        if (!timeline)
        {
            LogError(@"track init failed, timeline is nil");
            return nil;
        }
        
        NSInteger timelineId = [timeline timelineId];
        self.editor = [[QHVCEditEditorManager sharedInstance] getEditor:timelineId];
        if (self.editor)
        {
            self.trackType = trackType;
            self.trackId = [self.editor getTrackIndex];
            self.trackSpeed = QHVCEDIT_DEAFULT_SPEED;
            self.trackVolume = QHVCEDIT_DEFAULT_VOLUME;
        }
        
        LogInfo(@"init track[%ld] of timeline[%ld]", (long)self.trackId, (long)timelineId);
    }
    
    return self;
}

- (QHVCEditTrackType)trackType
{
    return _trackType;
}

- (QHVCEditTrackArrangement)trackArrangement
{
    return QHVCEditTrackArrangementSequence;
}

- (NSInteger)trackId
{
    return _trackId;
}

- (NSInteger)duration
{
    return [self.editor trackDuration:self];
}

- (QHVCEditObject *)superObj
{
    return [self.editor getTimeline];
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

- (QHVCEditError)setZOrder:(NSInteger)zOrder
{
    LogInfo(@"track[%ld] set zorder[%ld]", (long)self.trackId, (long)zOrder);
    _trackZOrder = zOrder;
    return QHVCEditErrorNoError;
}

- (NSInteger)zOrder
{
    return _trackZOrder;
}

- (QHVCEditError)setSpeed:(CGFloat)speed
{
    QHVCEditError err = [self.editor track:self setSpeed:speed];
    if (err == QHVCEditErrorNoError)
    {
        _trackSpeed = speed;
    }
    
    LogInfo(@"track[%ld] set speed[%.1f] ret[%ld]", (long)self.trackId, speed, (long)err);
    return err;
}

- (CGFloat)speed
{
    return _trackSpeed;
}

- (QHVCEditError)setVolume:(NSInteger)volume
{
    QHVCEditError err = [self.editor track:self setVolume:volume];
    if (err == QHVCEditErrorNoError)
    {
        _trackVolume = volume;
    }
    
    LogInfo(@"track[%ld] set volume[%ld] ret[%ld]", (long)self.trackId, (long)volume, (long)err);
    return err;
}

- (NSInteger)volume
{
    return self.trackVolume;
}

- (QHVCEditError)setRenderX:(CGFloat)x
                    renderY:(CGFloat)y
                renderWidth:(NSInteger)width
               renderHeight:(NSInteger)height
{
    if (width <= 0)
    {
        LogError(@"clip setRenderRect error, width <= 0");
        return QHVCEditErrorParamError;
    }
    
    if (height <= 0)
    {
        LogError(@"clip setRenderRect error, height <= 0");
        return QHVCEditErrorParamError;
    }
    
    _trackRenderRect = CGRectMake(x, y, width, height);
    LogInfo(@"track[%ld] set reder rect(%.1f, %.1f, %ld, %ld) ret[%ld]",
            (long)self.trackId,
            x, y, (long)width, (long)height,
            (long)QHVCEditErrorNoError);
    return QHVCEditErrorNoError;
}

- (CGRect)renderRect
{
    return _trackRenderRect;
}

- (QHVCEditError)setRenderRadian:(CGFloat)renderRadian
{
    LogInfo(@"track[%ld] set render radian[%.1f] ret[%ld]", (long)self.trackId, renderRadian, (long)QHVCEditErrorNoError);
    _trackRenderRadian = renderRadian;
    return QHVCEditErrorNoError;
}

- (CGFloat)renderRadian
{
    return _trackRenderRadian;
}

- (QHVCEditError)setFillMode:(QHVCEditFillMode)mode
{
    LogInfo(@"track[%ld] set fill mode[%ld] ret[%ld]", (long)self.trackId, (long)mode, (long)QHVCEditErrorNoError);
    _trackFillMode = mode;
    return QHVCEditErrorNoError;
}

- (QHVCEditFillMode)fillMode
{
    return _trackFillMode;
}

- (QHVCEditError)setBgParams:(QHVCEditBgParams *)bgParams
{
    LogInfo(@"track[%ld] set bgParam ret[%ld], mode[%ld] info[%@]",
            (long)self.trackId,
            (long)QHVCEditErrorNoError,
            (long)bgParams.mode,
            bgParams.bgInfo);
    _trackBgParams = bgParams;
    return QHVCEditErrorNoError;
}

- (QHVCEditBgParams *)bgParams
{
    return _trackBgParams;
}

#pragma mark - 文件素材相关

- (QHVCEditError)updateClipParams:(QHVCEditTrackClip *)clip
{
    QHVCEditError err = [self.editor track:self updateClipParams:clip];
    NSString* detail = [self printClipInfo:clip];
    LogInfo(@"track[%ld] update clip params ret[%ld], detail: %@", (long)self.trackId, (long)err, detail);
    return err;
}

- (QHVCEditError)deleteClipById:(NSInteger)clipId
{
    QHVCEditError err = [self.editor track:self deleteClipById:clipId];
    LogInfo(@"track[%ld] delete clip of id[%ld] ret[%ld]", (long)self.trackId, (long)clipId, (long)err);
    return err;
}

- (QHVCEditTrackClip *)getClipById:(NSInteger)clipId
{
    QHVCEditTrackClip* clip = [self.editor track:self getClipById:clipId];
    return clip;
}

- (NSArray<QHVCEditTrackClip *>*)getClips
{
    NSArray* array = [self.editor trackGetClips:self];
    return array;
}

- (NSString *)printClipInfo:(QHVCEditTrackClip *)clip
{
    NSInteger clipId = [clip clipId];
    NSString* filePath = [clip filePath];
    QHVCEditTrackClipType clipType = [clip clipType];
    NSInteger startTime = [clip fileStartTime];
    NSInteger endTime = [clip fileEndTime];
    NSInteger insertTime = [clip insertTime];
    CGFloat speed = [clip speed];
    NSInteger volume = [clip volume];
    BOOL flipX = [clip flipX];
    BOOL flipY = [clip flipY];
    CGRect sourceRect = [clip sourceRect];
    CGFloat sourceRadian = [clip sourceRadian];
    
    NSString* info = [NSString stringWithFormat:
                      @"clipId[%ld] filePath[%@] clipType[%lu] startTime[%ld] endTime[%ld] insertTime[%ld] speed[%.1f] volume[%ld] flipX[%@] flipY[%@] sourceRect(%.1f, %.1f, %.1f, %.1f) sourceRadian[%.1f]",
                      (long)clipId,
                      filePath,
                      (long)clipType,
                      (long)startTime,
                      (long)endTime,
                      (long)insertTime,
                      speed,
                      (long)volume,
                      flipX ? @"yes":@"no",
                      flipY ? @"yes":@"no",
                      sourceRect.origin.x,
                      sourceRect.origin.y,
                      sourceRect.size.width,
                      sourceRect.size.height,
                      sourceRadian];
    return info;
}

#pragma mark - 轨道特效

- (QHVCEditError)addEffect:(QHVCEditEffect *)effect
{
    QHVCEditError err = [self.editor track:self addEffect:effect];
    NSString* detail = [[QHVCEditConfig sharedInstance] printEffectDetail:effect];
    LogInfo(@"track[%ld] add effect ret[%ld], detail: %@", (long)self.trackId, (long)err, detail);
    return err;
}

- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect
{
    QHVCEditError err = [self.editor track:self updateEffect:effect];
    NSString* detail = [[QHVCEditConfig sharedInstance] printEffectDetail:effect];
    LogInfo(@"track[%ld] update effect ret[%ld], detail: %@", (long)self.trackId, (long)err, detail);
    return err;
}

- (QHVCEditError)deleteEffectById:(NSInteger)effectId
{
    QHVCEditError err = [self.editor track:self deleteEffectById:effectId];
    LogInfo(@"track[%ld] delete effect of id[%ld] ret[%ld]", (long)self.trackId, (long)effectId, (long)err);
    return err;
}

- (QHVCEditEffect *)getEffectById:(NSInteger)effectId
{
    QHVCEditEffect* effect = [self.editor track:self getEffectById:effectId];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)getEffects
{
    NSArray* effects = [self.editor trackGetEffects:self];
    return effects;
}

@end

#pragma mark - 有序轨道

@implementation QHVCEditSequenceTrack

- (QHVCEditTrackArrangement)trackArrangement
{
    return QHVCEditTrackArrangementSequence;
}

- (QHVCEditError)appendClip:(QHVCEditTrackClip *)clip
{
    QHVCEditError err = [self.editor track:self appendClip:clip];
    NSString* detail = [self printClipInfo:clip];
    LogInfo(@"track[%ld] append clip ret[%ld], detail: %@", (long)self.trackId, (long)err, detail);
    return err;
}

- (QHVCEditError)insertClip:(QHVCEditTrackClip *)clip atIndex:(NSInteger)index
{
    QHVCEditError err = [self.editor track:self insertClip:clip atIndex:index];
    NSString* detail = [self printClipInfo:clip];
    LogInfo(@"track[%ld] insert clip at index[%ld] ret[%ld], detail: %@", (long)self.trackId, (long)index, (long)err, detail);
    return err;
}

- (QHVCEditError)moveClip:(NSInteger)fromIndex toIndex:(NSInteger)toIndex
{
    QHVCEditError err = [self.editor track:self moveClip:fromIndex toIndex:toIndex];
    LogInfo(@"track[%ld] move clip from index[%ld] to index[%ld] ret[%ld]",
            (long)self.trackId,
            (long)fromIndex,
            (long)toIndex,
            (long)err);
    return err;
}

- (QHVCEditError)deleteClipAtIndex:(NSInteger)index
{
    QHVCEditError err = [self.editor track:self deleteClipAtIndex:index];
    LogInfo(@"track[%ld] delete clip at index[%ld] ret[%ld]", (long)self.trackId, (long)index, (long)err);
    return err;
}

- (QHVCEditTrackClip *)getClipAtIndex:(NSInteger)index
{
    QHVCEditTrackClip* clip = [self.editor track:self getClipAtIndex:index];
    return clip;
}

#pragma mark - 转场

- (QHVCEditError)addVideoTransitionToIndex:(NSInteger)clipIndex
                                  duration:(NSInteger)duration
                       videoTransitionName:(NSString *)transitionName
                        easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    QHVCEditError err = [self.editor track:self
                 addVideoTransitionToIndex:clipIndex
                                  duration:duration
                       videoTransitionName:transitionName
                        easingFunctionType:easingFunctionType];
    LogInfo(@"track[%ld] add transition to index[%ld] ret[%ld] duration[%ld] name[%@] easingFunction[%ld]",
            (long)self.trackId,
            (long)clipIndex,
            (long)err,
            (long)duration,
            transitionName,
            (long)easingFunctionType);
    return err;
}

- (QHVCEditError)deleteVideoTransition:(NSInteger)clipIndex
{
    QHVCEditError err = [self.editor track:self deleteVideoTransition:clipIndex];
    LogInfo(@"track[%ld] delete transition of index[%ld] ret[%ld]", (long)self.trackId, (long)clipIndex, (long)err);
    return err;
}

- (QHVCEditError)updateVideoTransitionAtIndex:(NSInteger)clipIndex
                                     duration:(NSInteger)duration
                          videoTransitionName:(NSString *)transitionName
                           easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    QHVCEditError err = [self.editor track:self
                         updateVideoTransitionAtIndex:clipIndex
                                  duration:duration
                       videoTransitionName:transitionName
                        easingFunctionType:easingFunctionType];
    LogInfo(@"track[%ld] update transition at index[%ld] ret[%ld] duration[%ld] name[%@] easingFunction[%ld]",
            (long)self.trackId,
            (long)clipIndex,
            (long)err,
            (long)duration,
            transitionName,
            (long)easingFunctionType);
    return err;
}

@end

#pragma mark - 画中画轨道

@implementation QHVCEditOverlayTrack

- (QHVCEditTrackArrangement)trackArrangement
{
    return QHVCEditTrackArrangementOverlay;
}

- (QHVCEditError)addClip:(QHVCEditTrackClip *)clip atTime:(NSInteger)time
{
    QHVCEditError err = [self.editor track:self addClip:clip atTime:time];
    NSString* detail = [self printClipInfo:clip];
    LogInfo(@"track[%ld] add clip at time[%ld] ret[%ld], detail: %@", (long)self.trackId, (long)time, (long)err, detail);
    return err;
}

- (QHVCEditError)changeClipInsertTime:(NSInteger)time clipId:(NSInteger)clipId
{
    QHVCEditError err = [self.editor track:self changeClipInsertTime:time clipId:clipId];
    LogInfo(@"track[%ld] change clip[%ld] insert time to[%ld] ret[%ld]", (long)self.trackId, (long)clipId, (long)time, (long)err);
    return err;
}

@end
