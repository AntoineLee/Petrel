//
//  CIDRotateFilter.m
//  CoreImageDemo
//
//  Created by liyue-g on 2017/12/5.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import "QHVCEditCIRotateFilter.h"
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import "QHVCEditUtilsSet.h"
#import "QHVCEditMacroDefs.h"

@interface QHVCEditCIRotateFilter()
@property (nonatomic, retain) CIFilter* rotateFilter;
@property (nonatomic, retain) CIFilter* scaleFilter;
@property (nonatomic, retain) CIFilter* blurFilter;
@property (nonatomic, retain) CIFilter* bgFilter;
@property (nonatomic, retain) CIFilter* alphaFilter;
@property (nonatomic, retain) CIImage*  bgImage;
@property (nonatomic, assign) CGSize    curOutputSize;
@property (nonatomic, retain) CIImage*  clearColorBgImage;
@property (nonatomic, retain) NSString* curBgColor;

@end

@implementation QHVCEditCIRotateFilter

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!self.rotateFilter)
        {
            self.rotateFilter = [CIFilter filterWithName:@"CIAffineTransform"];
            [self.rotateFilter setDefaults];
        }
        
        if (!self.bgFilter)
        {
            self.bgFilter = [CIFilter filterWithName:@"CISourceOverCompositing"];
            [self.bgFilter setDefaults];
        }
        
        if (!self.blurFilter)
        {
            self.blurFilter = [CIFilter filterWithName:@"CIGaussianBlur"];
            [self.blurFilter setValue:@30 forKey:kCIInputRadiusKey];
        }
        
        if (!self.alphaFilter)
        {
            self.alphaFilter = [CIFilter filterWithName:@"CIAccordionFoldTransition"];
        }
        
        if (!self.scaleFilter)
        {
            self.scaleFilter = [CIFilter filterWithName:@"CILanczosScaleTransform"];
        }
        
        self.bgColor = @"FF000000";
        self.bgMode = QHVCEditCIRotateFilterBGMode_Color;
        self.fillMode = QHVCEditCIRotateFilterFillMode_AspectFit;
    }
    
    return self;
}

- (void)setBackgroundImage:(UIColor *)color frame:(CGRect)frame
{
    self.bgImage = [self createImageWithColor:color frame:frame];
}

