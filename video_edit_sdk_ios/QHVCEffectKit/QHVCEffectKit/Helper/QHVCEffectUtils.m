//
//  QHVCEffectUtils.m
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/10/30.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEffectUtils.h"

@implementation QHVCEffectUtils

#pragma mark - String

+ (BOOL)stringIsNull:(NSString *)parameter
{
    if ((NSNull *)parameter == [NSNull null])
    {
        return YES;
    }
    if (parameter == nil || [parameter length] == 0)
    {
        return YES;
    }
    return NO;
}

#pragma mark - transfer

+ (CGFloat)computeAlpha:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
              startTime:(NSInteger)startTime
                endTime:(NSInteger)endTime
            currentTime:(NSInteger)currentTime
{
    CGFloat alpha = 1.0;
    alpha = [QHVCEffectUtils computeType:QHVCEffectVideoTransferTypeAlpha
                            defaultValue:alpha
                          transferParams:transferParam
                         startTime:startTime
                           endTime:endTime
                       currentTime:currentTime];
    return alpha;
}

+ (CGFloat)computeOffsetX:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                startTime:(NSInteger)startTime
                  endTime:(NSInteger)endTime
              currentTime:(NSInteger)currentTime
{
    CGFloat offsetX = 0.0;
    offsetX = [QHVCEffectUtils computeType:QHVCEffectVideoTransferTypeOffsetX
                              defaultValue:offsetX
                            transferParams:transferParam
                                 startTime:startTime
                                   endTime:endTime
                                   currentTime:currentTime];
    return offsetX;
}

+ (CGFloat)computeOffsetY:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                startTime:(NSInteger)startTime
                  endTime:(NSInteger)endTime
              currentTime:(NSInteger)currentTime
{
    CGFloat offsetY = 0.0;
    offsetY = [QHVCEffectUtils computeType:QHVCEffectVideoTransferTypeOffsetY
                   defaultValue:offsetY
                 transferParams:transferParam
                      startTime:startTime
                        endTime:endTime
                    currentTime:currentTime];
    return offsetY;
}

+ (CGFloat)computeOffsetScale:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                    startTime:(NSInteger)startTime
                      endTime:(NSInteger)endTime
                  currentTime:(NSInteger)currentTime
{
    CGFloat scale = 1.0;
    scale = [QHVCEffectUtils computeType:QHVCEffectVideoTransferTypeScale
                            defaultValue:scale
                          transferParams:transferParam
                               startTime:startTime
                                 endTime:endTime
                             currentTime:currentTime];
    return scale;
}

+ (CGFloat)computeOffsetRadian:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                     startTime:(NSInteger)startTime
                       endTime:(NSInteger)endTime
                   currentTime:(NSInteger)currentTime
{
    CGFloat radian = 0.0;
    radian = [QHVCEffectUtils computeType:QHVCEffectVideoTransferTypeRadian
                             defaultValue:radian
                           transferParams:transferParam
                                startTime:startTime
                                  endTime:endTime
                              currentTime:currentTime];
    return radian;
}


