//
//  QHVCEffectFilter.m
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/10/30.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEffect.h"
#import "CICustomColorCube.h"
#import <UIKit/UIKit.h>
#import "QHVCEffectUtils.h"

@interface QHVCEffectFilter ()
@property (nonatomic, retain) CIFilter* clutFilter;

@end

@implementation QHVCEffectFilter

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.intensity = 1.0;
        self.clutFilter = [CIFilter filterWithName:@"CICustomColorCube"];
    }
    
    return self;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    CIImage* outImage = image;
    if (self.clutImage)
    {
        [self.clutFilter setValue:outImage forKey:@"inputImage"];
        [self.clutFilter setValue:self.clutImage forKey:@"clutImage"];
        [self.clutFilter setValue:@(self.intensity) forKey:@"intensity"];
        outImage = self.clutFilter.outputImage;
    }
    return outImage;
}

@end
