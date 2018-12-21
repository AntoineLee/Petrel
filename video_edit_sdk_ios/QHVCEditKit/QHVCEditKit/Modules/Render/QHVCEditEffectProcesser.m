//
//  QHVCEditEffectProcesser.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/9.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEffectProcesser.h"
#import <QHVCEffectKit/QHVCEffectBase+Process.h>
#import "QHVCEditMacroDefs.h"
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditEffect.h"
#import "QHVCEditCommonDef.h"
#import "QHVCEditEffect.h"
#import "QHVCEditEffectManager.h"
#import "QHVCEditCIFilterStack.h"
#import "QHVCEditCIRotateFilter.h"

@interface QHVCEditEffectProcesser () <QHVCEditCIOutputProtocol>

@property (nonatomic, retain) QHVCEditEditor* editor;
@property (nonatomic, retain) QHVCEditCIFilterStack* stack;
@property (nonatomic, retain) QHVCEditCIRotateFilter* rotateFilter;
@property (nonatomic, retain) NSMutableDictionary* effectDict;
@property (nonatomic, retain) NSMutableDictionary* filterDict;
@property (nonatomic,   copy) QHVCEditEffectProcesserComplete effectProcessCompleteBlock;

@end

@implementation QHVCEditEffectProcesser

#pragma mark - Public Methods

- (instancetype)initWithEditor:(QHVCEditEditor *)editor
{
    self = [super init];
    if (self)
    {
        self.editor = editor;
        self.stack = [[QHVCEditCIFilterStack alloc] initWithOutput:self];
        self.rotateFilter = [[QHVCEditCIRotateFilter alloc] init];
        self.effectDict = [[NSMutableDictionary alloc] initWithCapacity:0];
        self.filterDict = [[NSMutableDictionary alloc] initWithCapacity:0];
    }

    return self;
}

- (void)processClipFrame:(QHVCEditRenderClip *)clip
               timestamp:(NSInteger)timestampMs
                complete:(QHVCEditEffectProcesserComplete)complete
{
    if (!clip.inputImage)
    {
        LogError(@"render frame error, inputImage is nil");
        return;
    }
    
    //frame 做裁剪、旋转处理
    CGFloat sourceRadian = clip.sourceRotate*0.25*M_PI*2;
    [self.rotateFilter setBgMode:(QHVCEditCIRotateFilterBGMode)clip.bgMode];
    [self.rotateFilter setBgColor:clip.bgColor];
    [self.rotateFilter setFillMode:(QHVCEditCIRotateFilterFillMode)clip.fillMode];
    [self.rotateFilter setRenderRect:clip.renderRect];
    [self.rotateFilter setSourceRect:clip.sourceRect];
    [self.rotateFilter setSourceRadian:sourceRadian];
    [self.rotateFilter setContentRadian:clip.frameRadian];
    [self.rotateFilter setFlipX:clip.flipX];
    [self.rotateFilter setFilpY:clip.flipY];
    [self.rotateFilter setOutputSize:clip.outputSize];
    [self.rotateFilter setPreviewRadian:clip.previewRadian];
    CIImage* outputImage = [self.rotateFilter processImage:clip.inputImage timestamp:timestampMs];
    QHVCEDIT_SAFE_BLOCK(complete, outputImage);
}

- (void)processEffect:(CIImage *)inputImage
               effect:(NSArray<QHVCEditRenderEffect *> *)effects
            timestamp:(NSInteger)timestampMs
             complete:(QHVCEditEffectProcesserComplete)complete
{
    if (!inputImage)
    {
        LogWarn(@"render frame error, process effect, inputImage is nil");
        QHVCEDIT_SAFE_BLOCK(complete, inputImage);
        return;
    }
    
    if ([effects count] <= 0)
    {
        QHVCEDIT_SAFE_BLOCK(complete, inputImage);
        return;
    }
    
    self.effectProcessCompleteBlock = complete;
    [self preProcessEffect:effects timestamp:timestampMs];
    [self.stack processImage:inputImage timestamp:timestampMs];
}

- (void)processImage:(CIImage *)image timestampMs:(NSInteger)timestampMs userData:(id)userData;
{
    QHVCEDIT_SAFE_BLOCK(self.effectProcessCompleteBlock, image);
    self.effectProcessCompleteBlock = nil;
}

#pragma mark - Effect Methods

