//
//  QHVCEditPlayerInternal.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/11.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "QHVCEditCommonDef.h"
#import "QHVCEditPlayer.h"

@class QHVCEditTimeline;

@interface QHVCEditPlayerInternal : NSObject

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline;
- (QHVCEditError)setPlayerDelegate:(id<QHVCEditPlayerDelegate>)delegate;
- (QHVCEditError)free;

- (QHVCEditError)setPreview:(UIView *)preview;
- (QHVCEditError)setPreviewFillMode:(QHVCEditFillMode)fillMode;
- (QHVCEditError)setPreviewBgColor:(NSString *)color;

- (QHVCEditError)playerPlay;
- (QHVCEditError)playerStop;
- (QHVCEditError)playerSeekToTime:(NSInteger)timestamp
                     forceRequest:(BOOL)forceRequest
                         complete:(void(^)(NSInteger currentTimeMs))block;
- (QHVCEditError)resetPlayer:(NSInteger)seekTimestamp;
- (QHVCEditError)refreshPlayer:(BOOL)forBasicParams
                  forceRefresh:(BOOL)forceRefresh
                    completion:(void(^)(void))completion;

- (BOOL)isPlaying;
- (NSInteger)getCurrentTimestamp;
- (NSInteger)getPlayerDuration;
- (UIImage *)getCurrentFrame;

//仅内部使用
- (void)onPlayerEventCB:(void *)playerHandle
                 status:(int)status;
- (void)onPlayerDataCB:(void *)playerHandle
                  data:(void *)data;

@end
