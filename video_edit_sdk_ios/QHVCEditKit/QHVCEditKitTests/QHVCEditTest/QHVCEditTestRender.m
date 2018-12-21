//
//  QHVCEditTestRender.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/7.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEditTestRender.h"
#import "QHVCEditTestMacroDefs.h"
#import <QHVCEditKit/QHVCEditKit.h>
#import <QHVCEffectKit/QHVCEffectBase+Process.h>
#import "QHVCEditTwoInputEffectProcesser.h"
#import "QHVCEditRenderParam.h"
#import "QHVCEditCIRotateFilter.h"
#import "QHVCEditCIPreview.h"
#import "QHVCEditCIFilterStack.h"

int QHVCEditTestRenderAll(void)
{
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestTwoInputEffectProcesser());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestRotateFilter());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestCIPreview());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestCIFilerStack());
    return 0;
}

int QHVCEditTestTwoInputEffectProcesser()
{
    QHVCEditTwoInputEffectProcesser* processer = [[QHVCEditTwoInputEffectProcesser alloc] initWithEditor:nil];
    UIImage* uiImage = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* image = [CIImage imageWithCGImage:uiImage.CGImage];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeCubicEaseIn];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeCubicEaseOut];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeCubicEaseInOut];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeQuintEaseInOut];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeQuartEaseInOut];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeQuadEaseInOut];
    
    [processer processTransition:image
                     secondFrame:image
                  transitionName:@"LinearBlur"
                        progress:0.5
                       timestamp:0
              easingFunctionType:QHVCEditEasingFunctionTypeQuadEaseOut];
    
    QHVCEditRenderEffect* renderEffect = [[QHVCEditRenderEffect alloc] init];
    [processer processMix:image
                 topImage:image
               outputSize:CGSizeMake(1280, 720)
          backgroundColor:@"FF000000"
                mixEffect:renderEffect
                timestamp:0];
    
    return 0;
}

int QHVCEditTestRotateFilter()
{
    UIImage* uiImage = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* image = [CIImage imageWithCGImage:uiImage.CGImage];
    
    QHVCEditCIRotateFilter* rotateFilter = [[QHVCEditCIRotateFilter alloc] init];
    [rotateFilter processImage:image timestamp:0];
    
    [rotateFilter setRenderRect:CGRectMake(0, 0, 1280, 720)];
    [rotateFilter setBgMode:QHVCEditCIRotateFilterBGMode_Blur];
    [rotateFilter setFillMode:QHVCEditCIRotateFilterFillMode_AspectFill];
    [rotateFilter setPreviewRadian:M_PI_2];
    [rotateFilter processImage:nil timestamp:0];
    [rotateFilter processImage:image timestamp:0];
    
    [rotateFilter setFillMode:QHVCEditCIRotateFilterFillMode_ScaleToFill];
    [rotateFilter processImage:image timestamp:0];
    
    return 0;
}

int QHVCEditTestCIPreview()
{
    UIImage* uiImage = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* image = [CIImage imageWithCGImage:uiImage.CGImage];
    
    QHVCEditCIPreview* preview = [[QHVCEditCIPreview alloc] init];
    [preview setPreviewFrame:CGRectMake(0, 0, 1280, 720)];
    [preview setBackgroundColorRed:0 green:0 blue:0 alpha:1];
    
    [preview setFillMode:QHVCEditFillModeAspectFill];
    [preview processImage:nil timestampMs:0 userData:nil];
    [preview processImage:image timestampMs:0 userData:nil];
    [preview renderFrame];
    
    [preview setFillMode:QHVCEditFillModeAspectFit];
    [EAGLContext setCurrentContext:nil];
    [preview processImage:image timestampMs:0 userData:nil];
    [preview renderFrame];
    
    image = [image imageByCroppingToRect:CGRectMake(0, 0, 300, 500)];
    
    [preview setFillMode:QHVCEditFillModeAspectFill];
    [preview processImage:image timestampMs:0 userData:nil];
    [preview renderFrame];
    
    [preview setFillMode:QHVCEditFillModeAspectFit];
    [preview processImage:image timestampMs:0 userData:nil];
    [preview renderFrame];
    
    image = [image imageByCroppingToRect:CGRectMake(0, 0, 300, 300)];
    
    [preview setFillMode:QHVCEditFillModeAspectFill];
    [preview processImage:image timestampMs:0 userData:nil];
    [preview renderFrame];
    
    [preview setFillMode:QHVCEditFillModeAspectFit];
    [preview processImage:image timestampMs:0 userData:nil];
    [preview renderFrame];
    
    [preview screenLocked:nil];
    [preview willEnterForeground:nil];
    [preview willResignActive:nil];
    [preview didBecomeActive:nil];
    
    [preview free];
    [preview setDisable];
    [preview processImage:image timestampMs:0 userData:nil];
    [preview renderFrame];
    
    return 0;
}

int QHVCEditTestCIFilerStack()
{
    QHVCEditCIFilterStack* stack = [[QHVCEditCIFilterStack alloc] initWithOutput:nil];
    QHVCEffectBase* effect = [[QHVCEffectBase alloc] init];
    
    [stack addFilter:nil];
    [stack addFilter:effect];
    [stack addFilter:effect atIndex:1];
    [stack addFilter:nil atIndex:0];
    [stack addFilter:effect atIndex:10];
    [stack processImage:nil timestamp:0];
    [stack removeFilterAtIndex:0];
    [stack removeFilter:nil];
    [stack removeFilter:effect];
    [stack removeAllFilters];
    
    return 0;
}

