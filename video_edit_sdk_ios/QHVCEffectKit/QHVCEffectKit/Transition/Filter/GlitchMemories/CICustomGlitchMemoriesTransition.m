//
//  CICustomGlitchMemoriesTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/2.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomGlitchMemoriesTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customGlitchMemoriesTransitionKernelString = KERNEL_STRING
(
 vec4 getImageRed(sampler image, float progress)
{
    vec2 p = samplerTransform(image, destCoord());
    vec2 block = floor(p.xy / vec2(16));
    vec2 uv_noise = block / vec2(64);
    uv_noise += floor(vec2(progress) * vec2(1200.0, 3500.0)) / vec2(64);
    vec2 dist = progress > 0.0 ? (fract(uv_noise) - 0.5) * 0.3 *(1.0 - progress) : vec2(0.0);
    vec2 red = p + dist * 0.2;
    
    vec4 imageRed = sample(image, red);
    return imageRed;
}
 
 vec4 getImageGreen(sampler image, float progress)
{
    vec2 p = samplerTransform(image, destCoord());
    vec2 block = floor(p.xy / vec2(16));
    vec2 uv_noise = block / vec2(64);
    uv_noise += floor(vec2(progress) * vec2(1200.0, 3500.0)) / vec2(64);
    vec2 dist = progress > 0.0 ? (fract(uv_noise) - 0.5) * 0.3 *(1.0 - progress) : vec2(0.0);
    vec2 green = p + dist * .3;
    
    vec4 imageGreen = sample(image, green);
    return imageGreen;
}
 
 vec4 getImageBlue(sampler image, float progress)
{
    vec2 p = samplerTransform(image, destCoord());
    vec2 block = floor(p.xy / vec2(16));
    vec2 uv_noise = block / vec2(64);
    uv_noise += floor(vec2(progress) * vec2(1200.0, 3500.0)) / vec2(64);
    vec2 dist = progress > 0.0 ? (fract(uv_noise) - 0.5) * 0.3 *(1.0 - progress) : vec2(0.0);
    vec2 blue = p + dist * .5;
    
    vec4 imageBlue = sample(image, blue);
    return imageBlue;
}
 
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    vec4 fromRed = getImageRed(inputImage1, progress);
    vec4 toRed = getImageRed(inputImage2, progress);
    
    vec4 fromGreen = getImageGreen(inputImage1, progress);
    vec4 toGreen = getImageGreen(inputImage2, progress);
    
    vec4 fromBlue = getImageBlue(inputImage1, progress);
    vec4 toBlue   = getImageBlue(inputImage2, progress);
    
    return vec4(mix(fromRed, toRed, progress).r, mix(fromGreen, toGreen, progress).g, mix(fromBlue, toBlue, progress).b, 1.0);
}
 
 kernel vec4 customGlitchMemoriesTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}

);

static CIKernel* customGlitchMemoriesTransitionKernel = nil;
@implementation CICustomGlitchMemoriesTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customGlitchMemoriesTransitionKernel)
        {
            customGlitchMemoriesTransitionKernel = [CIKernel kernelWithString:customGlitchMemoriesTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customGlitchMemoriesTransitionKernel applyWithExtent:_inputImage.extent
                                                            roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                              arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
