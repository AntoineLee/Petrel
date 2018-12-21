//
//  QHVCEditEffectProcesser.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/9.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreImage/CoreImage.h>
#import "QHVCEditRenderParam.h"
#import "QHVCEditEditor.h"

typedef void(^QHVCEditEffectProcesserComplete)(CIImage* outputImage);

@interface QHVCEditEffectProcesser : NSObject

- (instancetype)initWithEditor:(QHVCEditEditor *)editor;

- (void)processClipFrame:(QHVCEditRenderClip *)clip
               timestamp:(NSInteger)timestampMs
                complete:(QHVCEditEffectProcesserComplete)complete;

- (void)processEffect:(CIImage *)inputImage
               effect:(NSArray<QHVCEditRenderEffect *>*)effects
            timestamp:(NSInteger)timestampMs
             complete:(QHVCEditEffectProcesserComplete)complete;

@end
