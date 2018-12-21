//
//  QHVCEditTestProducer.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/9.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEditTestProducer.h"
#import "QHVCEditTestMacroDefs.h"
#import <QHVCEditKit/QHVCEditKit.h>
#import "QHVCEditTestEditor.h"
#import "QHVCEditProducer.h"
#import "QHVCEditProducerInternal.h"

#pragma mark - TestProducer

@interface QHVCEditTestProducer : NSObject <QHVCEditProducerDelegate>
@property (nonatomic, retain) QHVCEditTimeline* timeline;
@property (nonatomic, retain) QHVCEditProducer* producer;

@end

@implementation QHVCEditTestProducer

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        self.timeline = timeline;
    }
    return self;
}

- (void)createProducer
{
    if (!self.producer)
    {
        self.producer = [[QHVCEditProducer alloc] initWithTimeline:nil];
        self.producer = [[QHVCEditProducer alloc] initWithTimeline:self.timeline];
        [self.producer setDelegate:self];
    }
}

- (void)free
{
    [self.producer free];
}

- (void)start
{
    [self.producer start];
}

- (void)stop
{
    [self.producer stop];
}

- (void)onProducerError:(QHVCEditError)error
{}

- (void)onProducerInterrupt
{}

- (void)onProducerComplete
{}

- (void)onProducerProgress:(float)progress
{}

@end

#pragma mark - Producer Test Interfacce

QHVCEditTimeline* QHVCEditTestProducerCreateTimeline()
{
    QHVCEditTimeline* timeline = QHVCEditTestEditorCreateTimeline();
    QHVCEditSequenceTrack* track1 = [[QHVCEditSequenceTrack alloc] initWithTimeline:timeline type:QHVCEditTrackTypeVideo];
    QHVCEditBgParams* bgParam = [[QHVCEditBgParams alloc] init];
    [track1 setBgParams:bgParam];
    [timeline appendTrack:track1];
    
    NSString* path1 = [[NSBundle mainBundle] pathForResource:@"video1" ofType:@"MOV"];
    QHVCEditTrackClip* clip1 = [[QHVCEditTrackClip alloc] initClipWithTimeline:timeline];
    [clip1 setFilePath:path1 type:QHVCEditTrackClipTypeVideo];
    [clip1 setFileStartTime:0];
    [clip1 setFileEndTime:1000];
    [track1 appendClip:clip1];
    
    NSString* path2 = [[NSBundle mainBundle] pathForResource:@"video2" ofType:@"mp4"];
    QHVCEditTrackClip* clip2 = [[QHVCEditTrackClip alloc] initClipWithTimeline:timeline];
    [clip2 setFilePath:path2 type:QHVCEditTrackClipTypeVideo];
    [clip2 setFileStartTime:0];
    [clip2 setFileEndTime:1000];
    [track1 appendClip:clip2];
    
    [track1 addVideoTransitionToIndex:1 duration:500 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    
    QHVCEditEffect* effect1 = [[QHVCEditEffect alloc] initEffectWithTimeline:timeline];
    [effect1 setStartTime:0];
    [effect1 setEndTime:1000];
    [clip1 addEffect:effect1];
    
    QHVCEditEffect* effect2 = [[QHVCEditEffect alloc] initEffectWithTimeline:timeline];
    [effect2 setStartTime:0];
    [effect2 setEndTime:1000];
    [clip2 addEffect:effect2];
    
    QHVCEditEffect* effect3 = [[QHVCEditEffect alloc] initEffectWithTimeline:timeline];
    [effect3 setStartTime:0];
    [effect3 setEndTime:1000];
    [track1 addEffect:effect3];
    
    QHVCEditEffect* effect4 = [[QHVCEditEffect alloc] initEffectWithTimeline:timeline];
    [effect4 setStartTime:0];
    [effect4 setEndTime:1000];
    [track1 addEffect:effect4];
    [timeline addEffect:effect4];
    
    return timeline;
}

int QHVCEditTestProducerInternal(void)
{
    QHVCEditTimeline* timeline = QHVCEditTestProducerCreateTimeline();
    QHVCEditProducerInternal* internal = [[QHVCEditProducerInternal alloc] initWithTimeline:timeline];
    [internal setOperationCount:3];
    [internal start];
    
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, 2*1000*1000*1000);
    dispatch_semaphore_wait(semaphore, t);
    
    return 0;
}

int QHVCEditTestProducerAll(void)
{
    QHVCEditTimeline* timeline = QHVCEditTestProducerCreateTimeline();
    QHVCEditTestProducer* testProducer = [[QHVCEditTestProducer alloc] initWithTimeline:timeline];
    [testProducer createProducer];
    [testProducer start];
    [testProducer stop];
    
    [testProducer start];
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, 2*1000*1000*1000);
    dispatch_semaphore_wait(semaphore, t);

    [testProducer free];
    [testProducer start];
    
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestProducerInternal());
    
    return 0;
}
