//
//  QHVCEditTwoInputEffectProcesser.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/9.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditTwoInputEffectProcesser.h"
#import "QHVCEditEffect.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditEffectManager.h"
#import <QHVCEffectKit/QHVCEffect.h>
#import <QHVCEffectKit/QHVCEffectBase+Process.h>

@interface QHVCEditTwoInputEffectProcesser ()
@property (nonatomic, retain) NSMutableDictionary* transitionDict;
@property (nonatomic, retain) QHVCEditEditor* editor;
@property (nonatomic, retain) QHVCEffectMix* defaultMixEffect;
@property (nonatomic, retain) QHVCEffectVideoTransition* defaultTransition;

@end

@implementation QHVCEditTwoInputEffectProcesser

- (void)dealloc
{
    [self.transitionDict removeAllObjects];
}

- (instancetype)initWithEditor:(QHVCEditEditor *)editor
{
    self = [super init];
    if (self)
    {
        self.editor = editor;
        self.transitionDict = [[NSMutableDictionary alloc] initWithCapacity:0];
    }
    
    return self;
}

- (CIImage *)processTransition:(CIImage *)firstFrame
                   secondFrame:(CIImage *)secondFrame
                transitionName:(NSString *)transitionName
                      progress:(CGFloat)progress
                     timestamp:(NSInteger)timestamp
            easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    if (!self.defaultTransition)
    {
        self.defaultTransition = [[QHVCEffectVideoTransition alloc] init];
    }
    
    CGFloat newProgress = progress;
    switch (easingFunctionType)
    {
        case QHVCEditEasingFunctionTypeLinear:
        {
            break;
        }
        case QHVCEditEasingFunctionTypeCubicEaseIn:
        {
            newProgress = [QHVCEditUtils cubicEaseIn:0.0 endValue:1.0 duration:1.0 curTime:progress];
            break;
        }
        case QHVCEditEasingFunctionTypeCubicEaseOut:
        {
            newProgress = [QHVCEditUtils cubicEaseOut:0.0 endValue:1.0 duration:1.0 curTime:progress];
            break;
        }
        case QHVCEditEasingFunctionTypeCubicEaseInOut:
        {
            newProgress = [QHVCEditUtils cubicEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:progress];
            break;
        }
        case QHVCEditEasingFunctionTypeQuintEaseInOut:
        {
            newProgress = [QHVCEditUtils quintEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:progress];
            break;
        }
        case QHVCEditEasingFunctionTypeQuartEaseInOut:
        {
            newProgress = [QHVCEditUtils quartEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:progress];
            break;
        }
        case QHVCEditEasingFunctionTypeQuadEaseInOut:
        {
            newProgress = [QHVCEditUtils quadEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:progress];
            break;
        }
        case QHVCEditEasingFunctionTypeQuadEaseOut:
        {
            newProgress = [QHVCEditUtils quadEaseOut:0.0 endValue:1.0 duration:1.0 curTime:progress];
            break;
        }
    }
    
    [self.defaultTransition setSecondImage:secondFrame];
    [self.defaultTransition setTransitionName:transitionName];
    [self.defaultTransition setProgress:newProgress];
    CIImage* outImage = [self.defaultTransition processImage:firstFrame timestamp:timestamp];
    return outImage;
}

- (CIImage *)processMix:(CIImage *)backgroundImage
               topImage:(CIImage *)topImage
             outputSize:(CGSize)outputSize
        backgroundColor:(NSString *)backgroundColor
              mixEffect:(QHVCEditRenderEffect *)mixEffect
              timestamp:(NSInteger)timestamp
{
    CIImage* outImage = backgroundImage;
    BOOL processByDefault = YES;
    if (mixEffect)
    {
        QHVCEditEffectManager* effectMgr = [self.editor getEffectOfId:mixEffect.effectId];
        QHVCEditEffect* effect = [effectMgr getEffect];
        if (effect)
        {
            [effect setTargetImage:topImage];
            outImage = [effect processImage:backgroundImage timestamp:timestamp];
            processByDefault = NO;
        }
    }
    
    if (processByDefault)
    {
        if (!self.defaultMixEffect)
        {
            self.defaultMixEffect = [[QHVCEffectMix alloc] init];
        }
        
        [self.defaultMixEffect setIntensity:1.0];
        [self.defaultMixEffect setOutputSize:outputSize];
        [self.defaultMixEffect setBackgroundColor:backgroundColor];
        [self.defaultMixEffect setTopImage:topImage];
        outImage = [self.defaultMixEffect processImage:backgroundImage timestamp:timestamp];
    }
    
    return outImage;
}


@end
