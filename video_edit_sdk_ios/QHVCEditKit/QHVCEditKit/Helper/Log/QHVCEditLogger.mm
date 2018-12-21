//
//  REKLogger.m
//  QHLCRtcEngineKit
//
//  Created by liyue-g on 16/12/12.
//  Copyright © 2016年 liyue-g. All rights reserved.
//

#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditConfig.h"
#import "ve_interface.h"
#import "QHVCEditLogPrinter.h"

static QHVCEditLoggerLevel gSDKLogLevel = QHVCEditLoggerLevelError;
static QHVCEditLoggerLevel gSDKLogForFileLevel = QHVCEditLoggerLevelError;
static QHVCEditLoggerLevel gUserLogLevel = QHVCEditLoggerLevelError;
static QHVCEditLoggerLevel gUserLogForFileLevel = QHVCEditLoggerLevelError;
static BOOL gWriteToLocal = NO;
static NSString* gLogFilePath = @"";

void ve_log_printer(int level, const char* log)
{
    QHVCEditLoggerLevel logLevel = QHVCEditLoggerLevelError;
    if (level == 0)
    {
        logLevel = QHVCEditLoggerLevelTrace;
    }
    else if (level == 1)
    {
        logLevel = QHVCEditLoggerLevelInfo;
    }
    else if (level == 2)
    {
        logLevel = QHVCEditLoggerLevelDebug;
    }
    else if (level == 3)
    {
        logLevel = QHVCEditLoggerLevelWarn;
    }
    else if (level == 4)
    {
        logLevel = QHVCEditLoggerLevelError;
    }
    
    NSString* msg = [NSString stringWithCString:log encoding:NSUTF8StringEncoding];
    [QHVCEditLogger printSDKLog:logLevel prefix:@"" content:msg];
}

@implementation QHVCEditLogger

+ (void)setSDKLogLevel:(QHVCEditLoggerLevel)level
{
    if (level > QHVCEditLoggerLevelNone)
    {
        ve_enable_log(1);
        ve_set_log_printer(ve_log_printer);
    }
    else
    {
        ve_enable_log(0);
    }
    gSDKLogLevel = level;
}

+ (void)setSDKLogLevelForFile:(QHVCEditLoggerLevel)level
{
    gSDKLogForFileLevel = level;
}

+ (void)setUserLogLevel:(QHVCEditLoggerLevel)level
{
    gUserLogLevel = level;
}

+ (void)setUserLogLevelForFile:(QHVCEditLoggerLevel)level
{
    gUserLogForFileLevel = level;
}

+ (void)printUserLog:(QHVCEditLoggerLevel)level prefix:(NSString *)prefix content:(NSString *)content
{
    @autoreleasepool
    {
        NSString* msg = [NSString stringWithFormat:@"%@%@", prefix, content];
        if (level <= gUserLogLevel)
        {
            [QHVCEditLogPrinter printLogger:msg level:level];
        }
        
        if (gWriteToLocal && level <= gSDKLogForFileLevel)
        {
            [QHVCEditLogPrinter writeLogger:msg level:level];
        }   
    }
}

+ (void)printSDKLog:(QHVCEditLoggerLevel)level prefix:(NSString *)prefix content:(NSString *)content
{
    @autoreleasepool
    {
        NSString* msg = [NSString stringWithFormat:@"%@%@", prefix, content];
        if (level <= gSDKLogLevel)
        {
            [QHVCEditLogPrinter printLogger:msg level:level];
        }
        
        if (gWriteToLocal && level <= gSDKLogForFileLevel)
        {
            [QHVCEditLogPrinter writeLogger:msg level:level];
        }
    }
}

+ (void)setLogFilePath:(NSString *)path
{
    if (![QHVCEditUtils stringIsNull:path])
    {
        gLogFilePath = path;
    }
}

+ (void)writeLogToLocal:(BOOL)writeToLocal
{
    gWriteToLocal = writeToLocal;
    [QHVCEditLogPrinter writeToLocal:writeToLocal];
}

+ (void)setLogFileParams:(NSInteger)singleSize count:(NSInteger)count
{
    [QHVCEditLogPrinter setLogFileParams:singleSize count:count];
}

#pragma mark - 本地日志格式化

+ (void)logFunc:(const char *)func line:(int)line prefix:(NSString*)prefix level:(QHVCEditLoggerLevel)level msg:(NSString *)fmt, ...
{
    va_list args;
    va_start(args, fmt);
    NSString* content = [[NSString alloc] initWithFormat:fmt arguments:args];
    va_end(args);
    [QHVCEditLogger printSDKLog:level prefix:prefix content:content];
}

@end
