//
//  CICustomWindowsliceTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/2.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomWindowsliceTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customWindowsliceTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    float uCount = 20.0;
    float uSmoothness = 0.5;
    
    vec2 p = samplerTransform(inputImage1, destCoord());
    float pr = smoothstep(-progress, 0.0, p.x - progress * (1.0 + uSmoothness));
    float s = step(pr, fract(uCount * p.x));
    
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    return mix(color1, color2, s);
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customWindowsliceTransitionKernel = nil;
@implementation CICustomWindowsliceTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customWindowsliceTransitionKernel)
        {
            customWindowsliceTransitionKernel = [CIKernel kernelWithString:customWindowsliceTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customWindowsliceTransitionKernel applyWithExtent:_inputImage.extent
                                                               roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                                 arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
