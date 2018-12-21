//
//  QHVCEditCommonDef.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditCommonDef.h"
#import "QHVCEditToolsInternal.h"
#import "QHVCEditLogger.h"

@implementation QHVCEditSlowMotionVideoInfo
@end

@implementation QHVCEditBgParams
@end

@implementation QHVCEditObject
@end

@implementation QHVCEditFileInfo
@end

@implementation QHVCEditTools

+ (QHVCEditFileInfo *)getFileInfo:(NSString *)filePath
{
    return [QHVCEditToolsInternal getFileInfo:filePath];
}

+ (NSString *)getVersion
{
    //打包脚本读取专用，值不能修改
    return @"0.0.0.0";
}

+ (void)setSDKLogLevel:(QHVCEditLogLevel)level
{
    [QHVCEditLogger setSDKLogLevel:(QHVCEditLoggerLevel)level];
}

+ (void)setSDKLogLevelForFile:(QHVCEditLogLevel)level
{
    [QHVCEditLogger setSDKLogLevelForFile:(QHVCEditLoggerLevel)level];
}

+ (void)setUserLogLevel:(QHVCEditLogLevel)level
{
    [QHVCEditLogger setUserLogLevel:(QHVCEditLoggerLevel)level];
}

+ (void)setUserLogLevelForFile:(QHVCEditLogLevel)level
{
    [QHVCEditLogger setUserLogLevelForFile:(QHVCEditLoggerLevel)level];
}

+ (void)setLogFilePath:(NSString *)path
{
    [QHVCEditLogger setLogFilePath:path];
}

+ (void)writeLogToLocal:(BOOL)writeToLocal
{
    [QHVCEditLogger writeLogToLocal:writeToLocal];
}

+ (void)setLogFileParams:(NSInteger)singleSize count:(NSInteger)count
{
    [QHVCEditLogger setLogFileParams:singleSize count:count];
}

+ (void)printUserLog:(QHVCEditLogLevel)level prefix:(NSString *)prefix content:(NSString *)content
{
    [QHVCEditLogger printUserLog:(QHVCEditLoggerLevel)level prefix:prefix content:content];
}

@end
