//
//  QHVCEditToolsInternal.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/6/26.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditToolsInternal.h"
#import "QHVCEditCommonDef.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditLogger.h"
#import "ve_interface.h"

@implementation QHVCEditToolsInternal

+ (QHVCEditFileInfo *)getFileInfo:(NSString *)filePath
{
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        LogError(@"GetFileInfo error, filePath is null.");
        return nil;
    }

    BOOL isHeif = [QHVCEditUtils isHEICFile:filePath];
    if (isHeif)
    {
        QHVCEditFileInfo* heicInfo = [[QHVCEditFileInfo alloc] init];
        UIImage* image = [UIImage imageWithContentsOfFile:filePath];
        heicInfo.isPicture = YES;
        heicInfo.fps = 1;
        heicInfo.width = image.size.width;;
        heicInfo.height = image.size.height;

        return heicInfo;
    }

    ve_clip_info info;
    memset(&info, 0, sizeof(info));
    int ret = ve_get_file_info([filePath UTF8String], &info);
    if (ret < 0)
    {
        LogError(@"GetFileInfo error, %d", ret);
        return nil;
    }

    QHVCEditFileInfo* fileInfo = [[QHVCEditFileInfo alloc] init];
    fileInfo.isPicture = info.picture;
    fileInfo.durationMs = info.duration;
    fileInfo.videoBitrate = info.v_bitrate;
    fileInfo.fps = info.fps;
    fileInfo.audioBitrate = info.a_bitrate;
    fileInfo.audioChannels = info.channels;
    fileInfo.audioSamplerate = info.samplerate;

    int width = info.width;
    int height = info.height;

    if (info.picture)
    {
        //获取图片宽高，避免底层拿不到图片旋转属性导致图片宽高相反
        UIImage* image = [UIImage imageWithContentsOfFile:filePath];
        width = image.size.width;
        height = image.size.height;
    }
    else if (info.rotate == 1 || info.rotate == 3)
    {
        //90 度和 270 度宽高颠倒
        width = info.height;
        height = info.width;
    }
    fileInfo.width = width;
    fileInfo.height = height;

    return fileInfo;
}

@end
