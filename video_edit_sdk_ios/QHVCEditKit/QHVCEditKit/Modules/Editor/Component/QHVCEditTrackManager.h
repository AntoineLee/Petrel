//
//  QHVCEditTrackManager.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import "QHVCEditCommonDef.h"

@class QHVCEditEffect;
@class QHVCEditTrack;
@class QHVCEditTrackClip;
@class QHVCEditTrackClipManager;

@interface QHVCEditTrackManager : NSObject

//基础方法
- (instancetype)initWithTrack:(QHVCEditTrack *)track timelineHandle:(void *)timelineHandle;
- (QHVCEditTrack *)getTrack;
- (NSInteger)duration;
- (QHVCEditError)setSpeed:(CGFloat)speed;
- (QHVCEditError)setVolume:(NSInteger)volume;

//clip
- (QHVCEditError)appendClip:(QHVCEditTrackClipManager *)clipMgr;
- (QHVCEditError)insertClip:(QHVCEditTrackClipManager *)clipMgr atIndex:(NSInteger)index;
- (QHVCEditError)addClip:(QHVCEditTrackClipManager *)clipMgr atTime:(NSInteger)timeMs;
- (QHVCEditError)moveClip:(NSInteger)fromIndex toIndex:(NSInteger)index;
- (QHVCEditError)changeClipInsertTime:(NSInteger)timeMs clipId:(NSInteger)clipId;
- (QHVCEditError)updateClipParams:(QHVCEditTrackClipManager *)clipMgr;
- (QHVCEditError)deleteClipById:(NSInteger)clipId;
- (QHVCEditError)deleteClipAtIndex:(NSInteger)index;
- (QHVCEditTrackClipManager *)getClipById:(NSInteger)clipId;
- (QHVCEditTrackClipManager *)getClipAtIndex:(NSInteger)index;
- (NSArray<QHVCEditTrackClipManager *>*)getClips;

//转场
- (QHVCEditError)addVideoTransitionToIndex:(NSInteger)clipIndex
                                  duration:(NSInteger)durationMs
                       videoTransitionName:(NSString *)transitionName
                              transitionId:(NSInteger)transitionId
                        easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (QHVCEditError)updateVideoTransitionAtIndex:(NSInteger)clipIndex
                                     duration:(NSInteger)durationMs
                          videoTransitionName:(NSString *)transitionName
                           easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (QHVCEditError)deleteVideoTransition:(NSInteger)index;

//特效
- (QHVCEditError)addEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)deleteEffectById:(NSInteger)effectId;
- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect;
- (QHVCEditEffect *)getEffectById:(NSInteger)effectId;
- (NSArray<QHVCEditEffect *>*)getEffects:(QHVCEditTrack *)track;

@end
