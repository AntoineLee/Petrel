//
//  CICustomFadeGrayScaleTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomFadeGrayScaleTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customFadeGrayScaleTransitionKernelString = KERNEL_STRING
(
 vec3 grayscale (vec3 color)
{
    return vec3(0.2126*color.r + 0.7152*color.g + 0.0722*color.b);
}
 
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    float uIntensity = 0.3;
    vec2 p = samplerTransform(inputImage1, destCoord());
    vec4 fc = sample(inputImage1, p);
    vec4 tc = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    
    return mix(
               mix(vec4(grayscale(fc.rgb), 1.0), fc, smoothstep(1.0-uIntensity, 0.0, progress)),
               mix(vec4(grayscale(tc.rgb), 1.0), tc, smoothstep(    uIntensity, 1.0, progress)),
               progress);
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customFadeGrayScaleTransitionKernel = nil;
@implementation CICustomFadeGrayScaleTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customFadeGrayScaleTransitionKernel)
        {
            customFadeGrayScaleTransitionKernel = [CIKernel kernelWithString:customFadeGrayScaleTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customFadeGrayScaleTransitionKernel applyWithExtent:_inputImage.extent
                                                                roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                                  arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
