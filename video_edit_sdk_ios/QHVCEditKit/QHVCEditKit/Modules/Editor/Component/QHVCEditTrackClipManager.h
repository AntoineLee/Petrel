//
//  QHVCEditTrackClipManager.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditCommonDef.h"

@class QHVCEditTrackClip;
@class QHVCEditEffect;

@interface QHVCEditTrackClipManager : NSObject

- (instancetype)initWithClip:(QHVCEditTrackClip *)clip
                     toTrack:(NSInteger)trackId
              ofTimelineHandle:(void *)timelineHandle;
- (NSInteger)getTrackId;
- (QHVCEditTrackClip *)getClip;
- (NSInteger)duration;

- (NSInteger)insertTime;
- (void)updateInsertTime:(NSInteger)time;

- (QHVCEditError)addEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)deleteEffectById:(NSInteger)effectId;
- (QHVCEditEffect *)getEffectById:(NSInteger)effectId;
- (NSArray<QHVCEditEffect *>*)getEffects;

- (void)addVideoTransition:(NSInteger)transitionId
                  duration:(NSInteger)durationMs
            transitionName:(NSString *)transitionName
        easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (void)updateVideoTransition:(NSInteger)durationMs
               transitionName:(NSString *)transitionName
           easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (void)deleteVideoTransition;

@property (nonatomic, assign) NSInteger transitionId;
@property (nonatomic, assign) NSInteger transitionDuration;
@property (nonatomic, strong) NSString* transitionName;
@property (nonatomic, assign) BOOL haveTransition;
@property (nonatomic, assign) QHVCEditEasingFunctionType transitionEasingFunctionType;

@end
