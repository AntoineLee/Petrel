//
//  QHVCEditEditor+Timeline.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditEditor.h"
#import "QHVCEditEffect.h"

@class QHVCEditTrack;
@interface QHVCEditEditor(Timeline)

//timeline属性
- (QHVCEditError)setOutputWidth:(NSInteger)width height:(NSInteger)height;
- (CGSize)outputSize;

- (QHVCEditError)setOutputBgColor:(NSString *)bgColor;
- (NSString *)outputBgColor;

- (QHVCEditError)setOutputFps:(NSInteger)fps;
- (NSInteger)outputFps;

- (QHVCEditError)setOutputBitrate:(NSInteger)bitrate;
- (NSInteger)outputBitrate;

- (QHVCEditError)setSpeed:(CGFloat)speed;
- (CGFloat)speed;

- (QHVCEditError)setVolume:(NSInteger)volume;
- (NSInteger)volume;

- (QHVCEditError)setOutputPath:(NSString *)filePath;
- (NSString *)outputPath;

- (NSInteger)timelineDuration;

//轨道相关
- (QHVCEditError)timelineAppendTrack:(QHVCEditTrack *)track;
- (QHVCEditError)timelineDeleteTrackById:(NSInteger)trackId;
- (QHVCEditTrack *)timelineGetTrackById:(NSInteger)trackId;
- (NSArray<QHVCEditTrack *>*)timelineGetTracks;

//特效相关
- (QHVCEditError)timelineAddEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)timelineDeleteEffectById:(NSInteger)effectId;
- (QHVCEditError)timelineUpdateEffect:(QHVCEditEffect *)effect;
- (QHVCEditEffect *)timelineGetEffectById:(NSInteger)effectId;
- (NSArray<QHVCEditEffect *>*)timelineGetEffects;


@end
