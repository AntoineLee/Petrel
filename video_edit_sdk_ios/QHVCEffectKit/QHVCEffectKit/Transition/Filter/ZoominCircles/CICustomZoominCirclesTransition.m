//
//  CICustomZoominCirclesTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/2.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomZoominCirclesTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customZoominCirclesTransitionKernelString = KERNEL_STRING
(
 vec2 zoom(vec2 uv, float amount)
{
    return 0.5 + (uv - 0.5) * amount;
}
 
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    float ratio = samplerSize(inputImage1).x / samplerSize(inputImage1).y;
    vec2 ratio2 = vec2(1.0, 1.0 / ratio);

    vec2 uv = samplerTransform(inputImage1, destCoord());
    vec2 r = 2.0 * (uv - 0.5) * ratio2;
    float pro = progress / 0.8;
    float z = pro * 0.2;
    float t = 0.0;
    
    if (pro > 1.0)
    {
        z = 0.2 + (pro - 1.0) * 5.0;
        t = clamp(((progress) - 0.8) / 0.07, 0.0, 1.0);
    }

    if (length(r) < 0.8+z*1.5)
    {
        uv = zoom(uv, 1.0 - 0.15 * pro);
        t = t * 0.5;
    }
    else if (length(r) < 1.2+z*2.5)
    {
        uv = zoom(uv, 1.0 - 0.2 * pro);
        t = t * 0.2;
    }
    else
    {
        uv = zoom(uv, 1.0 - 0.25 * pro);
    }
    
    vec4 color1 = sample(inputImage1, uv);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    return mix(color1, color2, t);
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customZoominCirclesTransitionKernel = nil;
@implementation CICustomZoominCirclesTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customZoominCirclesTransitionKernel)
        {
            customZoominCirclesTransitionKernel = [CIKernel kernelWithString:customZoominCirclesTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customZoominCirclesTransitionKernel applyWithExtent:_inputImage.extent
                                                             roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                               arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
