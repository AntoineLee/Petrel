
#import <Foundation/Foundation.h>
#import "QHVCEditUtils.h"

typedef NS_ENUM(NSUInteger, QHVCEditUtilsPixelbufferType)
{
    QHVCEditUtilsPixelbufferType_NV12,
    QHVCEditUtilsPixelbufferType_BGRA,
};

@interface QHVCEditUtils(QHVCEditBufferUtils)

/**
 创建pixelbuffer

 @param width 宽度
 @param height 高度
 @param bufferType 数据类型
 @return pixelbuffer
 */
+ (CVPixelBufferRef)createPixelbuffer:(int)width height:(int)height type:(QHVCEditUtilsPixelbufferType)bufferType;

@end
