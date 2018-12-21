
#import <Foundation/Foundation.h>
#import "QHVCEditUtils.h"

@interface QHVCEditUtils(QHVCEditTimeUtils)

/**
 * 获取当前时间
 */
+(NSDate *)currentTime;

/**
 * 获取当前时间戳
 */
+ (NSTimeInterval)currentTimestamp;


/**
 * 获取系统时间
 */
+ (NSString *)systemTime;


@end
