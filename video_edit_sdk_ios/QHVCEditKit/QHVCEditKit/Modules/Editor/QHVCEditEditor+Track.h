//
//  QHVCEditEditor+Track.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditEditor.h"

@class QHVCEditEffect;
@interface QHVCEditEditor(Track)

#pragma mark - 基础方法

//文件管理
- (QHVCEditError)track:(QHVCEditTrack*)track updateClipParams:(QHVCEditTrackClip *)clip;
- (QHVCEditError)track:(QHVCEditTrack *)track deleteClipById:(NSInteger)clipId;
- (QHVCEditTrackClip *)track:(QHVCEditTrack *)track getClipById:(NSInteger)clipId;
- (NSArray<QHVCEditTrackClip *>*)trackGetClips:(QHVCEditTrack *)track;

//轨道属性
- (NSInteger)trackDuration:(QHVCEditTrack *)track;
- (QHVCEditError)track:(QHVCEditTrack *)track setSpeed:(CGFloat)speed;
- (QHVCEditError)track:(QHVCEditTrack *)track setVolume:(NSInteger)volume;

//特效
- (QHVCEditError)track:(QHVCEditTrack *)track addEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)track:(QHVCEditTrack *)track deleteEffectById:(NSInteger)effectId;
- (QHVCEditError)track:(QHVCEditTrack *)track updateEffect:(QHVCEditEffect *)effect;
- (QHVCEditEffect *)track:(QHVCEditTrack *)track getEffectById:(NSInteger)effectId;
- (NSArray<QHVCEditEffect *>*)trackGetEffects:(QHVCEditTrack *)track;

#pragma mark - 顺序轨道

//文件管理
- (QHVCEditError)track:(QHVCEditTrack *)track appendClip:(QHVCEditTrackClip *)clip;
- (QHVCEditError)track:(QHVCEditTrack *)track insertClip:(QHVCEditTrackClip *)clip atIndex:(NSInteger)index;
- (QHVCEditError)track:(QHVCEditTrack *)track moveClip:(NSInteger)fromIndex toIndex:(NSInteger)toIndex;
- (QHVCEditError)track:(QHVCEditTrack *)track deleteClipAtIndex:(NSInteger)index;
- (QHVCEditTrackClip *)track:(QHVCEditTrack *)track getClipAtIndex:(NSInteger)index;

//转场
- (QHVCEditError)track:(QHVCEditTrack *)track
addVideoTransitionToIndex:(NSInteger)clipIndex
              duration:(NSInteger)durationMs
   videoTransitionName:(NSString *)transitionName
    easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (QHVCEditError)track:(QHVCEditTrack *)track
updateVideoTransitionAtIndex:(NSInteger)clipIndex
              duration:(NSInteger)durationMs
   videoTransitionName:(NSString *)transitionName
    easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (QHVCEditError)track:(QHVCEditTrack *)track
 deleteVideoTransition:(NSInteger)clipIndex;

#pragma mark - 画中画轨道

//文件管理
- (QHVCEditError)track:(QHVCEditTrack *)track addClip:(QHVCEditTrackClip *)clip atTime:(NSInteger)timeMs;
- (QHVCEditError)track:(QHVCEditTrack *)track changeClipInsertTime:(NSInteger)timeMs clipId:(NSInteger)clipId;

@end
