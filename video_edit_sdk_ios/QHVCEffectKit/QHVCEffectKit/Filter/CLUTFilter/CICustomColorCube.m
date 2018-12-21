//
//  CICustomColorCube.m
//  CustomCIFilter
//
//  Created by liyue-g on 2018/9/27.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "CICustomColorCube.h"
#import "QHVCEffectUtils.h"

NSString* const customColorCubeKernelString = KERNEL_STRING
(
kernel vec4 customColorCube(sampler inputImage, sampler colorCube, float intensity)
{
    float cubeLength = samplerSize(colorCube).x;
    
    vec4 originPixel = sample(inputImage, samplerTransform(inputImage, destCoord()));
    vec4 inputPixel = linear_to_srgb(originPixel);
    
    float blueColor = inputPixel.b * 63.0;
    
    vec2 quad1;
    quad1.y = floor(floor(blueColor) / 8.0);
    quad1.x = floor(blueColor) - (quad1.y * 8.0);
    
    vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 8.0);
    quad2.x = ceil(blueColor) - (quad2.y * 8.0);
    
    vec2 texPos1;
    texPos1.x = (quad1.x * 0.125) + 0.5/cubeLength + ((0.125 - 1.0/cubeLength) * inputPixel.r);
    texPos1.y = (quad1.y * 0.125) + 0.5/cubeLength + ((0.125 - 1.0/cubeLength) * inputPixel.g);
    
    vec2 texPos2;
    texPos2.x = (quad2.x * 0.125) + 0.5/cubeLength + ((0.125 - 1.0/cubeLength) * inputPixel.r);
    texPos2.y = (quad2.y * 0.125) + 0.5/cubeLength + ((0.125 - 1.0/cubeLength) * inputPixel.g);
    
    //texPos1 = clamp(texPos1, 0.0, 1.0);
    //texPos2 = clamp(texPos2, 0.0, 1.0);
    
    texPos1 = vec2(texPos1.x * cubeLength, (1.0 - texPos1.y) * cubeLength);
    texPos2 = vec2(texPos2.x * cubeLength, (1.0 - texPos2.y) * cubeLength);
    
    vec4 newColor1 = sample(colorCube, samplerTransform(colorCube, texPos1));
    vec4 newColor2 = sample(colorCube, samplerTransform(colorCube, texPos2));
    
    //newColor1 = clamp(newColor1, 0.0, 1.0);
    //newColor2 = clamp(newColor2, 0.0, 1.0);
    
    vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
    vec4 result = mix(originPixel, vec4(newColor.rgb, originPixel.a), intensity);
    
    return result;
}
);

static CIKernel* customColorCubeKernel = nil;
@implementation CICustomColorCube

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        if (!customColorCubeKernel)
        {
            customColorCubeKernel = [CIKernel kernelWithString:customColorCubeKernelString];
            self.intensity = @(1.0);
        }
    }
    
    return self;
}

- (CIImage *)outputImage
{
    CGFloat width = CGRectGetWidth(_inputImage.extent);
    CGFloat height = CGRectGetHeight(_inputImage.extent);
    CGFloat minLen = MIN(width, height);
    CGFloat clutImgLen = CGRectGetWidth(self.clutImage.extent);
    CGRect outRect = _inputImage.extent;
    if (minLen < clutImgLen)
    {
        CGFloat scale = clutImgLen/minLen;
        outRect = CGRectMake(outRect.origin.x * scale,
                             outRect.origin.y * scale,
                             outRect.size.width * scale,
                             outRect.size.height * scale);
    }
    
    CIImage* result = [customColorCubeKernel applyWithExtent:_inputImage.extent
                                        roiCallback:^CGRect(int index, CGRect destRect) {return outRect;}
                                          arguments:@[_inputImage, self.clutImage, self.intensity]];
    return result;
}

@end
