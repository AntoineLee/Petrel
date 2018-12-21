//
//  QHVCEditLogPrinter.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/25.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditLogger.h"

@interface QHVCEditLogPrinter : NSObject

+ (void)printLogger:(nullable NSString *)content level:(QHVCEditLoggerLevel)level;

+ (void)writeToLocal:(BOOL)writeToLocal;
+ (void)writeLogger:(nullable NSString *)content level:(QHVCEditLoggerLevel)level;
+ (void)setLogFileParams:(NSInteger)singleSize count:(NSInteger)count;

@end
