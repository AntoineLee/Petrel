//
//  QHVCEffectBase.h
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/10/30.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreImage/CoreImage.h>

NS_ASSUME_NONNULL_BEGIN

@interface QHVCEffectBase : NSObject

@property (nonatomic, assign) NSInteger effectStartTime;
@property (nonatomic, assign) NSInteger effectEndTime;
@property (nonatomic, retain) CIImage* targetImage;

@end

NS_ASSUME_NONNULL_END
