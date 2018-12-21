
#import <Foundation/Foundation.h>
#import "QHVCEditUtils.h"

@interface QHVCEditUtils(QHVCEditEasingFunctionUtils)

/**
 缓动函数 cubicEaseIn
 示例：[QHVCEditUtils cubicEaseIn:0.0 endValue:1.0 duration:1.0 curTime:progress];
 
 @param beginValue 初始值
 @param endValue 结束值
 @param duration 持续时长
 @param time 当前时间
 @return 返回值
 */
+ (float)cubicEaseIn:(float)beginValue endValue:(float)endValue duration:(float)duration curTime:(float)time;


/**
 缓动函数 cubicEaseOut

 @param beginValue 初始值
 @param endValue 结束值
 @param duration 持续时长
 @param time 当前时间
 @return 返回值
 */
+ (float)cubicEaseOut:(float)beginValue endValue:(float)endValue duration:(float)duration curTime:(float)time;


/**
 缓动函数 cubicEaseInOut

 @param begainValue 初始值
 @param endValue 结束值
 @param duration 持续时间
 @param time 当前时间
 @return 返回值
 */
+ (float)cubicEaseInOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time;


/**
 缓动函数 quintEaseInOut
 示例: [QHVCEditUtils quintEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:progress];

 @param beginValue 初始值
 @param endValue 结束值
 @param duration 持续时长
 @param time 当前时间
 @return 返回值
 */
+ (float)quintEaseInOut:(float)beginValue endValue:(float)endValue duration:(float)duration curTime:(float)time;


/**
 缓动函数 quartEaseInOut

 @param begainValue 初始化
 @param endValue 结束值
 @param duration 持续时长
 @param time 当前时间
 @return 返回值
 */
+ (float)quartEaseInOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time;


/**
 缓动函数 quadEaseInOut

 @param begainValue 初始值
 @param endValue 结束值
 @param duration 持续时长
 @param time 当前时间
 @return 返回值
 */
+ (float)quadEaseInOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time;


/**
 缓动函数 quadEaseOut

 @param begainValue 初始值
 @param endValue 结束值
 @param duration 持续时长
 @param time 当前时间
 @return 返回值
 */
+ (float)quadEaseOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time;

@end
