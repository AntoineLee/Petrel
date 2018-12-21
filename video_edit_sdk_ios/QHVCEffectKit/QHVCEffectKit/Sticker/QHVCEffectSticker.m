//
//  CIDImageFilter.m
//  QHVCEffectKit
//
//  Created by liyue-g on 2017/12/6.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import "QHVCEffect.h"
#import <UIKit/UIKit.h>
#import "QHVCEffectUtils.h"

@interface QHVCEffectSticker ()

@property (nonatomic, retain) CIFilter* compositFilter;
@property (nonatomic, retain) CIFilter* transformFilter;
@property (nonatomic, retain) CIFilter* scaleFilter;
@property (nonatomic, retain) CIFilter* alphaFilter;
@property (nonatomic, retain) CIImage*  clearColorBgImage;

@property (nonatomic, assign) CGFloat offsetAlpha;  //透明度基于初始值的偏移，用来做透明度动画
@property (nonatomic, assign) CGFloat offsetX;      //x偏移，用来做x方向位移动画
@property (nonatomic, assign) CGFloat offsetY;      //y偏移，用来做y方向位移动画
@property (nonatomic, assign) CGFloat offsetScale;  //缩放，用来做缩放动画
@property (nonatomic, assign) CGFloat offsetRadian; //旋转，用来做缩放动画

@end

@implementation QHVCEffectSticker

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!self.compositFilter)
        {
            self.compositFilter = [CIFilter filterWithName:@"CISourceOverCompositing"];
        }
        
        if (!self.alphaFilter)
        {
            self.alphaFilter = [CIFilter filterWithName:@"CIAccordionFoldTransition"];
        }
        
        if (!self.transformFilter)
        {
            self.transformFilter = [CIFilter filterWithName:@"CIAffineTransform"];
        }
        
        if (!self.scaleFilter)
        {
            self.scaleFilter = [CIFilter filterWithName:@"CILanczosScaleTransform"];
        }
        
        self.offsetX = 0.0;
        self.offsetY = 0.0;
        self.offsetScale = 1.0;
        self.offsetRadian = 0.0;
        self.offsetAlpha = 1.0;
    }
    return self;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestampMs
{    
    if (!self.sticker)
    {
        return image;
    }
    
    if (!self.clearColorBgImage)
    {
        self.clearColorBgImage = [QHVCEffectUtils createImageWithColor:[UIColor clearColor] frame:CGRectMake(0, 0, CGRectGetWidth(image.extent), CGRectGetHeight(image.extent))];
        self.clearColorBgImage = [QHVCEffectUtils imageTranslateToZeroPoint:self.clearColorBgImage];
    }
    
    //计算动画效果
    [self computeVideoTransferParams:timestampMs];
    
    // 缩放
    CIImage* img = self.sticker;
    CGFloat scaleW = self.renderWidth / img.extent.size.width;
    [self.scaleFilter setValue:img forKey:kCIInputImageKey];
    [self.scaleFilter setValue:[NSNumber numberWithFloat:scaleW] forKey:@"inputScale"];
    img = self.scaleFilter.outputImage;
    
    //旋转
    CATransform3D rotateTrans = [self rotateTransform:CATransform3DIdentity radian:self.renderRadian clockwise:NO];
    img = [img imageByApplyingTransform:CATransform3DGetAffineTransform(rotateTrans)];
    img = [QHVCEffectUtils imageTranslateToZeroPoint:img];
    CGFloat outputImageHeight = img.extent.size.height;
    CGFloat outputImageWidth = img.extent.size.width;
    
    if (self.offsetScale != 1.0)
    {
        CATransform3D offsetScale3D = CATransform3DMakeScale(self.offsetScale, self.offsetScale, 1);
        CGAffineTransform offsetScaleTrans = CATransform3DGetAffineTransform(offsetScale3D);
        img = [img imageByApplyingTransform:offsetScaleTrans];
    }
    
    if (self.offsetRadian != 0)
    {
        CATransform3D offsetRadianTransform3D = [self rotateTransform:CATransform3DIdentity radian:self.offsetRadian clockwise:NO];
        CGAffineTransform offsetRadianTransform = CATransform3DGetAffineTransform(offsetRadianTransform3D);
        img = [img imageByApplyingTransform:offsetRadianTransform];
        img = [QHVCEffectUtils imageTranslateToZeroPoint:img];
    }
    
    CIImage* bgImage = [QHVCEffectUtils imageTranslateToZeroPoint:image];
    CGFloat offsetX = self.renderX + self.offsetX + img.extent.origin.x + (outputImageWidth - CGRectGetWidth(img.extent))/2.0;
    CGFloat offsetY = -(self.renderY + self.offsetY) + CGRectGetHeight(bgImage.extent) - outputImageHeight + (outputImageHeight - CGRectGetHeight(img.extent))/2.0;
    CGAffineTransform trans = CGAffineTransformMakeTranslation(offsetX, offsetY);
    
    [self.transformFilter setValue:img forKey:kCIInputImageKey];
    [self.transformFilter setValue:[NSValue valueWithBytes:&trans objCType:@encode(CGAffineTransform)] forKey:kCIInputTransformKey];
    img = self.transformFilter.outputImage;
    
    //alpha处理
    CGFloat alpha = self.offsetAlpha;
    if (alpha < 1)
    {
        img = [img imageByCompositingOverImage:self.clearColorBgImage];
        [self.alphaFilter setValue:self.clearColorBgImage forKey:kCIInputImageKey];
        [self.alphaFilter setValue:img forKey:@"inputTargetImage"];
        [self.alphaFilter setValue:[NSNumber numberWithFloat:alpha] forKey:@"inputTime"];
        img = self.alphaFilter.outputImage;
    }
    
    CIImage* outImage = [img imageByCompositingOverImage:image];
    outImage = [outImage imageByCroppingToRect:CGRectMake(0, 0, CGRectGetWidth(image.extent), CGRectGetHeight(image.extent))];
    
    return outImage;
}


- (CATransform3D)rotateTransform:(CATransform3D)initialTransform radian:(CGFloat)radian clockwise:(BOOL)clockwise
{
    //旋转角度计算
    CGFloat radianArg = radian;
    if (!clockwise)
    {
        radianArg *= -1;
    }
    
    //transform 计算
    CATransform3D transform = initialTransform;
    transform = CATransform3DRotate(transform, radianArg, 0, 0, 1);
    
    return transform;
}

- (void)computeVideoTransferParams:(NSInteger)timestamp
{
    self.offsetAlpha = [QHVCEffectUtils computeAlpha:self.videoTransfer
                                           startTime:self.effectStartTime
                                             endTime:self.effectEndTime
                                         currentTime:timestamp];
    
    self.offsetX = [QHVCEffectUtils computeOffsetX:self.videoTransfer
                                         startTime:self.effectStartTime
                                           endTime:self.effectEndTime
                                       currentTime:timestamp];
    
    self.offsetY = [QHVCEffectUtils computeOffsetY:self.videoTransfer
                                         startTime:self.effectStartTime
                                           endTime:self.effectEndTime
                                       currentTime:timestamp];
    
    self.offsetScale = [QHVCEffectUtils computeOffsetScale:self.videoTransfer
                                                 startTime:self.effectStartTime
                                                   endTime:self.effectEndTime
                                               currentTime:timestamp];
    
    self.offsetRadian = [QHVCEffectUtils computeOffsetRadian:self.videoTransfer
                                                   startTime:self.effectStartTime
                                                     endTime:self.effectEndTime
                                                 currentTime:timestamp];
}

@end
