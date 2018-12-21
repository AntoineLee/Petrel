//
//  QHVCEffectVideoTransfer.m
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/10/31.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEffect.h"
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import "QHVCEffectUtils.h"

@interface QHVCEffectVideoTransfer ()
@property (nonatomic, retain) CIFilter* alphaFilter;
@property (nonatomic, retain) CIImage*  clearColorBgImage;

@property (nonatomic, assign) CGFloat offsetAlpha;  //透明度基于初始值的偏移，用来做透明度动画
@property (nonatomic, assign) CGFloat offsetX;      //x偏移，用来做x方向位移动画
@property (nonatomic, assign) CGFloat offsetY;      //y偏移，用来做y方向位移动画
@property (nonatomic, assign) CGFloat offsetScale;  //缩放，用来做缩放动画
@property (nonatomic, assign) CGFloat offsetRadian; //旋转，用来做缩放动画

@end

@implementation QHVCEffectVideoTransfer

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.alphaFilter = [CIFilter filterWithName:@"CIAccordionFoldTransition"];
    }
    
    return self;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    CIImage* outImage = image;
    [self computeVideoTransferParams:timestamp];
    
    if (self.offsetAlpha < 1)
    {
        NSInteger clearColorImageW = MIN(4096, CGRectGetWidth(image.extent));
        NSInteger clearColorImageH = MIN(4096, CGRectGetHeight(image.extent));
        if (!self.clearColorBgImage ||
            !CGSizeEqualToSize(CGSizeMake(clearColorImageW, clearColorImageH), self.clearColorBgImage.extent.size))
        {
            self.clearColorBgImage = [QHVCEffectUtils createImageWithColor:[UIColor clearColor] frame:CGRectMake(0, 0, clearColorImageW, clearColorImageH)];
            self.clearColorBgImage = [QHVCEffectUtils imageTranslateToZeroPoint:self.clearColorBgImage];
        }
        
        CGPoint point = image.extent.origin;
        outImage = [QHVCEffectUtils imageTranslateToZeroPoint:outImage];
        [self.alphaFilter setValue:self.clearColorBgImage forKey:kCIInputImageKey];
        [self.alphaFilter setValue:outImage forKey:@"inputTargetImage"];
        [self.alphaFilter setValue:[NSNumber numberWithFloat:self.offsetAlpha] forKey:@"inputTime"];
        outImage = self.alphaFilter.outputImage;
        outImage = [outImage imageByApplyingTransform:CGAffineTransformMakeTranslation(point.x, point.y)];
    }
    
    if (self.offsetScale != 1.0)
    {
        CGRect rect0 = outImage.extent;
        outImage = [QHVCEffectUtils imageTranslateToZeroPoint:outImage];
        
        CATransform3D offsetScale3D = CATransform3DMakeScale(self.offsetScale, self.offsetScale, 1);
        CGAffineTransform offsetScaleTrans = CATransform3DGetAffineTransform(offsetScale3D);
        outImage = [outImage imageByApplyingTransform:offsetScaleTrans];
        
        CGFloat offsetX = (CGRectGetWidth(rect0) - CGRectGetWidth(outImage.extent))/2.0;
        CGFloat offsetY = ( CGRectGetHeight(rect0) - CGRectGetHeight(outImage.extent))/2.0;
        CGFloat newX = offsetX + CGRectGetMinX(rect0);
        CGFloat newY = offsetY + CGRectGetMinY(rect0);
        outImage = [outImage imageByApplyingTransform:CGAffineTransformMakeTranslation(newX, newY)];
    }
    
    if (self.offsetRadian != 0)
    {
        CGRect renderRect = outImage.extent;
        CATransform3D offsetRadianTransform3D = [self rotateTransform:CATransform3DIdentity radian:self.offsetRadian clockwise:NO];
        CGAffineTransform offsetRadianTransform = CATransform3DGetAffineTransform(offsetRadianTransform3D);
        outImage = [outImage imageByApplyingTransform:offsetRadianTransform];
        outImage = [QHVCEffectUtils imageTranslateToZeroPoint:outImage];
        
        CGFloat x0 = CGRectGetMinX(renderRect) + CGRectGetWidth(renderRect) / 2.0;
        CGFloat y0 = CGRectGetMinY(renderRect) + CGRectGetHeight(renderRect) / 2.0;
        CGFloat newX = x0 - CGRectGetWidth(outImage.extent)/2.0;
        CGFloat newY = y0 - CGRectGetHeight(outImage.extent)/2.0;
        CGAffineTransform trans = CGAffineTransformMakeTranslation(newX, newY);
        outImage = [outImage imageByApplyingTransform:trans];
    }
    
    if (self.offsetX != 0 || self.offsetY != 0)
    {
        CGRect renderRect = outImage.extent;
        CGFloat x0 = CGRectGetMinX(renderRect) + CGRectGetWidth(renderRect) / 2.0;
        CGFloat y0 = CGRectGetMinY(renderRect) + CGRectGetHeight(renderRect) / 2.0;
        CGFloat x1 = x0 + self.offsetX;
        CGFloat y1 = y0 + self.offsetY;
        CGFloat newX = x1 - CGRectGetWidth(outImage.extent)/2.0;
        CGFloat newY = y1 - CGRectGetHeight(outImage.extent)/2.0;
        CGFloat newOffsetX = newX - CGRectGetMinX(renderRect);
        CGFloat newOffsetY = newY - CGRectGetMinY(renderRect);
        
        CGFloat offsetX = newOffsetX;
        CGFloat offsetY = newOffsetY;
        CGAffineTransform trans = CGAffineTransformMakeTranslation(offsetX, offsetY);
        outImage = [outImage imageByApplyingTransform:trans];
    }

    return outImage;
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

@end
