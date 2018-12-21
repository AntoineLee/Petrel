//
//  CICustomFadeColorTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomFadeColorTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customFadeColorTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    vec3  uColor = vec3(0.0);
    float uColorPhase = 0.4;
    
    vec2 p = samplerTransform(inputImage1, destCoord());
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    
    return mix(
               mix(vec4(uColor, 1.0), color1, smoothstep(1.0 - uColorPhase, 0.0, progress)),
               mix(vec4(uColor, 1.0), color2, smoothstep(uColorPhase, 1.0, progress)),
               progress);
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customFadeColorTransitionKernel = nil;
@implementation CICustomFadeColorTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customFadeColorTransitionKernel)
        {
            customFadeColorTransitionKernel = [CIKernel kernelWithString:customFadeColorTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customFadeColorTransitionKernel applyWithExtent:_inputImage.extent
                                                      roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                        arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
