//
//  QHVCEditTimelineManager.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import "QHVCEditCommonDef.h"

@class QHVCEditTimeline;
@class QHVCEditTrack;
@class QHVCEditEffect;

@interface QHVCEditTimelineManager : NSObject

//初始化、释放
- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline;
- (QHVCEditError)freeTimeline;
- (void *)getTimelineHandle;

//属性
- (QHVCEditError)setOutputWidth:(NSInteger)width height:(NSInteger)height;
- (CGSize)outputSize;

- (QHVCEditError)setTimelineOutputBgColor:(NSString *)bgColor;
- (NSString *)outputBgColor;

- (QHVCEditError)setTimelineOutputFps:(NSInteger)fps;
- (NSInteger)outputFps;

- (QHVCEditError)setTimelineOutputBitrate:(NSInteger)bitrate;
- (NSInteger)outputBitrate;

- (QHVCEditError)setTimelineSpeed:(CGFloat)speed;
- (CGFloat)speed;

- (QHVCEditError)setTimelineVolume:(NSInteger)volume;
- (NSInteger)volume;

- (QHVCEditError)setTimelineOutputPath:(NSString *)filePath;
- (NSString *)outputPath;

- (NSInteger)duration;

//轨道相关
- (QHVCEditError)appendTrack:(QHVCEditTrack *)track;
- (QHVCEditError)deleteTrackById:(NSInteger)trackId;

//特效相关
- (QHVCEditError)addEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)deleteEffectById:(NSInteger)effectId;
- (QHVCEditEffect *)getEffectById:(NSInteger)effectId;
- (NSArray<QHVCEditEffect *>*)getEffects;

@end
