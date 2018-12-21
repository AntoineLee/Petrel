
#import "QHVCEditImageUtils.h"
#import "QHVCEditUtilsSet.h"
#import <AVFoundation/AVFoundation.h>
#import <Photos/Photos.h>

@implementation QHVCEditUtils(QHVCEditImageUtils)


+ (BOOL)isHEICFile:(NSString *)filePath
{
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        return NO;
    }
    
    NSURL* url = [NSURL URLWithString:filePath];
    NSString *ext = [[url lastPathComponent] pathExtension];
    if ([ext isEqualToString:@"heic"] || [ext isEqualToString:@"HEIC"])
    {
        return YES;
    }
    
    return NO;
}

+ (NSString *)heicToJPEG:(NSString *)filePath
{
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        return nil;
    }
    
    NSString* jpegPath = filePath;
    NSDictionary* properties = [NSDictionary dictionaryWithObject:[NSDictionary dictionaryWithObject:@0 forKey:(NSString *)kCGImagePropertyGIFLoopCount] forKey:(NSString *)kCGImagePropertyGIFDictionary];
    CGImageSourceRef imgRef = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:filePath], (CFDictionaryRef)properties);
    if (!imgRef)
    {
        return nil;
    }
    
    CGImageRef currentRef = CGImageSourceCreateThumbnailAtIndex(imgRef, 0, (__bridge CFDictionaryRef) @{
                                                                                                        (NSString *)kCGImageSourceCreateThumbnailFromImageAlways : @YES,
                                                                                                        (NSString *)kCGImageSourceCreateThumbnailWithTransform : @YES,
                                                                                                        });
    if (!currentRef)
    {
        return nil;
    }
    
    jpegPath = [jpegPath stringByDeletingPathExtension];
    jpegPath = [jpegPath stringByAppendingString:@".jpg"];
    UIImage* img = [[UIImage alloc] initWithCGImage:currentRef];
    NSData* jpegData = UIImageJPEGRepresentation(img, 1);
    [jpegData writeToFile:jpegPath atomically:YES];
    CGImageRelease(currentRef);
    
    CFRelease(imgRef);
    
    return jpegPath;
}


@end
