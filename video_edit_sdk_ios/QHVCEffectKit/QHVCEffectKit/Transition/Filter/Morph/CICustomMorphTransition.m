//
//  CICustomMorphTransition.m
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "CICustomMorphTransition.h"
#import "QHVCEffectUtils.h"

NSString* const customMorphTransitionKernelString = KERNEL_STRING
(
 vec4 transition(sampler inputImage1, sampler inputImage2, float progress)
{
    float strength = 0.08;
    vec2 p1 = samplerTransform(inputImage1, destCoord());
    vec2 p2 = samplerTransform(inputImage2, destCoord());
    vec4 ca = sample(inputImage1, p1);
    vec4 cb = sample(inputImage2, p2);
    
//    vec2 oa = (((ca.rg+ca.b)*0.5)*2.0-1.0);
//    vec2 ob = (((cb.rg+cb.b)*0.5)*2.0-1.0);
    vec2 oa = ((ca.rg+ca.b));
    vec2 ob = ((cb.rg+cb.b));
    vec2 oc = mix(oa,ob,0.5)*strength;
    
    float w0 = (progress);
    float w1 = 1.0-w0;
    return  mix(sample(inputImage1, p1+oc*w0), sample(inputImage2, p2-oc*w1), (progress));
}
 
 kernel vec4 customTransition(sampler inputImage1, sampler inputImage2, float progress)
{
    return transition(inputImage1, inputImage2, progress);
}
);

static CIKernel* customMorphTransitionKernel = nil;
@implementation CICustomMorphTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customMorphTransitionKernel)
        {
            customMorphTransitionKernel = [CIKernel kernelWithString:customMorphTransitionKernelString];
        }
    }
    
    self.progress = @(0.0);
    return self;
}

- (CIImage *)outputImage
{
    CIImage* result = [customMorphTransitionKernel applyWithExtent:_inputImage.extent
                                                            roiCallback:^CGRect(int index, CGRect destRect){return destRect;}
                                                              arguments:@[_inputImage, _targetImage, self.progress]];
    return result;
}

@end
