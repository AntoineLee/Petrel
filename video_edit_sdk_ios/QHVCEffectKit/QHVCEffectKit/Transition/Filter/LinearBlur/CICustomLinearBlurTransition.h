//
//  CICustomLinearBlurTransition.h
//  CustomCIFilter
//
//  Created by liyue-g on 2018/10/24.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <CoreImage/CoreImage.h>

@interface CICustomLinearBlurTransition : CIFilter
@property (nonatomic, retain) CIImage* inputImage;
@property (nonatomic, retain) CIImage* targetImage;
@property (nonatomic, retain) NSNumber* progress; //0~1

@end
