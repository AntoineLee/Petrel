//
//  QHVCEditEffectManager.h
//  QHVCEditKit
//
//  Created by 李越 on 2018/7/29.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>

@class QHVCEditEffect;
@class QHVCEditObject;
@interface QHVCEditEffectManager : NSObject

- (instancetype)initWithEffect:(QHVCEditEffect *)effect;
- (QHVCEditEffect *)getEffect;

@property (nonatomic, retain) QHVCEditObject* superObject;
@property (nonatomic, assign) BOOL needReset;

@end
