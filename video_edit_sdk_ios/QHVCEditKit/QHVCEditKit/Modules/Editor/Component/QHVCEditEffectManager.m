//
//  QHVCEditEffectManager.m
//  QHVCEditKit
//
//  Created by 李越 on 2018/7/29.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEffectManager.h"
#import "QHVCEditEffect.h"

@interface QHVCEditEffectManager ()
@property (nonatomic, retain) QHVCEditEffect* effect;

@end

@implementation QHVCEditEffectManager

- (instancetype)initWithEffect:(QHVCEditEffect *)effect
{
    self = [super init];
    if (self)
    {
        self.effect = effect;
    }
    
    return self;
}

- (QHVCEditEffect *)getEffect
{
    return _effect;
}

@end
