//
//  QHVCEditCIFilterStack.h
//  QHVCEditKit
//
//  Created by liyue-g on 2017/11/21.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QHVCEffectKit/QHVCEffectBase.h>
#import "QHVCEditCIOutput.h"

@interface QHVCEditCIFilterStack : NSObject

- (instancetype)initWithOutput:(id<QHVCEditCIOutputProtocol>)output;
- (void)addFilter:(QHVCEffectBase *)filter;
- (void)addFilter:(QHVCEffectBase *)filter atIndex:(NSUInteger)index;
- (void)removeFilter:(QHVCEffectBase *)filter;
- (void)removeFilterAtIndex:(NSUInteger)index;
- (void)removeAllFilters;

- (void)processImage:(CIImage *)image timestamp:(NSInteger)timestamp;

@end
