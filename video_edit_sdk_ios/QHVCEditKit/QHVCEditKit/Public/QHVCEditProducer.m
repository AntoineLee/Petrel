//
//  QHVCEditProducer.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditProducer.h"
#import "QHVCEditProducerInternal.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditLogger.h"

@interface QHVCEditProducer ()

@property (nonatomic, strong) QHVCEditProducerInternal* internal;

@end

@implementation QHVCEditProducer

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        self.internal = [[QHVCEditProducerInternal alloc] initWithTimeline:timeline];
        if (!self.internal)
        {
            return nil;
        }
    }

    LogInfo(@"initProducer, timeline[%ld]", (long)[timeline timelineId]);
    return self;
}

- (QHVCEditError)setDelegate:(id<QHVCEditProducerDelegate>)delegate
{
     LogInfo(@"set producer delegate[%@]", delegate);
    return [self.internal setProducerDelegate:delegate];
}

- (QHVCEditError)free
{
    LogInfo(@"free producer");
    return [self.internal free];
}

- (QHVCEditError)start
{
    LogInfo(@"start producer");
    return [self.internal start];
}

- (QHVCEditError)stop
{
    LogInfo(@"stop producer");
    return [self.internal stop];
}

@end
