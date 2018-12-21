//
//  QHVCEditTestPlayer.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/7.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEditTestPlayer.h"
#import "QHVCEditTestMacroDefs.h"
#import <QHVCEditKit/QHVCEditKit.h>
#import "QHVCEditTestEditor.h"
#import "QHVCEditPlayerInternal.h"

#pragma mark - TestPlayerVC

@interface QHVCEditTestPlayerVC : UIViewController <QHVCEditPlayerDelegate>

@property (nonatomic, retain) QHVCEditPlayer* player;
@property (nonatomic, retain) QHVCEditTimeline* timeline;
@property (nonatomic, retain) UIView* preview;

@end

@implementation QHVCEditTestPlayerVC

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
         self.timeline = timeline;
    }
    return self;
}

- (void)createPlayer
{
    QHVCEditPlayer* player = [[QHVCEditPlayer alloc] initWithTimeline:nil];
    player = [[QHVCEditPlayer alloc] initWithTimeline:self.timeline];
    
    if (player)
    {
        self.player = player;
        [self.player setDelegate:self];
    }
}

- (void)free
{
    if (self.player)
    {
        [self.player free];
    }
    
    if (self.timeline)
    {
        [self.timeline free];
    }
}

- (void)setPreview
{
    if (!self.preview)
    {
        [self.player setPreview:nil];
        [self.timeline setOutputWidth:1280 height:720];
        [self.player setPreview:nil];
        
        self.preview = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 1280, 720)];
        [self.view addSubview:self.preview];
    }
    
    [self.player setPreview:self.preview];
    [self.player setPreview:self.preview];
    [self.player setPreviewFillMode:QHVCEditFillModeAspectFit];
    [self.player setPreviewBgColor:@"FF000000"];
}

- (void)resetPlayer
{
    NSInteger curTime = [self.player getCurrentTimestamp];
    [self.player resetPlayer:curTime];
}

- (void)refreshPlayer
{
    [self.player refreshPlayer:YES forceRefresh:NO completion:nil];
    [self.player refreshPlayer:NO forceRefresh:NO completion:nil];
    [self.player refreshPlayer:NO forceRefresh:YES completion:nil];
}

- (void)playPlayer
{
    if (![self.player isPlaying])
    {
        [self.player playerPlay];
    }
}

- (void)stopPlayer
{
    [self.player playerStop];
}

- (void)seekPlayer
{
    [self.player playerSeekToTime:0 forceRequest:YES complete:^(NSInteger currentTimeMs) {NSLog(@"");}];
}

- (void)playerDuration
{
    [self.player getPlayerDuration];
}

- (void)currentFrame
{
    [self.player getCurrentFrame];
}

#pragma mark - Delegate

- (void)onPlayerFirstFrameDidRendered
{
}

- (void)onPlayerPlayComplete
{
}

@end

#pragma mark - Player Test Interface

QHVCEditTimeline* QHVCEditTestPlayerCreateTimeline()
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

int QHVCEditTestPlayerInternal()
{
    QHVCEditTimeline* errorTimeline = [[QHVCEditTimeline alloc] initTimeline];
    QHVCEditPlayerInternal* errorInternal = [[QHVCEditPlayerInternal alloc] initWithTimeline:errorTimeline];
    UIView* errorPreview = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 1280, 720)];
    [errorInternal setPreview:errorPreview];
    
    QHVCEditTimeline* errorTimeline2 = QHVCEditTestPlayerCreateTimeline();
    QHVCEditPlayerInternal* errorInternal2 = [[QHVCEditPlayerInternal alloc] initWithTimeline:errorTimeline2];
    [errorTimeline2 free];
    UIView* errorPreview2 = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 1280, 720)];
    [errorInternal2 setPreview:errorPreview2];
    
    
    UIView* preview = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 1280, 720)];
    QHVCEditTimeline* timeline = QHVCEditTestPlayerCreateTimeline();
    QHVCEditPlayerInternal* playerInternal = [[QHVCEditPlayerInternal alloc] initWithTimeline:timeline];
    [playerInternal setPreview:preview];
    
    [playerInternal playerSeekToTime:600 forceRequest:YES complete:^(NSInteger currentTimeMs)
     {
         [playerInternal refreshPlayer:NO forceRefresh:NO completion:^{
             [playerInternal playerPlay];
         }];
    }];
    
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, 2*1000*1000*1000);
    dispatch_semaphore_wait(semaphore, t);
    
    return 0;
}

int QHVCEditTestPlayerAll()
{
    QHVCEditTimeline* timeline = QHVCEditTestPlayerCreateTimeline();
    QHVCEditTestPlayerVC* playerVC = [[QHVCEditTestPlayerVC alloc] initWithTimeline:timeline];
    QHVCEDIT_TEST_OBJECT(playerVC);

    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, 0.5*1000*1000*1000);

    [playerVC createPlayer];
    [playerVC setPreview];
    [playerVC playPlayer];
    [playerVC playerDuration];

    [playerVC refreshPlayer];
    [playerVC stopPlayer];

    [playerVC seekPlayer];
    dispatch_semaphore_wait(semaphore, t);

    [playerVC refreshPlayer];
    dispatch_semaphore_wait(semaphore, t);

    [playerVC stopPlayer];
    [playerVC refreshPlayer];
    dispatch_semaphore_wait(semaphore, t);

    [playerVC resetPlayer];
    dispatch_semaphore_wait(semaphore, t);

    [playerVC stopPlayer];
    [playerVC currentFrame];

    [playerVC playPlayer];

    dispatch_time_t end = dispatch_time(DISPATCH_TIME_NOW, 2*1000*1000*1000);
    dispatch_semaphore_wait(semaphore, end);

    [playerVC free];
    [playerVC resetPlayer];
    
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestPlayerInternal());
    
    return 0;
}
