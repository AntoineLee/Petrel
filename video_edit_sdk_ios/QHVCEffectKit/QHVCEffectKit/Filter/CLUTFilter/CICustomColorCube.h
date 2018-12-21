//
//  CICustomColorCube.h
//  CustomCIFilter
//
//  Created by liyue-g on 2018/9/27.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <CoreImage/CoreImage.h>

NS_ASSUME_NONNULL_BEGIN

@interface CICustomColorCube : CIFilter
@property (nonatomic, retain) CIImage* inputImage;
@property (nonatomic, retain) CIImage* clutImage;
@property (nonatomic, retain) NSNumber* intensity;

@end

NS_ASSUME_NONNULL_END
