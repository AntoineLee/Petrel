//
//  QHVCEditThumbnailInternal.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/23.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditThumbnailInternal.h"
#import <AVFoundation/AVFoundation.h>
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditConfig.h"
#import "ve_interface.h"

void thumbnail_event_cb(HANDLE handle, ve_thumbnail_callback_param* param, int status, void* userExt)
{
    NSMutableDictionary* dict = (__bridge NSMutableDictionary *)(userExt);
    [dict enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSValue* obj, BOOL * _Nonnull stop)
     {
         QHVCEditThumbnailInternal* thumbnail = (QHVCEditThumbnailInternal *)[obj pointerValue];
         [thumbnail onThumbnailEventCB:handle params:param status:status blockIndex:[key intValue]];
     }];
    
    if ((status == VE_THUMBNAIL_END) || (status == VE_THUMBNAIL_CANCEL) || (status == VE_THUMBNAIL_ERR))
    {
        CFBridgingRelease(userExt);
    }
}

@interface QHVCEditThumbnailInternal ()
{
    HANDLE _thumbnailHandle;
}

@property (nonatomic, retain) NSMutableDictionary* blockDict;
@property (nonatomic, assign) NSInteger blockCount;

@end

@implementation QHVCEditThumbnailInternal

#pragma mark - Public Methods

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        QHVCEditError error = [self createThumbnailHandle];
        if (error == QHVCEditErrorNoError)
        {
            self.blockDict = [[NSMutableDictionary alloc] initWithCapacity:0];
        }
    }
    
    return self;
}

- (QHVCEditError)free
{
    [self cancelAllThumbnailRequest];
    ve_thumbnail_free(_thumbnailHandle);
    
    [self.blockDict removeAllObjects];
    self.blockDict = nil;
    self.blockCount = 0;
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)requestThumbnailFromFile:(NSString *)filePath
                                    width:(NSInteger)width
                                   height:(NSInteger)height
                                startTime:(NSInteger)startTime
                                  endTime:(NSInteger)endTime
                                    count:(NSInteger)count
                             dataCallback:(QHVCEditThumbnailCallback)callback
{
    //参数检查
    if (!callback)
    {
        LogWarn(@"thumbnail request, dataCallback is null");
        return QHVCEditErrorNoError;
    }
    
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        LogError(@"thumbnail request error, filePath is null");
        return QHVCEditErrorParamError;
    }
    
    if (width <= 0)
    {
        LogError(@"thumbnail request error, width <= 0");
        return QHVCEditErrorParamError;
    }
    
    if (height <= 0)
    {
        LogError(@"thumbnail request error, height <= 0");
        return QHVCEditErrorParamError;
    }
    
    if (startTime > endTime)
    {
        LogError(@"thumbnail request error, startTime > endTime");
        return QHVCEditErrorParamError;
    }
    
    if (count <= 0)
    {
        LogError(@"thumbnail request error, count <= 0");
        return QHVCEditErrorParamError;
    }
    
    BOOL ret = [self nativeThumbnailCheck:filePath];
    if (ret)
    {
        //用系统API获取缩略图
        return [self nativeRequestThumbnailFromFile:filePath
                                              width:width
                                             height:height
                                          startTime:startTime
                                            endTime:endTime
                                              count:count
                                           complete:callback];
    }
    else
    {
        //用video_edit API获取缩略图
        return [self videoEditRequestThumbnailFromFile:filePath
                                                 width:(int)width
                                                height:(int)height
                                             startTime:startTime
                                               endTime:endTime
                                                 count:count
                                              complete:callback];
    }
}

