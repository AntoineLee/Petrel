//
//  CICustomCrossZoomTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/2.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomCrossZoomTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customCrossZoomTransitionKernelString = KERNEL_STRING
(
 float Linear_ease(float begin, float change, float duration, float time)
{
    return change * time / duration + begin;
}
 
 float Exponential_easeInOut(float begin, float change, float duration, float time)
{
    if (time == 0.0)
        return begin;
    else if (time == duration)
        return begin + change;
    time = time / (duration / 2.0);
    if (time < 1.0)
        return change / 2.0 * pow(2.0, 10.0 * (time - 1.0)) + begin;
    return change / 2.0 * (-pow(2.0, -10.0 * (time - 1.0)) + 2.0) + begin;
}
 
 float Sinusoidal_easeInOut(float begin, float change, float duration, float time)
{
    float PI = 3.141592653589793;
    return -change / 2.0 * (cos(PI * time / duration) - 1.0) + begin;
}
 
 float rand (vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
 
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    float uStrength = 0.4;
    vec3 color = vec3(0.0);
    float total = 0.0;
    
    vec2 center = vec2(Linear_ease(0.25, 0.5, 1.0, progress), 0.5);
    float dissolve = Exponential_easeInOut(0.0, 1.0, 1.0, progress);
    float strength = Sinusoidal_easeInOut(0.0, uStrength, 0.5, progress);
    
    vec2 uv = samplerTransform(inputImage1, destCoord());
    vec2 texCoord = uv.xy / vec2(1.0).xy;
    vec2 toCenter = center - texCoord;
    float offset = rand(uv);
    
    for (float t = 0.0; t <= 40.0; t++)
    {
        float percent = (t + offset) / 40.0;
        float weight = 4.0 * (percent - percent * percent);
        
        vec4 fromColor = sample(inputImage1, uv);
        vec4 toColor = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
        color += mix(fromColor.rgb, toColor.rgb, dissolve) * weight;
        total += weight;
    }
    
    if (progress < 1.0)
    {
        return vec4(color / total, 1.0);
    }
    else
    {
        return sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    }
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customCrossZoomTransitionKernel = nil;
@implementation CICustomCrossZoomTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customCrossZoomTransitionKernel)
        {
            customCrossZoomTransitionKernel = [CIKernel kernelWithString:customCrossZoomTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customCrossZoomTransitionKernel applyWithExtent:_inputImage.extent
                                                                roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                                  arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
