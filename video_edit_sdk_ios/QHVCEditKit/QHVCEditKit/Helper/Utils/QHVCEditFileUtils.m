
#import "QHVCEditUtilsSet.h"
#import "QHVCEditLogger.h"

@implementation QHVCEditUtils(QHVCEditFileUtils)

+ (BOOL)isFileExisted:(NSString*)filepath isDirectory:(BOOL)isDir
{
    NSFileManager* fileMgr = [NSFileManager defaultManager];
    BOOL exist = [fileMgr fileExistsAtPath:filepath isDirectory:&isDir];
    return exist;
}

+ (BOOL)removeFile:(NSString*)filepath
{
    NSFileManager *fileMgr = [NSFileManager defaultManager];
    
    NSError* error;
    return [fileMgr removeItemAtPath:filepath error:&error];
}

+ (void)createDirectoryAtPath:(NSString *)path
{
    NSFileManager* fileMgr = [NSFileManager defaultManager];
    BOOL dirExist = [fileMgr fileExistsAtPath:path];
    if (!dirExist) {
        NSError* error = nil;
        [fileMgr createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:&error];
    }
}

@end