- (void)preProcessEffect:(NSArray<QHVCEditRenderEffect *> *)effects timestamp:(NSInteger)timestampMs
{
    NSMutableDictionary* effectIds = [[NSMutableDictionary alloc] initWithCapacity:0];
    NSMutableArray* effectArray = [[NSMutableArray alloc] initWithCapacity:0];
    
    QHVCEDIT_WEAK_SELF
    [effects enumerateObjectsUsingBlock:^(QHVCEditRenderEffect * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
    {
        QHVCEDIT_STRONG_SELF
        if (timestampMs > obj.endTime)
        {
            //结束指令
            [self removeEffect:obj];
        }
        else if (timestampMs >= obj.startTime)
        {
            //添加指令
            if (obj)
            {
                NSNumber* effectId = [NSNumber numberWithInteger:obj.effectId];
                [effectIds setObject:obj forKey:effectId];
                [effectArray addObject:obj];
            }
        }
    }];
    
    [self checkIfEffectNeedRemoved:effectIds];
    [effectArray enumerateObjectsUsingBlock:^(QHVCEditRenderEffect* effect, NSUInteger idx, BOOL * _Nonnull stop)
    {
        QHVCEDIT_STRONG_SELF
        //开始特效，保证有序
        [self addEffect:effect timestamp:timestampMs];
    }];
}

- (void)checkIfEffectNeedRemoved:(NSMutableDictionary *)effectIds
{
    NSMutableDictionary* removeDict = [[NSMutableDictionary alloc] initWithCapacity:0];
    [self.effectDict enumerateKeysAndObjectsUsingBlock:^(NSNumber* _Nonnull effectIdNum, QHVCEditRenderEffect* obj, BOOL * _Nonnull stop)
    {
        QHVCEditRenderEffect* effect = [effectIds objectForKey:effectIdNum];
        if (!effect)
        {
            [removeDict setObject:obj forKey:effectIdNum];
        }
    }];
    
    QHVCEDIT_WEAK_SELF
    [removeDict enumerateKeysAndObjectsUsingBlock:^(NSNumber* _Nonnull effectIdNum, QHVCEditRenderEffect* obj, BOOL * _Nonnull stop)
    {
        QHVCEDIT_STRONG_SELF
        [self removeEffect:obj];
    }];
}

- (void)addEffect:(QHVCEditRenderEffect *)renderEffect timestamp:(NSInteger)timestampMs
{
    QHVCEditEffect* item = [self getEffectOfDict:renderEffect.effectId];
    if (item)
    {
        [self updateEffect:renderEffect timestamp:timestampMs];
        return;
    }
    
    QHVCEditEffectManager* effectManager = [self.editor getEffectOfId:renderEffect.effectId];
    QHVCEditEffect* effect = [effectManager getEffect];
    if (effect)
    {
        [effect setEffectStartTime:renderEffect.startTime];
        [effect setEffectEndTime:renderEffect.endTime];
        [self saveEffectToDict:effect];
        [self.stack addFilter:effect];
    }
}

- (void)updateEffect:(QHVCEditRenderEffect *)renderEffect timestamp:(NSInteger)timestampMs
{
    QHVCEditEffectManager* effectManager = [self.editor getEffectOfId:renderEffect.effectId];
    QHVCEditEffect* effect = [effectManager getEffect];
    if (effect)
    {
        [effect setEffectStartTime:renderEffect.startTime];
        [effect setEffectEndTime:renderEffect.endTime];
    }
}

- (void)removeEffect:(QHVCEditRenderEffect *)renderEffect
{
    QHVCEditEffectManager* effectManager = [self.editor getEffectOfId:renderEffect.effectId];
    QHVCEditEffect* effect = [effectManager getEffect];
    if (effect)
    {
        [self.stack removeFilter:effect];
        [self removeEffectFromDict:effect];
    }
}

- (void)saveEffectToDict:(QHVCEditEffect *)effect
{
    NSNumber* effectId = [NSNumber numberWithInteger:effect.effectId];
    [self.effectDict setObject:effect forKey:effectId];
}

- (void)removeEffectFromDict:(QHVCEditEffect *)effect
{
    NSNumber* effectId = [NSNumber numberWithInteger:effect.effectId];
    [self.effectDict removeObjectForKey:effectId];
}

- (QHVCEditEffect *)getEffectOfDict:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    QHVCEditEffect* effect = [self.effectDict objectForKey:effectIdNum];
    return effect;
}

@end
