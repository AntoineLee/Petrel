//
//  QHVCEditTwoInputEffectProcesser.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/9.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "QHVCEditRenderParam.h"
#import "QHVCEditEditor.h"

@interface QHVCEditTwoInputEffectProcesser : NSObject

- (instancetype)initWithEditor:(QHVCEditEditor *)editor;

- (CIImage *)processTransition:(CIImage *)firstFrame
                   secondFrame:(CIImage *)secondFrame
                transitionName:(NSString *)transitionName
                      progress:(CGFloat)progress
                     timestamp:(NSInteger)timestamp
            easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (CIImage *)processMix:(CIImage *)backgroundImage
               topImage:(CIImage *)topImage
             outputSize:(CGSize)outputSize
        backgroundColor:(NSString *)backgroundColor
              mixEffect:(QHVCEditRenderEffect *)mixEffect
              timestamp:(NSInteger)timestamp;
@end
