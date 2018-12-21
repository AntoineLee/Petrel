//
//  CICustomFadeTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomFadeTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customFadeTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    vec2 p = samplerTransform(inputImage1, destCoord());
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    
    return color1 * (1.0 - progress) + progress * color2;
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customFadeTransitionKernel = nil;
@implementation CICustomFadeTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customFadeTransitionKernel)
        {
            customFadeTransitionKernel = [CIKernel kernelWithString:customFadeTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customFadeTransitionKernel applyWithExtent:_inputImage.extent
                                                        roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                          arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
