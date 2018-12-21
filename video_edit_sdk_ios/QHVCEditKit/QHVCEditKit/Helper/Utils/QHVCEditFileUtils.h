
#import <Foundation/Foundation.h>
#import "QHVCEditUtils.h"

@interface QHVCEditUtils(QHVCEditFileUtils)

/**
 * 检测文件是否存在
 */
+ (BOOL)isFileExisted:(NSString*)filepath isDirectory:(BOOL)isDir;

/**
 * 删除文件
 */
+(BOOL)removeFile:(NSString*)filepath;

/**
 在指定路径创建文件夹
 
 @param path 文件夹路径
 */
+ (void)createDirectoryAtPath:(NSString *)path;


@end
