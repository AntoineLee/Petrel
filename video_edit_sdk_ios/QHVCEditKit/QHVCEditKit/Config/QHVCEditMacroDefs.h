//
//  QHVCEditMacroDefs.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/23.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#ifndef QHVCEditMacroDefs_h
#define QHVCEditMacroDefs_h

//block回调
#define QHVCEDIT_SAFE_BLOCK(block, ...) if((block)) { block(__VA_ARGS__); }

//在主线程回调block
#define QHVCEDIT_SAFE_BLOCK_IN_MAIN_QUEUE(block, ...) if((block)) {\
if ([NSThread isMainThread]) {\
block(__VA_ARGS__);\
}\
else {\
dispatch_async(dispatch_get_main_queue(), ^{\
block(__VA_ARGS__);\
});\
}\
}

//当前系统版本号比较
#define QHVCEDIT_SYSTEM_VERSION_GREATER_THAN(v) ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

//block内外self宏，需成对使用
#define QHVCEDIT_WEAK_SELF    __weak __typeof(&*self) weakSelf = self;
#define QHVCEDIT_STRONG_SELF  __strong __typeof(&*self) self = weakSelf;

//包名
#define QHVCEDIT_FRAMEWORK                      @"QHVCEditKit.framework"

//字段定义
#define QHVCEDIT_DEFINE_EFFECT_ID                     @"effectId"
#define QHVCEDIT_DEFINE_START_TIME                    @"startTime"
#define QHVCEDIT_DEFINE_END_TIME                      @"endTime"
#define QHVCEDIT_DEFINE_EFFECT_TYPE                   @"effectType"
#define QHVCEDIT_DEFINE_MAIN_FRAME_PRCESSER           @"mainFrameProcesser"
#define QHVCEDIT_DEFINE_TRANSITION_FRAME_PRCESSER     @"transitionFrameProcesser"
#define QHVCEDIT_DEFINE_TRANSITION_PRCESSER           @"transitionProcesser"
#define QHVCEDIT_DEFINE_TRACK_PRCESSER                @"trackProcesser"

//值定义
#define QHVCEDIT_DEFAULT_FPS                    30
#define QHVCEDIT_DEAFULT_SPEED                  1.0
#define QHVCEDIT_DEFAULT_VOLUME                 100
#define QHVCEDIT_MAX_VOLUME                     200

#endif /* QHVCEditMacroDefs_h */