- (QHVCEditError)requestThumbnailFromFile:(NSString *)filePath
                                    width:(NSInteger)width
                                   height:(NSInteger)height
                                timestamp:(NSInteger)timeMs
                             dataCallback:(QHVCEditThumbnailCallback)callback
{ 
    //参数检查
    if (!callback)
    {
        LogWarn(@"thumbnail request, dataCallback is null");
        return QHVCEditErrorNoError;
    }
    
    if ([QHVCEditUtils stringIsNull:filePath])
    {
        LogError(@"thumbnail request error, filePath is null");
        return QHVCEditErrorParamError;
    }
    
    if (width <= 0)
    {
        LogError(@"thumbnail request error, width <= 0");
        return QHVCEditErrorParamError;
    }
    
    if (height <= 0)
    {
        LogError(@"thumbnail request error, height <= 0");
        return QHVCEditErrorParamError;
    }
    
    BOOL ret = [self nativeThumbnailCheck:filePath];
    if (ret)
    {
        //用系统API获取缩略图
        return [self nativeRequestThumbnailFromFile:filePath width:width height:height timestamp:timeMs complete:callback];
    }
    else
    {
        //用video_edit API获取缩略图
        return [self videoEditRequestThumbnailFromFile:filePath width:(int)width height:(int)height startTime:timeMs endTime:timeMs+1 count:1 complete:callback];
    }
}

- (QHVCEditError)cancelAllThumbnailRequest
{
    return QHVCEditErrorNoError;
}

#pragma mark - Native Methods

- (BOOL)nativeThumbnailCheck:(NSString *)filePath
{
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    AVURLAsset *asset = [[AVURLAsset alloc] initWithURL:fileURL options:nil];
    NSArray* array = [asset tracksWithMediaType:AVMediaTypeVideo];
    CGImageRef image = nil;
    
    if ([array count] > 0)
    {
        //视频文件
        AVAssetImageGenerator* assetImageGenerator = [[AVAssetImageGenerator alloc] initWithAsset:asset];
        assetImageGenerator.appliesPreferredTrackTransform = YES;
        assetImageGenerator.apertureMode = AVAssetImageGeneratorApertureModeEncodedPixels;
        image = [assetImageGenerator copyCGImageAtTime:CMTimeMake(0, 600) actualTime:nil error:nil];
    }
    else
    {
        //图片文件
        NSDictionary* properties = [NSDictionary dictionaryWithObject:[NSDictionary dictionaryWithObject:@0 forKey:(NSString *)kCGImagePropertyGIFLoopCount]
                                                               forKey:(NSString *)kCGImagePropertyGIFDictionary];
        CGImageSourceRef imgRef = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:filePath], (CFDictionaryRef)properties);
        image = CGImageSourceCreateImageAtIndex(imgRef, 0, (CFDictionaryRef)properties);
        if (imgRef)
        {
            CFRelease(imgRef);
        }
    }
    
    if (image)
    {
        CGImageRelease(image);
        return YES;
    }
    else
    {
        return NO;
    }
}