- (CIImage *)createImageWithColor:(UIColor *)color frame:(CGRect)rect
{
    CIImage* ciImage = nil;
    if (QHVCEDIT_SYSTEM_VERSION_GREATER_THAN(@"11.0"))
    {
        CIColor* ciColor = [CIColor colorWithCGColor:color.CGColor];
        CIFilter* colorFilter = [CIFilter filterWithName:@"CIConstantColorGenerator"];
        [colorFilter setValue:ciColor forKey:@"inputColor"];
        ciImage = colorFilter.outputImage;
        ciImage = [ciImage imageByCroppingToRect:rect];
    }
    else
    {
        UIGraphicsBeginImageContext(rect.size);
        CGContextRef context = UIGraphicsGetCurrentContext();
        CGContextSetFillColorWithColor(context, [color CGColor]);
        CGContextFillRect(context, rect);
        UIImage *theImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        ciImage = [CIImage imageWithCGImage:theImage.CGImage];
    }
    
    return ciImage;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestampMs
{
    if (!image)
    {
        return image;
    }
    
    //原图旋转
    CATransform3D sourceFlipTrans = [self flipTransform:CATransform3DIdentity];
    CATransform3D sourceRotateTrans = [self rotateTransform:sourceFlipTrans radian:self.sourceRadian clockwise:NO];
    CGAffineTransform sourceTransform = CATransform3DGetAffineTransform(sourceRotateTrans);
    [self.rotateFilter setValue:image forKey:kCIInputImageKey];
    [self.rotateFilter setValue:[NSValue valueWithBytes:&sourceTransform objCType:@encode(CGAffineTransform)] forKey:kCIInputTransformKey];
    CIImage* transImg = self.rotateFilter.outputImage;
    
    //原图裁剪
    transImg = [QHVCEditUtils imageTranslateToZeroPoint:transImg];
    CGSize frameSize = transImg.extent.size;
    int renderW = (self.renderRect.size.width > 0) ? self.renderRect.size.width:frameSize.width;
    int renderH = (self.renderRect.size.height > 0) ? self.renderRect.size.height:frameSize.height;
    CGFloat sourceW = (self.sourceRect.size.width > 0) ? self.sourceRect.size.width:frameSize.width;
    CGFloat sourceH = (self.sourceRect.size.height > 0) ? self.sourceRect.size.height:frameSize.height;
//    CGRect renderRect = CGRectMake(self.renderRect.origin.x, self.renderRect.origin.y, renderW, renderH);
    CGRect sourceRect = CGRectMake(self.sourceRect.origin.x, self.sourceRect.origin.y, sourceW, sourceH);
    CGRect cropRect = CGRectMake(CGRectGetMinX(sourceRect),
                                 CGRectGetHeight(transImg.extent) - CGRectGetMinY(sourceRect) - CGRectGetHeight(sourceRect),
                                 CGRectGetWidth(sourceRect),
                                 CGRectGetHeight(sourceRect));
    transImg = [transImg imageByCroppingToRect:cropRect];
    transImg = [QHVCEditUtils imageTranslateToZeroPoint:transImg];
    
    CATransform3D contentTransform3D = [self rotateTransform:CATransform3DIdentity radian:self.contentRadian clockwise:NO];
    CGAffineTransform contentTransform = CATransform3DGetAffineTransform(contentTransform3D);
    [self.rotateFilter setValue:transImg forKey:kCIInputImageKey];
    [self.rotateFilter setValue:[NSValue valueWithBytes:&contentTransform objCType:@encode(CGAffineTransform)] forKey:kCIInputTransformKey];
    CIImage* scaleImage = self.rotateFilter.outputImage;
    
    //原图缩放
    CATransform3D scale3D = [self scaleTransform:CATransform3DIdentity frameSize:scaleImage.extent.size mode:self.fillMode];
    CGAffineTransform scaleTransform = CATransform3DGetAffineTransform(scale3D);
    [self.rotateFilter setValue:scaleImage forKey:kCIInputImageKey];
    [self.rotateFilter setValue:[NSValue valueWithBytes:&scaleTransform objCType:@encode(CGAffineTransform)] forKey:kCIInputTransformKey];
    CIImage *outputImage = [self.rotateFilter outputImage];
    
    //设置背景样式
    if (self.bgMode == QHVCEditCIRotateFilterBGMode_Color)
    {
        //纯色
        if (!self.bgImage ||
            !CGSizeEqualToSize(self.curOutputSize, self.renderRect.size) ||
            ![self.bgColor isEqualToString:self.curBgColor])
        {
            CGRect rect = CGRectMake(0, 0, CGRectGetWidth(self.renderRect), CGRectGetHeight(self.renderRect));
            UIColor* bgColor = [QHVCEditUtils colorForHex:self.bgColor];
            [self setBackgroundImage:bgColor frame:rect];
            self.curBgColor = self.bgColor;
            self.curOutputSize = self.renderRect.size;
        }
    }
    else
    {
        //原图按不同比例重新缩放
        CATransform3D transform3D = [self scaleTransform:CATransform3DIdentity
                                               frameSize:CGSizeMake(CGRectGetWidth(scaleImage.extent), CGRectGetHeight(scaleImage.extent))
                                                    mode:QHVCEditCIRotateFilterFillMode_AspectFill];
        CIImage* blurImage = [scaleImage imageByApplyingTransform:CATransform3DGetAffineTransform(transform3D)];
        
        //生成毛玻璃效果
        [self.blurFilter setValue:blurImage forKey:kCIInputImageKey];
        blurImage = self.blurFilter.outputImage;
        blurImage = [QHVCEditUtils imageTranslateToZeroPoint:blurImage];
        self.bgImage = [self imageCropToRectFromCenter:blurImage size:CGSizeMake(CGRectGetWidth(self.renderRect), CGRectGetHeight(self.renderRect))];
    }
    
    if (self.fillMode == QHVCEditCIRotateFilterFillMode_AspectFit)
    {
        //原图平移
        CIImage* transImage = outputImage;
        transImage = [QHVCEditUtils imageTranslateToZeroPoint:transImage];
        CGFloat offsetX = fabs(CGRectGetWidth(self.renderRect) - CGRectGetWidth(transImage.extent)) / 2;
        CGFloat offsetY = fabs(CGRectGetHeight(self.renderRect) - CGRectGetHeight(transImage.extent))/2;
        CGAffineTransform translation = CGAffineTransformMakeTranslation(offsetX, offsetY);
        [self.rotateFilter setValue:transImage forKey:kCIInputImageKey];
        [self.rotateFilter setValue:[NSValue valueWithBytes:&translation objCType:@encode(CGAffineTransform)] forKey:kCIInputTransformKey];
        
        //原图、背景图叠加
        CIImage* bgImage = self.bgImage;
        outputImage = [self.rotateFilter outputImage];
        if (!CGRectEqualToRect(bgImage.extent, outputImage.extent))
        {
            outputImage = [outputImage imageByCompositingOverImage:bgImage];
        }
        outputImage = [QHVCEditUtils imageTranslateToZeroPoint:outputImage];
    }
    else if (self.fillMode == QHVCEditCIRotateFilterFillMode_AspectFill)
    {
        CIImage* transImage = outputImage;
        outputImage = [self imageCropToRectFromCenter:transImage size:CGSizeMake(CGRectGetWidth(self.renderRect), CGRectGetHeight(self.renderRect))];
    }
    else
    {
        outputImage = [QHVCEditUtils imageTranslateToZeroPoint:outputImage];
    }
    
    NSInteger clearColorImageW = MIN(4096, renderW);
    NSInteger clearColorImageH = MIN(4096, renderH);
    if (!self.clearColorBgImage || !CGSizeEqualToSize(CGSizeMake(clearColorImageW, clearColorImageH), self.clearColorBgImage.extent.size))
    {
        self.clearColorBgImage = [self createImageWithColor:[UIColor clearColor] frame:CGRectMake(0, 0, clearColorImageW, clearColorImageH)];
        self.clearColorBgImage = [QHVCEditUtils imageTranslateToZeroPoint:self.clearColorBgImage];
    }
    
    if (self.previewRadian)
    {
        CATransform3D previewTransform3D = [self rotateTransform:CATransform3DIdentity radian:self.previewRadian clockwise:NO];
        CGAffineTransform previewTransform = CATransform3DGetAffineTransform(previewTransform3D);
        outputImage = [outputImage imageByApplyingTransform:previewTransform];
        outputImage = [QHVCEditUtils imageTranslateToZeroPoint:outputImage];
    }
    
    //位置平移
    CGFloat offsetX = self.renderRect.origin.x + outputImage.extent.origin.x;
    CGFloat offsetY = -(self.renderRect.origin.y) + self.outputSize.height - outputImage.extent.size.height;
    
    CGAffineTransform trans = CGAffineTransformMakeTranslation(offsetX, offsetY);
    outputImage = [outputImage imageByApplyingTransform:trans];
    outputImage = [outputImage imageByCroppingToRect:CGRectMake(0, 0, self.outputSize.width, self.outputSize.height)];
    return outputImage;
}

- (CIImage *)imageCropToRectFromCenter:(CIImage *)inputImage size:(CGSize)size
{
    CIImage* outputImage = [QHVCEditUtils imageTranslateToZeroPoint:inputImage];
    
    CGFloat inW = CGRectGetWidth(inputImage.extent);
    CGFloat intH = CGRectGetHeight(inputImage.extent);
    CGFloat outW = size.width;
    CGFloat outH = size.height;
    int outX = (inW - outW)/2;
    int outY = (intH - outH)/2;
    
    outputImage = [outputImage imageByCroppingToRect:CGRectMake(outX, outY, outW, outH)];
    outputImage = [QHVCEditUtils imageTranslateToZeroPoint:outputImage];
    
    return outputImage;
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

- (CATransform3D)flipTransform:(CATransform3D)initialTransform
{
    //flip 计算
    NSInteger flipXState = self.flipX ? 1:0;
    NSInteger flipYState = self.filpY ? 1:0;
    
    CATransform3D transform = initialTransform;
    transform = CATransform3DRotate(transform, flipXState*M_PI, 0, 1, 0);
    transform = CATransform3DRotate(transform, flipYState*M_PI, 1, 0, 0);
    
    return transform;
}

- (CATransform3D)scaleTransform:(CATransform3D)initialTransform frameSize:(CGSize)size mode:(QHVCEditCIRotateFilterFillMode)mode
{
    CATransform3D transform = initialTransform;
    if (size.width <=0 || size.height <= 0 || CGRectGetWidth(self.renderRect) <= 0 || CGRectGetHeight(self.renderRect) <= 0)
    {
        return transform;
    }
    
    CGFloat scale = 1.0;
    CGFloat scaleW = CGRectGetWidth(self.renderRect) / size.width;
    CGFloat scaleH = CGRectGetHeight(self.renderRect) / size.height;
    
    if (mode == QHVCEditCIRotateFilterFillMode_AspectFit)
    {
        scale = MIN(scaleW, scaleH);
        transform = CATransform3DScale(transform, scale, scale, 1);
    }
    else if (mode == QHVCEditCIRotateFilterFillMode_AspectFill)
    {
        scale = MAX(scaleW, scaleH);
        transform = CATransform3DScale(transform, scale, scale, 1);
    }
    else
    {
        transform = CATransform3DScale(transform, scaleW, scaleH, 1);
    }
    
    return transform;
}

@end
