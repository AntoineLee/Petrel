
#import <Foundation/Foundation.h>
#import "QHVCEditUtils.h"

@interface QHVCEditUtils(QHVCEditImageUtils)

/**
 是否是HEIC图片文件

 @param filePath 文件路径
 @return 是否是HEIC图片
 */
+ (BOOL)isHEICFile:(NSString *)filePath;


/**
 HEIC图片转JPEG文件

 @param filePath HEIC文件路径
 @return 生成的JPEG文件
 */
+ (NSString *)heicToJPEG:(NSString *)filePath;


@end
