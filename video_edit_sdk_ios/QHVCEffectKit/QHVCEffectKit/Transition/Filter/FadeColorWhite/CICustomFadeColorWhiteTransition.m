//
//  CICustomFadeColorWhiteTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomFadeColorWhiteTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customFadeColorWhiteTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    vec3 color = vec3(1.0);
    float colorPhase = 0.4;
    
    vec2 p = samplerTransform(inputImage1, destCoord());
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    
    return mix(
               mix(vec4(color, 1.0), color1, smoothstep(1.0-colorPhase, 0.0, progress)),
               mix(vec4(color, 1.0), color2, smoothstep(colorPhase, 1.0, progress)),
               progress);
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customFadeColorWhiteTransitionKernel = nil;
@implementation CICustomFadeColorWhiteTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customFadeColorWhiteTransitionKernel)
        {
            customFadeColorWhiteTransitionKernel = [CIKernel kernelWithString:customFadeColorWhiteTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customFadeColorWhiteTransitionKernel applyWithExtent:_inputImage.extent
                                                           roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                             arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
