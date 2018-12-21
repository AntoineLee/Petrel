//
//  QHVCEditEditor+Clip.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditEditor.h"
#import "QHVCEditCommonDef.h"

@class QHVCEditTrackClip;
@class QHVCEditTrack;
@class QHVCEditEffect;
@class QHVCEditTrackClipManager;

@interface QHVCEditEditor(TrackClip)

- (QHVCEditTrackClipManager *)clipGetManager:(QHVCEditTrackClip *)clip;
- (QHVCEditTrack *)clipGetBelongsTrack:(QHVCEditTrackClip *)clip;
- (NSInteger)clipGetInsertTime:(QHVCEditTrackClip *)clip;
- (NSInteger)clipDuration:(QHVCEditTrackClip *)clip;

- (QHVCEditError)clip:(QHVCEditTrackClip *)clip addEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)clip:(QHVCEditTrackClip *)clip updateEffect:(QHVCEditEffect *)effect;
- (QHVCEditError)clip:(QHVCEditTrackClip *)clip deleteEffectById:(NSInteger)effectId;
- (QHVCEditEffect *)clip:(QHVCEditTrackClip *)clip getEffectById:(NSInteger)effectId;
- (NSArray<QHVCEditEffect *>*)clipGetEffects:(QHVCEditTrackClip *)clip;

@end
