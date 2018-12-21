//
//  QHVCEffectBase+Process.h
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/10/30.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEffectBase.h"
#import <CoreImage/CoreImage.h>

NS_ASSUME_NONNULL_BEGIN

@interface QHVCEffectBase(Process)

- (CIImage *)processImage:(CIImage *)image timestamp:(NSInteger)timestamp;

@end

NS_ASSUME_NONNULL_END
