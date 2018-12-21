//
//  CICustomRadialTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomRadialTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customRadialTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    vec2 p = samplerTransform(inputImage1, destCoord());
    float PI = 3.141592653589;
    vec2 rp = p*2.-1.;
    float smoothness = 1.0;
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    
    return mix(
               color2,
               color1,
               smoothstep(0., smoothness, atan(rp.y,rp.x) - ((progress)-.5) * PI * 2.5)
               );
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customRadialTransitionKernel = nil;
@implementation CICustomRadialTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customRadialTransitionKernel)
        {
            customRadialTransitionKernel = [CIKernel kernelWithString:customRadialTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customRadialTransitionKernel applyWithExtent:_inputImage.extent
                                                          roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                            arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
