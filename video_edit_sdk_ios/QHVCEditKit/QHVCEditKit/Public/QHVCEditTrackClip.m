//
//  QHVCEditTrackClip.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditTrackClip.h"
#import "QHVCEditConfig.h"
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"

#import "QHVCEditEditorManager.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Clip.h"
#import "QHVCEditTimeline.h"

@interface QHVCEditTrackClip ()
{
    void* _userData;
}

@property (nonatomic, strong) QHVCEditEditor* editor;
@property (nonatomic, assign) QHVCEditTrackClipType clipType;
@property (nonatomic, strong) NSString* filePath;
@property (nonatomic, assign) NSInteger clipId;
@property (nonatomic, assign) NSInteger startTime;
@property (nonatomic, assign) NSInteger endTime;
@property (nonatomic, assign) CGFloat clipSpeed;
@property (nonatomic, assign) NSInteger clipVolume;
@property (nonatomic, assign) NSInteger clipPitch;
@property (nonatomic, assign) BOOL clipFlipX;
@property (nonatomic, assign) BOOL clipFlipY;
@property (nonatomic, assign) CGRect clipSourceRect;
@property (nonatomic, assign) CGFloat clipSourceRadian;
@property (nonatomic, retain) NSArray<QHVCEditSlowMotionVideoInfo *>* clipSlowMotionVideoInfo;

@end

@implementation QHVCEditTrackClip

#pragma mark - 基础方法

- (QHVCEditObjectType)objType
{
    return QHVCEditObjectTypeClip;
}

- (instancetype)initClipWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        if (!timeline)
        {
            LogError(@"clip init failed, timeline is nil");
            return nil;
        }
        
        NSInteger timelineId = [timeline timelineId];
        self.editor = [[QHVCEditEditorManager sharedInstance] getEditor:timelineId];
        if (self.editor)
        {
            self.clipId = [self.editor getClipIndex];
            _clipSpeed = QHVCEDIT_DEAFULT_SPEED;
            _clipVolume = QHVCEDIT_DEFAULT_VOLUME;
        }
        LogInfo(@"init clip[%ld] of timeline[%ld]", (long)self.clipId, (long)timelineId);
    }

    return self;
}

- (NSInteger)clipId
{
    return _clipId;
}

- (QHVCEditObject *)superObj
{
    return (QHVCEditObject *)[self.editor clipGetBelongsTrack:self];
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

#pragma mark - 素材属性

- (QHVCEditError)setFilePath:(NSString *)filePath type:(QHVCEditTrackClipType)type
{
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        LogError(@"clip setFilePath error, filePath is nil");
        return QHVCEditErrorParamError;
    }
    
    _filePath = filePath;
    _clipType = type;
    
    return QHVCEditErrorNoError;
}

- (NSString *)filePath
{
    return _filePath;
}

- (QHVCEditTrackClipType)clipType
{
    return _clipType;
}

- (QHVCEditError)setFileStartTime:(NSInteger)time
{
    _startTime = time;
    return QHVCEditErrorNoError;
}

- (NSInteger)fileStartTime
{
    return _startTime;
}

- (QHVCEditError)setFileEndTime:(NSInteger)time
{
    _endTime = time;
    return QHVCEditErrorNoError;
}

- (NSInteger)fileEndTime
{
    return _endTime;
}

- (NSInteger)insertTime
{
    NSInteger insertTime = [self.editor clipGetInsertTime:self];
    return insertTime;
}

- (QHVCEditError)setSpeed:(CGFloat)speed
{
    if (speed <= 0)
    {
        LogError(@"clip setSpeed error, speed <= 0");
        return QHVCEditErrorParamError;
    }
    
    _clipSpeed = speed;
    return QHVCEditErrorNoError;
}

- (CGFloat)speed
{
    return _clipSpeed;
}

