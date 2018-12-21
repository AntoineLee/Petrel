//
//  CICustomCircleCropTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomCircleCropTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customCircleCropTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    vec4 uBgcolor = vec4(0.0, 0.0, 0.0, 1.0);
    vec2 p = samplerTransform(inputImage1, destCoord());
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    float ratio = samplerSize(inputImage1).x / samplerSize(inputImage1).y;
    vec2 ratio2 = vec2(1.0, 1.0 / ratio);
    
    float s = pow(2.0 * abs(progress - 0.5), 3.0);
    float dist = length((vec2(p) - 0.5) * ratio2);
    return mix(
               progress < 0.5 ? color1 : color2,
               uBgcolor,
               step(s, dist)
               );
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customCircleCropTransitionKernel = nil;
@implementation CICustomCircleCropTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customCircleCropTransitionKernel)
        {
            customCircleCropTransitionKernel = [CIKernel kernelWithString:customCircleCropTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customCircleCropTransitionKernel applyWithExtent:_inputImage.extent
                                                       roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                         arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
