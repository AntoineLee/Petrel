//
//  QHVCEditConfig.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/23.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditMacroDefs.h"
#import <GLKit/GLKit.h>

@class QHVCEditEffect;

@interface QHVCEditConfig : NSObject

+ (instancetype)sharedInstance;

- (NSString *)getBundlePath;
- (NSString *)cacheDirectory;

- (NSInteger)getTimelineIndex;

- (NSString *)getErrorCodeInfo:(int)errorCode;
- (NSString *)getEffectBasicInfo:(QHVCEditEffect *)effect;
- (NSString *)printEffectDetail:(QHVCEditEffect *)effect;

- (int)getVEFilterType:(QHVCEditEffect *)effect; //0视频，1音频

//eaglContext
- (EAGLContext *)sharedImageProcessingContext;


@end
