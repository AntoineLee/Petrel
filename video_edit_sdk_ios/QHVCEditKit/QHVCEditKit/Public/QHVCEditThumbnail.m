//
//  QHVCEditThumbnail.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/23.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditThumbnail.h"
#import "QHVCEditThumbnailInternal.h"
#import "QHVCEditLogger.h"

@implementation QHVCEditThumbnailItem
@end

@interface QHVCEditThumbnail ()
@property (nonatomic, strong) QHVCEditThumbnailInternal* internal;
@end

@implementation QHVCEditThumbnail

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.internal = [[QHVCEditThumbnailInternal alloc] init];
    }
    
    LogInfo(@"init thumbnail");
    return self;
}

- (QHVCEditError)free
{
    LogInfo(@"free thumbnail");
    return [self.internal free];
}

- (QHVCEditError)requestThumbnailFromFile:(NSString *)filePath
                                    width:(NSInteger)width
                                   height:(NSInteger)height
                                startTime:(NSInteger)startTime
                                  endTime:(NSInteger)endTime
                                    count:(NSInteger)count
                             dataCallback:(QHVCEditThumbnailCallback)callback
{
    LogInfo(@"request thumbnail filePath[%@] width[%ld] height[%ld] startTime[%ld] endTime[%ld] count[%ld]",
            filePath,
            (long)width,
            (long)height,
            (long)startTime,
            (long)endTime,
            (long)count);
   return [self.internal requestThumbnailFromFile:filePath
                                      width:width
                                     height:height
                                  startTime:startTime
                                    endTime:endTime
                                      count:count
                               dataCallback:callback];
}

- (QHVCEditError)requestThumbnailFromFile:(NSString *)filePath
                                    width:(NSInteger)width
                                   height:(NSInteger)height
                                timestamp:(NSInteger)timeMs
                             dataCallback:(QHVCEditThumbnailCallback)callback
{
    LogInfo(@"request thumbnail filePath[%@] width[%ld] height[%ld] timestamp[%ld]",
            filePath,
            (long)width,
            (long)height,
            (long)timeMs);
    return [self.internal requestThumbnailFromFile:filePath
                                             width:width
                                            height:height
                                         timestamp:timeMs
                                      dataCallback:callback];
}

- (QHVCEditError)cancelAllThumbnailRequest
{
    LogInfo(@"thumbnail cancel request");
    return [self.internal cancelAllThumbnailRequest];
}

@end
