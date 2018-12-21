//
//  CICustomLinearBlurTransition.m
//  CustomCIFilter
//
//  Created by liyue-g on 2018/10/24.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "CICustomLinearBlurTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customLinearBlurTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    float uIntensity = 0.1;
    int passes = 6;
    
    vec4 c1 = vec4(0.0);
    vec4 c2 = vec4(0.0);
    
    float width = samplerSize(inputImage1).x;
    float height = samplerSize(inputImage1).y;
    float ratio = height / width;
    
    vec2 position1 = samplerTransform(inputImage1, destCoord());
    vec2 position2 = samplerTransform(inputImage2, destCoord());
    
    float disp = uIntensity * (0.5 - distance(vec2(0.5, 0.0), vec2(progress, 0.0)));
    for (int xi = 0; xi < passes; xi++)
    {
        float x = float(xi) / float(passes) - 0.5;
        for (int yi = 0; yi < passes; yi++)
        {
            float y = float(yi) / float(passes) - 0.5;
            vec2 v = vec2(x,y * ratio);
            float d = disp;
            
            c1 += sample(inputImage1, position1 + d*v);
            c2 += sample(inputImage2, position2 + d*v);
        }
    }
    
    c1 /= float(passes * passes);
    c2 /= float(passes * passes);
    return mix(c1, c2, progress);
}
 
 kernel vec4 customLinearBlurTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customLinearBlurTransitionKernel = nil;

@implementation CICustomLinearBlurTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customLinearBlurTransitionKernel)
        {
            customLinearBlurTransitionKernel = [CIKernel kernelWithString:customLinearBlurTransitionKernelString];
            self.progress = @(0.0);
        }
    }
    
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customLinearBlurTransitionKernel applyWithExtent:_inputImage.extent
                                                            roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                              arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
