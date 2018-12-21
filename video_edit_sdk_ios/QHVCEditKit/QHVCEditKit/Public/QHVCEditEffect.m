//
//  QHVCEditEffect.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEffect.h"
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditTrack.h"
#import "QHVCEditEditorManager.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditEffectManager.h"
#import <QHVCEffectKit/QHVCEffectBase+Process.h>

#pragma mark - 特效基类

@interface QHVCEditEffect ()
{
    void* _userData;
}

@property (nonatomic, retain) QHVCEditEditor* editor;
@property (nonatomic, assign) NSInteger effectId;
@property (nonatomic, assign) QHVCEditEffectType effectType; //特效类型
@property (nonatomic, retain) QHVCEditEffectManager* effectManager;
@property (nonatomic, assign) NSInteger acturalStartTime;
@property (nonatomic, assign) NSInteger acturalEndTime;

@end

@implementation QHVCEditEffect

- (void)dealloc
{
    [self.editor removeEffectOfId:self.effectId];
}

- (instancetype)initEffectWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        if (!timeline)
        {
            LogError(@"effect init failed, timeline is nil");
            return nil;
        }
        
        NSInteger timelineId = [timeline timelineId];
        self.editor = [[QHVCEditEditorManager sharedInstance] getEditor:timelineId];
        if (self.editor)
        {
            self.effectId = [self.editor getEffectIndex];
            self.effectManager = [[QHVCEditEffectManager alloc] initWithEffect:self];
            [self.editor addEffect:self.effectManager effectId:self.effectId];
        }
    }
    
    return self;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    return image;
}

- (QHVCEditObject *)superObj
{
    return [self.effectManager superObject];
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

@end

#pragma mark - 滤镜

@interface QHVCEditFilterEffect ()
@property (nonatomic, strong) QHVCEffectFilter* effect;
@end

@implementation QHVCEditFilterEffect

- (instancetype)initEffectWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super initEffectWithTimeline:timeline];
    if (self)
    {
        self.effect = [[QHVCEffectFilter alloc] init];
    }
    
    return self;
}

- (void)setFilePath:(NSString *)filePath
{
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        return;
    }
    
    if (![filePath isEqualToString:_filePath])
    {
        UIImage* image = [UIImage imageWithContentsOfFile:filePath];
        [self.effect setClutImage:[CIImage imageWithCGImage:image.CGImage]];
        _filePath = filePath;
    }
}

- (void)setIntensity:(CGFloat)intensity
{
    _intensity = intensity;
    [self.effect intensity];
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    return [self.effect processImage:image timestamp:timestamp];
}

@end

#pragma mark - 贴图

@interface QHVCEditStickerEffect ()
@property (nonatomic, strong) QHVCEffectSticker* effect;
@end

@implementation QHVCEditStickerEffect

- (instancetype)initEffectWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super initEffectWithTimeline:timeline];
    if (self)
    {
        self.effect = [[QHVCEffectSticker alloc] init];
    }
    
    return self;
}

- (void)setSticker:(UIImage *)sticker
{
    if (sticker != _sticker && [QHVCEditUtils stringIsNull:_stickerPath])
    {
        [self.effect setSticker:[CIImage imageWithCGImage:sticker.CGImage]];
         _sticker = sticker;
    }
}

- (void)setStickerPath:(NSString *)stickerPath
{
    if ([QHVCEditUtils stringIsNull:stickerPath])
    {
        return;
    }
    
    if (stickerPath != _stickerPath)
    {
        UIImage* sticker = [UIImage imageWithContentsOfFile:stickerPath];
        [self.effect setSticker:[CIImage imageWithCGImage:sticker.CGImage]];
        _stickerPath = stickerPath;
    }
}

- (void)setRenderX:(CGFloat)renderX
{
    [self.effect setRenderX:renderX];
    _renderX = renderX;
}

- (void)setRenderY:(CGFloat)renderY
{
    [self.effect setRenderY:renderY];
    _renderY = renderY;
}

- (void)setRenderWidth:(NSInteger)renderWidth
{
    [self.effect setRenderWidth:renderWidth];
    _renderWidth = renderWidth;
}

- (void)setRenderHeight:(NSInteger)renderHeight
{
    [self.effect setRenderHeight:renderHeight];
    _renderHeight = renderHeight;
}

- (void)setRenderRadian:(CGFloat)renderRadian
{
    [self.effect setRenderRadian:renderRadian];
    _renderRadian = renderRadian;
}

- (void)setVideoTransfer:(NSArray<QHVCEffectVideoTransferParam *> *)videoTransfer
{
    [self.effect setVideoTransfer:videoTransfer];
    _videoTransfer = videoTransfer;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    return [self.effect processImage:image timestamp:timestamp];
}

@end


#pragma mark - 视频过渡效果

@interface QHVCEditVideoTransferEffect ()
@property (nonatomic, strong) QHVCEffectVideoTransfer* effect;
@end

@implementation QHVCEditVideoTransferEffect

- (instancetype)initEffectWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super initEffectWithTimeline:timeline];
    if (self)
    {
        self.effect = [[QHVCEffectVideoTransfer alloc] init];
    }

    return self;
}

- (void)setVideoTransfer:(NSArray<QHVCEffectVideoTransferParam *> *)videoTransfer
{
    [self.effect setVideoTransfer:videoTransfer];
    _videoTransfer = videoTransfer;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    return [self.effect processImage:image timestamp:timestamp];
}

@end

#pragma mark - 图层混合效果

@interface QHVCEditMixEffect ()
@property (nonatomic, strong) QHVCEffectMix* effect;

@end

@implementation QHVCEditMixEffect

- (instancetype)initEffectWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super initEffectWithTimeline:timeline];
    if (self)
    {
        _intensity = 1.0;
        self.effect = [[QHVCEffectMix alloc] init];
    }
    
    return self;
}

- (QHVCEditEffectType)effectType
{
    return QHVCEditEffectTypeMix;
}

- (void)setIntensity:(CGFloat)intensity
{
    [self.effect setIntensity:intensity];
    _intensity = intensity;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    CIImage* outImage = image;
    if (!self.superObj)
    {
        return image;
    }
    
    if (self.superObj.objType != QHVCEditObjectTypeTrack)
    {
        LogWarn(@"QHVCEditMixEffect only can add to track");
        return image;
    }
    
    QHVCEditTrack* track = (QHVCEditTrack *)self.superObj;
    NSString* bgColor = @"FF000000";
    QHVCEditBgParams* bgParams = [track bgParams];
    if (!bgParams)
    {
        bgColor = [self.editor outputBgColor];
    }
    else
    {
        if (bgParams.mode == QHVCEditBgModeColor)
        {
            bgColor = bgParams.bgInfo;
        }
    }
    
    [self.effect setOutputSize:[self.editor outputSize]];
    [self.effect setBackgroundColor:bgColor];
    [self.effect setTopImage:self.targetImage];
    outImage =  [self.effect processImage:image timestamp:timestamp];
    return outImage;
}

@end

#pragma mark - 音频过渡效果

@implementation QHVCEditAudioTransferEffect

- (QHVCEditEffectType)effectType
{
    return QHVCEditEffectTypeAudio;
}

@end
