//
//  CICustomFilterSimpleZoomTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomSimpleZoomTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customSimpleZoomTransitionKernelString = KERNEL_STRING
(
 vec2 zoom(vec2 uv, float amount)
{
    return 0.5 + ((uv - 0.5) * (1.0-amount));
}
 
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    float uZoom_quickness = 0.8;
    float nQuick = clamp(uZoom_quickness,0.2,1.0);
    vec2 p = samplerTransform(inputImage1, destCoord());
    vec4 color1 = sample(inputImage1, p);
    vec4 color2 = sample(inputImage2, samplerTransform(inputImage2, destCoord()));
    
    return mix(
               sample(inputImage1, zoom(p, smoothstep(0.0, nQuick, progress))),
               color2,
               smoothstep(nQuick-0.2, 1.0, progress)
               );
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customSimpleZoomTransitionKernel = nil;
@implementation CICustomSimpleZoomTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customSimpleZoomTransitionKernel)
        {
            customSimpleZoomTransitionKernel = [CIKernel kernelWithString:customSimpleZoomTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customSimpleZoomTransitionKernel applyWithExtent:_inputImage.extent
                                                               roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                                 arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
