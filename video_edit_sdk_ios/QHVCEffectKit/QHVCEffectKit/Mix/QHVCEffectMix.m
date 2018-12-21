//
//  QHVCEffectMix.m
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/11/1.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEffect.h"
#import "QHVCEffectUtils.h"

@interface QHVCEffectMix ()
@property (nonatomic, retain) CIImage* backgroundImage;
@property (nonatomic, strong) NSString* currentBackgroundColor;
@property (nonatomic, retain) CIFilter* blendIntensityFilter;    //叠加程度

@end

@implementation QHVCEffectMix

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.intensity = 1.0;
        self.blendIntensityFilter = [CIFilter filterWithName:@"CIAccordionFoldTransition"];
    }

    return self;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{    
    //背景
    if (self.outputSize.width > 0 && self.outputSize.height > 0 &&
        ((self.outputSize.width != CGRectGetWidth(self.backgroundImage.extent)) ||
         (self.outputSize.height != CGRectGetHeight(self.backgroundImage.extent)) ||
         ![self.backgroundColor isEqualToString:self.currentBackgroundColor]))
    {
        self.currentBackgroundColor = self.backgroundColor;
        UIColor* color = [QHVCEffectUtils colorForHex:self.backgroundColor];
        CGRect frame = CGRectMake(0, 0, self.outputSize.width, self.outputSize.height);
        self.backgroundImage = [QHVCEffectUtils createImageWithColor:color frame:frame];
    }
    
    if (!image)
    {
        if ((self.outputSize.width != CGRectGetWidth(self.topImage.extent)) ||
            (self.outputSize.height != CGRectGetHeight(self.topImage.extent)))
        {
            image = self.backgroundImage;
        }
    }
    
    CIImage* outImage = image;
    if (self.topImage)
    {
        //混合
        self.topImage = [self.topImage imageByCroppingToRect:CGRectMake(0, 0, self.outputSize.width, self.outputSize.height)];
        outImage = [self.topImage imageByCompositingOverImage:outImage];
        
        if (self.intensity < 1.0)
        {
            [self.blendIntensityFilter setValue:image forKey:kCIInputImageKey];
            [self.blendIntensityFilter setValue:outImage forKey:@"inputTargetImage"];
            [self.blendIntensityFilter setValue:[NSNumber numberWithFloat:self.intensity] forKey:@"inputTime"];
            outImage = self.blendIntensityFilter.outputImage;
        }
    }
    
    outImage = [outImage imageByCroppingToRect:CGRectMake(0, 0, self.outputSize.width, self.outputSize.height)];
    return outImage;
}

@end