- (QHVCEditError)nativeRequestThumbnailFromFile:(NSString *)filePath
                                          width:(NSInteger)width
                                         height:(NSInteger)height
                                      startTime:(NSInteger)startTime
                                        endTime:(NSInteger)endTime
                                          count:(NSInteger)count
                                       complete:(QHVCEditThumbnailCallback)complete
{
    //计算时间数组
    NSMutableArray* times = [[NSMutableArray alloc] initWithCapacity:0];
    float total = (endTime - startTime) / 1000;
    float interval = total/count;
    int range = interval*600/2;
    float timestamp = (float)startTime/1000;
    
    for (int i = 0; i < count; i ++)
    {
        CMTime time = CMTimeMakeWithSeconds(timestamp, 600);
        [times addObject:[NSValue valueWithCMTime:time]];
        timestamp += interval;
    }
    
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    AVURLAsset *asset = [[AVURLAsset alloc] initWithURL:fileURL options:nil];
    NSArray* array = [asset tracksWithMediaType:AVMediaTypeVideo];
    
    if ([array count] > 0)
    {
        //视频文件
        [self nativeRequestVideoThumbnail:filePath width:width height:height times:times range:range complete:complete];
    }
    else
    {
        //图片
        [self nativeRequestImageThumbnail:filePath width:width height:height times:times range:range complete:complete];
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)nativeRequestThumbnailFromFile:(NSString *)filePath
                                          width:(NSInteger)width
                                         height:(NSInteger)height
                                      timestamp:(NSInteger)timestampMs
                                       complete:(QHVCEditThumbnailCallback)complete
{
    NSMutableArray* times = [[NSMutableArray alloc] initWithCapacity:0];
    CMTime time = CMTimeMakeWithSeconds(timestampMs/1000.0, 600);
    [times addObject:[NSValue valueWithCMTime:time]];
    
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    AVURLAsset *asset = [[AVURLAsset alloc] initWithURL:fileURL options:nil];
    NSArray* array = [asset tracksWithMediaType:AVMediaTypeVideo];
    int range = 0.05*600/2;;
    
    if ([array count] > 0)
    {
        //视频文件
        [self nativeRequestVideoThumbnail:filePath width:width height:height times:times range:range complete:complete];
    }
    else
    {
        //图片
        [self nativeRequestImageThumbnail:filePath width:width height:height times:times range:range complete:complete];
    }
    
    return QHVCEditErrorNoError;
}

- (void)nativeRequestVideoThumbnail:(NSString *)filePath
                              width:(NSInteger)width
                             height:(NSInteger)height
                              times:(NSArray<NSValue *>*)times
                              range:(float)range
                           complete:(QHVCEditThumbnailCallback)complete
{
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    AVURLAsset *asset = [[AVURLAsset alloc] initWithURL:fileURL options:nil];
    AVAssetImageGenerator* assetImageGenerator = [[AVAssetImageGenerator alloc] initWithAsset:asset];
    assetImageGenerator.appliesPreferredTrackTransform = YES;
    assetImageGenerator.apertureMode = AVAssetImageGeneratorApertureModeEncodedPixels;
    assetImageGenerator.maximumSize = CGSizeMake(width*2, height*2);
    
    //需要指定一个搜寻范围
    __block int index = 0;
    __block int preIndex = 0;
    
    float scale = asset.duration.timescale;
    assetImageGenerator.requestedTimeToleranceBefore = CMTimeMake(range, scale);
    assetImageGenerator.requestedTimeToleranceAfter = CMTimeMake(range, scale);
    [assetImageGenerator generateCGImagesAsynchronouslyForTimes:times completionHandler:^(CMTime requestedTime, CGImageRef  _Nullable image, CMTime actualTime, AVAssetImageGeneratorResult result, NSError * _Nullable error)
     {
         LogInfo(@"on thumbnail %@, index = %d", error ? @"failed":@"success", index);
         if (error)
         {
             QHVCEDIT_SAFE_BLOCK(complete, nil, QHVCEditErrorRequestThumbnailError);
         }
         else
         {
             UIImage* img = [[UIImage alloc] initWithCGImage:image scale:2 orientation:UIImageOrientationUp];
             QHVCEditThumbnailItem* item = [[QHVCEditThumbnailItem alloc] init];
             item.thumbnail = img;
             item.index = index;
             item.videoPath = filePath;
             QHVCEDIT_SAFE_BLOCK(complete, item, QHVCEditErrorNoError);
         }
         
         preIndex = index;
         index ++;
     }];
}

- (void)nativeRequestImageThumbnail:(NSString *)filePath
                              width:(NSInteger)width
                             height:(NSInteger)height
                              times:(NSArray<NSValue *>*)times
                              range:(float)range
                           complete:(QHVCEditThumbnailCallback)complete
{
    NSDictionary* properties = [NSDictionary dictionaryWithObject:[NSDictionary dictionaryWithObject:@0 forKey:(NSString *)kCGImagePropertyGIFLoopCount] forKey:(NSString *)kCGImagePropertyGIFDictionary];
    CGImageSourceRef imgRef = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:filePath], (CFDictionaryRef)properties);
    int imgCount = (int)CGImageSourceGetCount(imgRef);
    
    if (imgCount > 1)
    {
        //GIF图
        NSMutableArray* timeArray = [[NSMutableArray alloc] initWithCapacity:0];
        for (int i = 0; i < imgCount; i++)
        {
            NSDictionary* imgInfo = CFBridgingRelease(CGImageSourceCopyPropertiesAtIndex(imgRef, i, (CFDictionaryRef)properties));
            NSDictionary *tmp = [imgInfo valueForKey:(NSString *)kCGImagePropertyGIFDictionary];
            NSNumber* timeValue = [tmp valueForKey:(NSString *)kCGImagePropertyGIFDelayTime];
            [timeArray addObject:timeValue];
        }
        
        for (int i = 0; i < [times count]; i++)
        {
            CMTime requestedTime = [[times objectAtIndex:i] CMTimeValue];
            float requestSecond = (float)requestedTime.value/requestedTime.timescale;
            
            __block float actualSecond = 0;
            __block NSUInteger index = 0;
            [timeArray enumerateObjectsUsingBlock:^(NSNumber* duration, NSUInteger idx, BOOL * _Nonnull stop)
             {
                 if (actualSecond >= requestSecond)
                 {
                     *stop = YES;
                 }
                 else
                 {
                     actualSecond += [duration floatValue];
                 }
                 index = idx;
             }];
            
            NSInteger size = MAX(width*2, height*2);
            CGImageRef currentRef = CGImageSourceCreateThumbnailAtIndex(imgRef, index,
                                                                    (__bridge CFDictionaryRef) @{
                                                                     (NSString *)kCGImageSourceCreateThumbnailFromImageAlways : @YES,
                                                                     (NSString *)kCGImageSourceThumbnailMaxPixelSize : [NSNumber numberWithInt:(int)size],
                                                                     (NSString *)kCGImageSourceCreateThumbnailWithTransform : @YES,
                                                                     });
            UIImage* img = [[UIImage alloc] initWithCGImage:currentRef scale:2 orientation:UIImageOrientationUp];
            LogInfo(@"on thumbnail of index[%d] ret[%@]", i, img ? @"failed":@"success");
            
            if (img)
            {
                QHVCEditThumbnailItem* item = [[QHVCEditThumbnailItem alloc] init];
                item.thumbnail = img;
                item.index = i;
                item.videoPath = filePath;
                CGImageRelease(currentRef);
                QHVCEDIT_SAFE_BLOCK(complete, item, QHVCEditErrorNoError);
            }
            else
            {
                QHVCEDIT_SAFE_BLOCK(complete, nil, QHVCEditErrorRequestThumbnailError);
            }
        }
    }
    else
    {
        //普通图
        NSInteger size = MAX(width*2, height*2);
        CGImageRef currentRef = CGImageSourceCreateThumbnailAtIndex(imgRef, 0,
                                                                    (__bridge CFDictionaryRef) @{
                                                                    (NSString *)kCGImageSourceCreateThumbnailFromImageAlways : @YES,
                                                                    (NSString *)kCGImageSourceThumbnailMaxPixelSize : [NSNumber numberWithInt:(int)size],
                                                                    (NSString *)kCGImageSourceCreateThumbnailWithTransform : @YES,
                                                                    });
        
        UIImage* img = [[UIImage alloc] initWithCGImage:currentRef scale:2 orientation:UIImageOrientationUp];
        for (int i = 0; i < [times count]; i++)
        {
            LogInfo(@"on thumbnail of index[%d] ret[%@]", i, img ? @"success":@"failed");
            if (img)
            {
                QHVCEditThumbnailItem* item = [[QHVCEditThumbnailItem alloc] init];
                item.thumbnail = img;
                item.index = i;
                item.videoPath = filePath;
                QHVCEDIT_SAFE_BLOCK(complete, item, QHVCEditErrorNoError);
            }
            else
            {
                QHVCEDIT_SAFE_BLOCK(complete, nil, QHVCEditErrorRequestThumbnailError);
            }
        }
        CGImageRelease(currentRef);
    }
    
    CFRelease(imgRef);
}

