//
//  CICustomDirectionalTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/2.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomDirectionalTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customDirectionalTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    const vec2 uDirection = vec2(0.0, 1.0);
    
    vec2 uv = samplerTransform(inputImage1, destCoord());
    vec2 p = uv + progress * sign(uDirection);
    vec2 f = fract(p);
    
    vec4 color1 = sample(inputImage1, f);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    return mix(
               color2,
               color1,
               step(0.0, p.y) * step(p.y, 1.0) * step(0.0, p.x) * step(p.x, 1.0)
               );
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customDirectionalTransitionKernel = nil;
@implementation CICustomDirectionalTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customDirectionalTransitionKernel)
        {
            customDirectionalTransitionKernel = [CIKernel kernelWithString:customDirectionalTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customDirectionalTransitionKernel applyWithExtent:_inputImage.extent
                                                           roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                             arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
