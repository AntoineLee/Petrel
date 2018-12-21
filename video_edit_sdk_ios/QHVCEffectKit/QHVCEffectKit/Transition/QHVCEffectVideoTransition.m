//
//  QHVCEffectVideoTransition.m
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEffect.h"
#import "QHVCEffectVideoTransitionList.h"

@interface QHVCEffectVideoTransition ()
@property (nonatomic, retain) CIFilter* filter;
@property (nonatomic, retain) NSString* currentTransitionName;

@end

@implementation QHVCEffectVideoTransition

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.progress = 1.0;
    }
    
    return self;
}

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    CIImage* outImage = image;
    if (!self.filter || ![self.currentTransitionName isEqualToString:self.transitionName])
    {
        NSDictionary* dict = [QHVCEffectVideoTransitionList transitionList];
        NSString* filterName = [dict objectForKey:self.transitionName];
        self.filter = [CIFilter filterWithName:filterName];
        self.currentTransitionName = self.transitionName;
    }
    
    if (self.filter)
    {
        [self.filter setValue:image forKey:@"inputImage"];
        [self.filter setValue:self.secondImage forKey:@"targetImage"];
        [self.filter setValue:@(self.progress) forKey:@"progress"];
        outImage = self.filter.outputImage;
    }
    
    return outImage;
}

@end
