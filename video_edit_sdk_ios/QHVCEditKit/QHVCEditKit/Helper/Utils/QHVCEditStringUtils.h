
#import <Foundation/Foundation.h>
#import "QHVCEditUtils.h"

@interface QHVCEditUtils(QHVCEditStringUtils)

/**
 *字符串比较大小
 */
#define REK_V1_GREATER_THAN_V2(v1,v2) ([v1 compare:v2 options:NSNumericSearch] != NSOrderedAscending)

/**
 * 判断字符串是否为空
 */
+ (BOOL)stringIsNull:(NSString *)parameter;


@end