+ (CGFloat)computeType:(QHVCEffectVideoTransferType)type
          defaultValue:(CGFloat)defaultValue
        transferParams:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
             startTime:(NSInteger)filterStartTime
               endTime:(NSInteger)filterEndTime
           currentTime:(NSInteger)currentTime
{
    __block CGFloat value = defaultValue;
    [transferParam enumerateObjectsUsingBlock:^(QHVCEffectVideoTransferParam * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
     {
         if (obj.transferType == type)
         {
             NSInteger time = currentTime - filterStartTime;
             //动画区间检测
             if (time >= obj.startTime && time < obj.endTime)
             {
                 if (obj.curveType == QHVCEffectVideoTransferCurveTypeLinear)
                 {
                     //线性
                     value = [QHVCEffectUtils computeLinearValue:obj.startTime
                                                         endTime:obj.endTime
                                                     currentTime:time
                                                      startValue:obj.startValue
                                                        endValue:obj.endValue];
                 }
                 else if (obj.curveType == QHVCEffectVideoTransferCurveTypeCurve)
                 {
                     //catmull-rom
                     value = [QHVCEffectUtils computeCatmullRomValue:obj.startTime
                                                             endTime:obj.endTime
                                                         currentTime:time
                                                          startValue:obj.startValue
                                                            endValue:obj.endValue];
                 }
             }
         }
     }];
    return value;
}

+ (CGFloat)computeLinearValue:(NSInteger)startTime
                      endTime:(NSInteger)endTime
                  currentTime:(NSInteger)currentTime
                   startValue:(CGFloat)startValue
                     endValue:(CGFloat)endValue
{
    CGFloat offset = (endValue - startValue)/(endTime - startTime);
    CGFloat value = (currentTime - startTime) * offset + startValue;
    return value;
}

+ (CGFloat)computeCatmullRomValue:(NSInteger)startTime
                          endTime:(NSInteger)endTime
                      currentTime:(NSInteger)currentTime
                       startValue:(CGFloat)startValue
                         endValue:(CGFloat)endValue
{
    CGFloat t = (CGFloat)(currentTime - startTime)/(endTime - startTime);
    CGFloat y0 = startValue - (endValue - startValue)/4.0;
    CGFloat y1 = startValue;
    CGFloat y2 = endValue;
    CGFloat y3 = endValue + (endValue - startValue)/4.0;
    
    CGFloat t2 = t * t;
    CGFloat a0 = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
    CGFloat a1 = y0 - 2.5 * y1 + 2 * y2 -0.5 * y3;
    CGFloat a2 = -0.5 * y0 + 0.5 * y2;
    CGFloat a3 = y1;
    
    CGFloat value = a0 * t * t2 + a1 * t2 + a2 * t + a3;
    return value;
}

#pragma mark - Color

+ (UIColor *)colorForHex:(NSString *)hexColor
{
    // String should be 8 or 9 characters if it includes '#'
    if ([hexColor length]  < 8)
        return nil;
    
    // strip # if it appears
    if ([hexColor hasPrefix:@"#"])
        hexColor = [hexColor substringFromIndex:1];
    
    // if the value isn't 6 characters at this point return
    // the color black
    if ([hexColor length] != 8)
        return nil;
    
    // Separate into a, r, g, b substrings
    NSRange range;
    range.location = 0;
    range.length = 2;
    
    NSString *aString = [hexColor substringWithRange:range];
    
    range.location = 2;
    NSString *rString = [hexColor substringWithRange:range];
    
    range.location = 4;
    NSString *gString = [hexColor substringWithRange:range];
    
    range.location = 6;
    NSString *bString = [hexColor substringWithRange:range];
    
    // Scan values
    unsigned int a, r, g, b;
    [[NSScanner scannerWithString:aString] scanHexInt:&a];
    [[NSScanner scannerWithString:rString] scanHexInt:&r];
    [[NSScanner scannerWithString:gString] scanHexInt:&g];
    [[NSScanner scannerWithString:bString] scanHexInt:&b];
    
    return [UIColor colorWithRed:((float) r / 255.0f)
                           green:((float) g / 255.0f)
                            blue:((float) b / 255.0f)
                           alpha:((float) a / 255.0f)];
}

+ (CIImage *)createImageWithColor:(UIColor *)color frame:(CGRect)frame
{
    CIImage* ciImage = nil;
    if (QHVCEFFECT_SYSTEM_VERSION_GREATER_THAN(@"11.0"))
    {
        CIColor* ciColor = [CIColor colorWithCGColor:color.CGColor];
        CIFilter* colorFilter = [CIFilter filterWithName:@"CIConstantColorGenerator"];
        [colorFilter setValue:ciColor forKey:@"inputColor"];
        ciImage = colorFilter.outputImage;
        ciImage = [ciImage imageByCroppingToRect:frame];
    }
    else
    {
        UIGraphicsBeginImageContext(frame.size);
        CGContextRef context = UIGraphicsGetCurrentContext();
        CGContextSetFillColorWithColor(context, [color CGColor]);
        CGContextFillRect(context, frame);
        UIImage *theImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        ciImage = [CIImage imageWithCGImage:theImage.CGImage];
    }
    
    return ciImage;
}

#pragma mark - Transform

+ (CIImage *)imageTranslateToZeroPoint:(CIImage *)inputImage
{
    CIImage* outputImage = inputImage;
    CGAffineTransform translation = CGAffineTransformMakeTranslation(-inputImage.extent.origin.x, -inputImage.extent.origin.y);
    outputImage = [inputImage imageByApplyingTransform:translation];
    
    return outputImage;
}

@end
