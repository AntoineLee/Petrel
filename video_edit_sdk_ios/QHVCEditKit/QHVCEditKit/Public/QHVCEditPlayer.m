//
//  QHVCEditPlayer.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditPlayer.h"
#import "QHVCEditPlayerInternal.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditLogger.h"

@interface QHVCEditPlayer ()
@property (nonatomic, retain) QHVCEditPlayerInternal* internal;

@end

@implementation QHVCEditPlayer

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        self.internal = [[QHVCEditPlayerInternal alloc] initWithTimeline:timeline];
        if (!self.internal)
        {
            return nil;
        }
    }
    
    LogInfo(@"initPlayer, timeline[%ld]", (long)[timeline timelineId]);
    return self;
}

- (QHVCEditError)setDelegate:(id<QHVCEditPlayerDelegate>)delegate
{
    LogInfo(@"set player delegate[%@]", delegate);
    return [self.internal setPlayerDelegate:delegate];
}

- (QHVCEditError)free
{
    LogInfo(@"free player");
    return [self.internal free];
}

- (BOOL)isPlaying
{
    return [self.internal isPlaying];
}

- (QHVCEditError)setPreview:(UIView *)preview
{
    LogInfo(@"player set preview [%@]", preview);
    return [self.internal setPreview:preview];
}

- (QHVCEditError)setPreviewFillMode:(QHVCEditFillMode)fillMode
{
    LogInfo(@"player set preview fill mode [%ld]", (long)fillMode);
    return [self.internal setPreviewFillMode:fillMode];
}

- (QHVCEditError)setPreviewBgColor:(NSString *)color
{
        LogInfo(@"player set preview bgColor[%@]", color);
    return [self.internal setPreviewBgColor:color];
}

- (QHVCEditError)resetPlayer:(NSInteger)seekTimestamp
{
    LogInfo(@"reset player at time[%ld]", (long)seekTimestamp);
    return [self.internal resetPlayer:seekTimestamp];
}

- (QHVCEditError)refreshPlayer:(BOOL)forBasicParams forceRefresh:(BOOL)forceRefresh completion:(void (^)(void))completion
{
    LogInfo(@"refresh player, forBasicParams[%@], forceRefresh[%@], completion[%@]",
            forBasicParams ? @"yes":@"no",
            forceRefresh ? @"yes":@"no",
            completion);
    return [self.internal refreshPlayer:forBasicParams forceRefresh:forceRefresh completion:completion];
}

- (QHVCEditError)playerPlay
{
    LogInfo(@"play player ");
    return [self.internal playerPlay];
}

- (QHVCEditError)playerStop
{
    LogInfo(@"stop player");
    return [self.internal playerStop];
}

- (QHVCEditError)playerSeekToTime:(NSInteger)timestamp forceRequest:(BOOL)forceRequest complete:(void (^)(NSInteger))block
{
    LogInfo(@"player seek to time[%ld] forceRequest[%@] complete[%@]",
            (long)timestamp,
            forceRequest ? @"yes":@"no",
            block);
    
    return [self.internal playerSeekToTime:timestamp forceRequest:forceRequest complete:block];
}

- (NSInteger)getCurrentTimestamp
{
    return [self.internal getCurrentTimestamp];
}

- (NSInteger)getPlayerDuration
{
    return [self.internal getPlayerDuration];
}

- (UIImage *)getCurrentFrame
{
    return [self.internal getCurrentFrame];
}

@end