#pragma mark - video_edit Methods

- (QHVCEditError)createThumbnailHandle
{
    _thumbnailHandle = ve_thumbnail_new();
    if (_thumbnailHandle == NULL)
    {
        return QHVCEditErrorRequestThumbnailError;
    }
    return QHVCEditErrorNoError;
}

- (int)imageRotationtoVERotate:(UIImage *)image
{
    int rotate = 0;
    switch (image.imageOrientation)
    {
        case UIImageOrientationUp:
        {
            rotate = 0;
            break;
        }
        case UIImageOrientationRight:
        {
            rotate = 1;
            break;
        }
        case UIImageOrientationDown:
        {
            rotate = 2;
            break;
        }
        case UIImageOrientationLeft:
        {
            rotate = 3;
        }
        default:
        {
            rotate = 0;
            break;
        }
    }
    return rotate;
}

- (QHVCEditError)videoEditRequestThumbnailFromFile:(NSString *)filePath
                                             width:(int)width
                                            height:(int)height
                                         startTime:(NSInteger)startTime
                                           endTime:(NSInteger)endTime
                                             count:(NSInteger)count
                                          complete:(QHVCEditThumbnailCallback)complete
{
    //计算图片旋转角度
    int rotate = 0;
    UIImage* image = [UIImage imageWithContentsOfFile:filePath];
    if (image)
    {
        rotate = [self imageRotationtoVERotate:image];
    }
    
    //video_edit 方法获取缩略图
    ve_thumbnail_param thumbnailParam;
    memset(&thumbnailParam, 0, sizeof(ve_thumbnail_param));
    thumbnailParam.filename = [filePath UTF8String];
    thumbnailParam.width = width;
    thumbnailParam.height = height;
    thumbnailParam.start_time = (int)startTime;
    thumbnailParam.end_time = (int)endTime;
    thumbnailParam.count = (int)count;
    thumbnailParam.rotate = rotate;
    thumbnailParam.path = [[[QHVCEditConfig sharedInstance] cacheDirectory] UTF8String];
    thumbnailParam.callback = thumbnail_event_cb;

    NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity:0];
    [dict setValue:[NSValue valueWithPointer:(__bridge void *)self] forKey:[[NSString alloc] initWithFormat:@"%ld", (long)self.blockCount]];
    thumbnailParam.userExt = (void *)CFBridgingRetain(dict);

    [self.blockDict setObject:complete forKey:@(self.blockCount)];
    self.blockCount++;

    int ret = ve_thumbnail_get(_thumbnailHandle, &thumbnailParam);
    if (ret != 0)
    {
        LogError(@"QHVCEditThumbnail getVideoThumbnailFromFile error = %d", ret);
        return QHVCEditErrorRequestThumbnailError;
    }

    return QHVCEditErrorNoError;
}

