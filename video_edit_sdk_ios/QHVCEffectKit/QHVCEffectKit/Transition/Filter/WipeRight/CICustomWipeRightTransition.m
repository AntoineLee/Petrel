//
//  CICustomWipeRightTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomWipeRightTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customWipeRightTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    vec2 uv = samplerTransform(inputImage1, destCoord());
    vec2 p=uv.xy/vec2(1.0).xy;
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    return mix(color1, color2, step(0.0+p.x,progress));
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customWipeRightTransitionKernel = nil;
@implementation CICustomWipeRightTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customWipeRightTransitionKernel)
        {
            customWipeRightTransitionKernel = [CIKernel kernelWithString:customWipeRightTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customWipeRightTransitionKernel applyWithExtent:_inputImage.extent
                                                                  roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                                    arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
