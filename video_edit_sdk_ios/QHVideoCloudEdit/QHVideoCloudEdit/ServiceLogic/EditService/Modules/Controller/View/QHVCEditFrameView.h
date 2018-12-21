//
//  QHVCEditFrameView.h
//  QHVideoCloudToolSet
//
//  Created by deng on 2018/1/18.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef void(^QHVCEditTimelineViewAddAction)(void);
typedef void(^QHVCEditTimelineViewPlayAction)(void);
typedef void(^QHVCEditTimelineViewPauseAction)(void);
typedef void(^QHVCEditTimelineViewSeekAction)(BOOL forceRefresh, NSInteger seekToTime);

@class QHVCEditPlayerBaseVC;
@interface QHVCEditFrameView : UIView

@property (nonatomic,   copy) QHVCEditTimelineViewAddAction addAction;
@property (nonatomic,   copy) QHVCEditTimelineViewPlayAction playAction;
@property (nonatomic,   copy) QHVCEditTimelineViewPauseAction pauseAction;
@property (nonatomic,   copy) QHVCEditTimelineViewSeekAction seekAction;
@property (nonatomic, retain) QHVCEditPlayerBaseVC* playerBaseVC;

- (void)pause;
- (void)play;

@end