- (void)onThumbnailEventCB:(void *)thumbnailHandle params:(void *)params status:(int)status blockIndex:(int)blockIndex
{
    LogInfo(@"on thumbnail of index[%d] ret[%d]", blockIndex, status);
    QHVCEditThumbnailCallback callbackBlock = [self.blockDict objectForKey:@(blockIndex)];
    if (callbackBlock)
    {
        VE_THUMBNAIL_STATUS state = (VE_THUMBNAIL_STATUS)status;
        QHVCEditError error = QHVCEditErrorNoError;
        __block QHVCEditThumbnailItem* thumbnailItem = [[QHVCEditThumbnailItem alloc] init];
        
        //读取缩略图
        dispatch_block_t readBlock = ^{
            ve_thumbnail_callback_param* paramItem = (ve_thumbnail_callback_param *)params;
            
            thumbnailItem.videoPath = [NSString stringWithCString:paramItem->mp4_file encoding:NSUTF8StringEncoding];
            thumbnailItem.index = paramItem->index;
            
            if (paramItem->output_jpegs && paramItem->output_jpegs_len)
            {
                NSData* data = [NSData dataWithBytes:paramItem->output_jpegs length:paramItem->output_jpegs_len];
                UIImage* image = [UIImage imageWithData:data];
                
                VE_ROTATE rotate = (VE_ROTATE)paramItem->rotate;
                image = [self rotateImage:image veRotate:rotate];
                thumbnailItem.thumbnail = image;
            }
        };
        
        switch (state)
        {
            case VE_THUMBNAIL_PROCESSING:
            {
                //正常
                LogInfo(@"Get thumbnail");
                readBlock();
                break;
            }
            case VE_THUMBNAIL_END:
            {
                //结束
                LogInfo(@"Get thumbnail finished");
                readBlock();
                break;
            }
            case VE_THUMBNAIL_CANCEL:
            {
                //取消
                LogInfo(@"Get thumbnail canceled.");
                break;
            }
            case VE_THUMBNAIL_ERR:
            {
                //异常
                LogError(@"Get thumbnail error.");
                error = QHVCEditErrorRequestThumbnailError;
                break;
            }
            default:
                break;
        }
        
        if (callbackBlock)
        {
            callbackBlock(thumbnailItem, error);
        }
        
        if ((status == VE_THUMBNAIL_END) || (status == VE_THUMBNAIL_CANCEL) || (status == VE_THUMBNAIL_ERR))
        {
            [self.blockDict removeObjectForKey:@(blockIndex)];
        }
    }
}

