//
//  CICustomSimpleZoomTransition.h
//  CustomCIFilter
//
//  Created by 李越 on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import <CoreImage/CoreImage.h>

NS_ASSUME_NONNULL_BEGIN

@interface CICustomSimpleZoomTransition : CIFilter
@property (nonatomic, retain) CIImage* inputImage;
@property (nonatomic, retain) CIImage* targetImage;
@property (nonatomic, retain) NSNumber* progress; //0~1

@end

NS_ASSUME_NONNULL_END