- (QHVCEditError)setVolume:(NSInteger)volume
{
    if (volume < 0)
    {
        LogError(@"clip setVolume error, volume < 0");
        return QHVCEditErrorParamError;
    }
    
    if (volume > QHVCEDIT_MAX_VOLUME)
    {
        LogWarn(@"clip setVolume error, max volume is %d, auto set it to %d", QHVCEDIT_MAX_VOLUME, QHVCEDIT_MAX_VOLUME);
        volume = QHVCEDIT_MAX_VOLUME;
    }
    
    _clipVolume = volume;
    return QHVCEditErrorNoError;
}

- (NSInteger)volume
{
    return _clipVolume;
}

- (QHVCEditError)setPitch:(NSInteger)pitch
{
    if (pitch <= -12 || pitch >= 12)
    {
        LogError(@"clip setPitch error, pitch range is [-12~12]");
        return QHVCEditErrorParamError;
    }
    
    _clipPitch = pitch;
    return QHVCEditErrorNoError;
}

- (NSInteger)pitch
{
    return _clipPitch;
}

- (QHVCEditError)setFlipX:(BOOL)flipX
{
    _clipFlipX = flipX;
    return QHVCEditErrorNoError;
}

- (BOOL)flipX
{
    return _clipFlipX;
}

- (QHVCEditError)setFlipY:(BOOL)flipY
{
    _clipFlipY = flipY;
    return QHVCEditErrorNoError;
}

- (BOOL)flipY
{
    return _clipFlipY;
}

- (QHVCEditError)setSourceX:(CGFloat)x
                    sourceY:(CGFloat)y
                sourceWidth:(NSInteger)width
               sourceHeight:(NSInteger)height
{
    if (width <= 0)
    {
        LogError(@"clip setSourceRect error, width <= 0");
        return QHVCEditErrorParamError;
    }
    
    if (height <= 0)
    {
        LogError(@"clip setSourceRect error, height <= 0");
        return QHVCEditErrorParamError;
    }
    
    _clipSourceRect = CGRectMake(x, y, width, height);
    return QHVCEditErrorNoError;
}

- (CGRect)sourceRect
{
    return _clipSourceRect;
}

- (QHVCEditError)setSourceRadian:(CGFloat)radian
{
    _clipSourceRadian = radian;
    return QHVCEditErrorNoError;
}

- (CGFloat)sourceRadian
{
    return _clipSourceRadian;
}

- (QHVCEditError)setSlowMotionVideoInfo:(NSArray<QHVCEditSlowMotionVideoInfo *> *)slowMotionVideoInfos
{
    _clipSlowMotionVideoInfo = slowMotionVideoInfos;
    return QHVCEditErrorNoError;
}

- (NSArray<QHVCEditSlowMotionVideoInfo *>*)slowMotionVideoInfo
{
    return _clipSlowMotionVideoInfo;
}

- (NSInteger)duration
{
    return [self.editor clipDuration:self];
}

#pragma mark - 素材特效

- (QHVCEditError)addEffect:(QHVCEditEffect *)effect
{
    QHVCEditError err = [self.editor clip:self addEffect:effect];
    NSString* detail = [[QHVCEditConfig sharedInstance] printEffectDetail:effect];
    LogInfo(@"clip[%ld] add effect ret[%ld], detail: %@", (long)self.clipId, (long)err, detail);
    return err;
}

- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect
{
    QHVCEditError err = [self.editor clip:self updateEffect:effect];
    NSString* detail = [[QHVCEditConfig sharedInstance] printEffectDetail:effect];
    LogInfo(@"clip[%ld] update effect ret[%ld], detail: %@", (long)self.clipId, (long)err, detail);
    return err;
}

- (QHVCEditError)deleteEffectById:(NSInteger)effectId
{
    QHVCEditError err = [self.editor clip:self deleteEffectById:effectId];
    LogInfo(@"clip[%ld] delete effect of id[%ld] ret[%ld]", (long)self.clipId, (long)effectId, (long)err);
    return err;
}

- (QHVCEditEffect *)getEffectById:(NSInteger)effectId
{
    QHVCEditEffect* effect = [self.editor clip:self getEffectById:effectId];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)getEffects
{
    NSArray* array = [self.editor clipGetEffects:self];
    return array;
}

@end
