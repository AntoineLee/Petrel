//
//  QHVCEditThumbnailInternal.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/23.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditThumbnail.h"

@interface QHVCEditThumbnailInternal : NSObject

- (instancetype)init;
- (QHVCEditError)free;

- (QHVCEditError)requestThumbnailFromFile:(NSString *)filePath
                                    width:(NSInteger)width
                                   height:(NSInteger)height
                                startTime:(NSInteger)startTime
                                  endTime:(NSInteger)endTime
                                    count:(NSInteger)count
                             dataCallback:(QHVCEditThumbnailCallback)callback;

- (QHVCEditError)requestThumbnailFromFile:(NSString *)filePath
                                    width:(NSInteger)width
                                   height:(NSInteger)height
                                timestamp:(NSInteger)timeMs
                             dataCallback:(QHVCEditThumbnailCallback)callback;

- (QHVCEditError)cancelAllThumbnailRequest;

//仅内部使用
- (void)onThumbnailEventCB:(void *)thumbnailHandle params:(void *)params status:(int)status blockIndex:(int)blockIndex;
- (UIImage *)rotateImage:(UIImage *)image veRotate:(int)veRotate;
- (int)imageRotationtoVERotate:(UIImage *)image;

@end
