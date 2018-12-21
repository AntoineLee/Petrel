
#import <Foundation/Foundation.h>
#import "QHVCEditUtils.h"

@interface QHVCEditUtils(QHVCEditColorUtils)


/**
 16进制RGB颜色转UIColor

 @param hexColor 16进制RGB颜色
 @param alpha alpha值
 @return UIColor对象
 */
+ (UIColor *)colorForHex:(NSString *)hexColor forAlpha:(CGFloat)alpha;

/**
 16进制ARGB颜色转UIColor

 @param hexColor 16进制ARGB颜色
 @return UIColor对象
 */
+ (UIColor *)colorForHex:(NSString *)hexColor;

/**
 UIColor转16进制ARGB颜色

 @param color UIColor对象
 @return hexColor 16进制ARGB颜色
 */
+ (NSString *)hexStringFromColor:(UIColor *)color;

@end
