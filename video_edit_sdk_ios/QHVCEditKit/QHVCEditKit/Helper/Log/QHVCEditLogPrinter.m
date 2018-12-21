//
//  QHVCEditLogPrinter.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/25.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditLogPrinter.h"
#import "QHVCEditConfig.h"
#import "QHVCEditUtilsSet.h"

@implementation QHVCEditLogPrinter

+ (NSString *)logLevelToString:(QHVCEditLoggerLevel)level
{
    NSString* logLevel = @"";
    switch (level)
    {
        case QHVCEditLoggerLevelTrace:
        {
            logLevel = @"QHVCTrace";
            break;
        }
        case QHVCEditLoggerLevelDebug:
        {
            logLevel = @"QHVCDebug";
            break;
        }
        case QHVCEditLoggerLevelInfo:
        {
            logLevel = @"QHVCInfo";
            break;
        }
        case QHVCEditLoggerLevelWarn:
        {
            logLevel = @"QHVCWarn";
            break;
        }
        case QHVCEditLoggerLevelError:
        {
            logLevel = @"QHVCError";
            break;
        }
        default:
            break;
    }
    return logLevel;
}

+ (void)printLogger:(nullable NSString *)content level:(QHVCEditLoggerLevel)level;
{
    NSDate* date = [NSDate date];
    NSDateFormatter* dateformatter = [[NSDateFormatter alloc] init];
    [dateformatter setDateFormat:@"YYYY-MM-dd HH:mm:ss.SSS"];
    dateformatter.lenient = YES;
    NSString* time = [dateformatter stringFromDate:date];
    
    NSString* logLevel = [QHVCEditLogPrinter logLevelToString:level];
    NSString* msg = [NSString stringWithFormat:@"[%@][%@]%@\n", time, logLevel, content];
    printf("%s", [msg UTF8String]);
}

+ (void)writeToLocal:(BOOL)writeToLocal
{
    //override
}

+ (void)writeLogger:(NSString *)content level:(QHVCEditLoggerLevel)level
{
   //override
}

+ (void)setLogFileParams:(NSInteger)singleSize count:(NSInteger)count
{
   //override
}

@end
