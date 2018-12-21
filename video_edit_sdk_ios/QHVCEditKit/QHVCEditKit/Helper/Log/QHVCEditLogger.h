//
//  WKLogger.h
//  WuKongTV
//
//  Created by liyue-g on 16/12/12.
//  Copyright © 2016年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum
{
    QHVCEditLoggerLevelNone,
    QHVCEditLoggerLevelError,
    QHVCEditLoggerLevelWarn,
    QHVCEditLoggerLevelInfo,
    QHVCEditLoggerLevelDebug,
    QHVCEditLoggerLevelTrace,
}QHVCEditLoggerLevel;

@interface QHVCEditLogger : NSObject

#define LogDebug(...)    [QHVCEditLogger logFunc:__FUNCTION__ line:__LINE__ prefix:@"[EDITKIT]:" level:QHVCEditLoggerLevelDebug msg:__VA_ARGS__]
#define LogInfo(...)     [QHVCEditLogger logFunc:__FUNCTION__ line:__LINE__ prefix:@"[EDITKIT]:" level:QHVCEditLoggerLevelInfo msg:__VA_ARGS__]
#define LogWarn(...)     [QHVCEditLogger logFunc:__FUNCTION__ line:__LINE__ prefix:@"[EDITKIT]:" level:QHVCEditLoggerLevelWarn msg:__VA_ARGS__]
#define LogError(...)    [QHVCEditLogger logFunc:__FUNCTION__ line:__LINE__ prefix:@"[EDITKIT]:" level:QHVCEditLoggerLevelError msg:__VA_ARGS__]

+ (void)setSDKLogLevel:(QHVCEditLoggerLevel)level;
+ (void)setSDKLogLevelForFile:(QHVCEditLoggerLevel)level;

+ (void)setUserLogLevel:(QHVCEditLoggerLevel)level;
+ (void)setUserLogLevelForFile:(QHVCEditLoggerLevel)level;

+ (void)printUserLog:(QHVCEditLoggerLevel)level prefix:(NSString *)prefix content:(NSString *)content;
+ (void)printSDKLog:(QHVCEditLoggerLevel)level prefix:(NSString *)prefix content:(NSString *)content;

+ (void)setLogFilePath:(NSString *)path;
+ (void)writeLogToLocal:(BOOL)writeToLocal;
+ (void)setLogFileParams:(NSInteger)singleSize count:(NSInteger)count;

//仅SDK内部使用
+ (void)logFunc:(const char *)func line:(int)line prefix:(NSString*)prefix level:(QHVCEditLoggerLevel)level msg:(NSString *)fmt, ...NS_FORMAT_FUNCTION(5, 6);

@end
