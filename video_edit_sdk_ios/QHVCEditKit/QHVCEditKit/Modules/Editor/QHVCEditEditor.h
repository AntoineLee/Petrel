//
//  QHVCEditEditor.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/25.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import "QHVCEditCommonDef.h"

@class QHVCEditEffectManager;
@class QHVCEditTimeline;
@class QHVCEditTimelineManager;
@class QHVCEditTrack;
@class QHVCEditTrackClip;

@interface QHVCEditEditor : NSObject

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline;
- (QHVCEditTimeline *)getTimeline;
- (void *)getTimelineHandle;
- (QHVCEditError)free;

- (NSInteger)getTrackIndex;
- (NSInteger)getClipIndex;
- (NSInteger)getEffectIndex;
- (NSInteger)getTransitionIndex;

@property (nonatomic, strong) QHVCEditTimelineManager* timelineMgr;
@property (atomic,    strong) NSMutableDictionary* trackMgrs;
@property (atomic,    strong) NSMutableDictionary* clipMgrs;

//渲染模块使用，effect查找表
@property (nonatomic, strong) NSMutableDictionary* editorEffects;
- (void)addEffect:(QHVCEditEffectManager *)effect effectId:(NSInteger)effectId;
- (void)removeEffectOfId:(NSInteger)effectId;
- (QHVCEditEffectManager *)getEffectOfId:(NSInteger)effectId;

@end