- (UIImage *)rotateImage:(UIImage *)image veRotate:(int)veRotate
{
    UIImageOrientation orientation = UIImageOrientationUp;
    switch (veRotate)
    {
        case VE_ROTATE_0:
        {
            break;
        }
        case VE_ROTATE_90:
        {
            orientation = UIImageOrientationRight;
            break;
        }
        case VE_ROTATE_180:
        {
            orientation = UIImageOrientationDown;
            break;
        }
        case VE_ROTATE_270:
        {
            orientation = UIImageOrientationLeft;
            break;
        }
        default:
            break;
    }
    
    if (orientation == UIImageOrientationUp)
    {
        return image;
    }
    
    long double rotate = 0.0;
    CGRect rect;
    float translateX = 0;
    float translateY = 0;
    float scaleX = 1.0;
    float scaleY = 1.0;
    
    switch (orientation) {
        case UIImageOrientationLeft:
            rotate = M_PI_2;
            rect = CGRectMake(0,0,image.size.height, image.size.width);
            translateX = 0;
            translateY = -rect.size.width;
            scaleY = rect.size.width/rect.size.height;
            scaleX = rect.size.height/rect.size.width;
            break;
        case UIImageOrientationRight:
            rotate = 3 *M_PI_2;
            rect = CGRectMake(0,0,image.size.height, image.size.width);
            translateX = -rect.size.height;
            translateY = 0;
            scaleY = rect.size.width/rect.size.height;
            scaleX = rect.size.height/rect.size.width;
            break;
        case UIImageOrientationDown:
            rotate = M_PI;
            rect = CGRectMake(0,0,image.size.width, image.size.height);
            translateX = -rect.size.width;
            translateY = -rect.size.height;
            break;
        default:
            rotate = 0.0;
            rect = CGRectMake(0,0,image.size.width, image.size.height);
            translateX = 0;
            translateY = 0;
            break;
    }
    
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context =UIGraphicsGetCurrentContext();
    //做CTM变换
    CGContextTranslateCTM(context, 0.0, rect.size.height);
    CGContextScaleCTM(context, 1.0, -1.0);
    CGContextRotateCTM(context, rotate);
    CGContextTranslateCTM(context, translateX,translateY);
    
    CGContextScaleCTM(context, scaleX,scaleY);
    //绘制图片
    CGContextDrawImage(context, CGRectMake(0,0,rect.size.width, rect.size.height), image.CGImage);
    
    UIImage *newPic =UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return newPic;
}

@end
