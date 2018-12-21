//
//  QHVCEditCoreImageOutput.m
//  QHVCEditKit
//
//  Created by liyue-g on 2017/12/11.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import "QHVCEditCIOutput.h"
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import <GLKit/GLKit.h>

@interface QHVCEditCIOutput ()
{
//    CVPixelBufferRef pixelbuffer;
}

@property (nonatomic, assign) BOOL disable;
@property (nonatomic, weak)   id<QHVCEditCIOutputDelegate> delegate;
@property (nonatomic, assign) CGSize outputSize;
@property (nonatomic, assign) CGFloat frameWidth;
@property (nonatomic, assign) CGFloat frameHeight;
@property (nonatomic, retain) CIContext* ciContext;

@end

@implementation QHVCEditCIOutput

- (void)dealloc
{
    self.disable = YES;
    _ciContext = nil;
}

- (instancetype)initWithOutputSize:(CGSize)size
{
    if (!(self = [super init]))
    {
        return nil;
    }
    
    self.outputSize = size;
    [self initContext];
    return self;
}

- (void)initContext
{
    if (!self.ciContext)
    {
        [EAGLContext setCurrentContext:nil];
        self.ciContext = [CIContext contextWithOptions:nil];
//        self.ciContext = [CIContext contextWithOptions:@{kCIContextUseSoftwareRenderer:@(NO)}];
    }
}

- (void)setDelegate:(id<QHVCEditCIOutputDelegate>)delegate
{
    _delegate = delegate;
}

- (void)setDisable
{
    self.disable = YES;
}

- (void)processImage:(CIImage *)image timestampMs:(NSInteger)timestampMs userData:(id)userData
{
    if (!self.disable)
    {
        CGRect sourceExtent = image.extent;
        if (image)
        {            
            int width = CGRectGetWidth(sourceExtent);
            int height = CGRectGetHeight(sourceExtent);
            CVPixelBufferRef pixelbuffer = [QHVCEditUtils createPixelbuffer:width height:height type:QHVCEditUtilsPixelbufferType_NV12];
            [_ciContext render:image toCVPixelBuffer:pixelbuffer];
            
            if (self.delegate && [self.delegate respondsToSelector:@selector(onFrameCallback:dataLen:width:height:timestamp:userData:)])
            {
                [self.delegate onFrameCallback:pixelbuffer dataLen:width*height*3/2 width:width height:height timestamp:timestampMs userData:userData];
            }
        }
    }
}

//- (CVPixelBufferRef)createPixelbuffer:(CGFloat)width height:(CGFloat)height
//{
//    OSType type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
//    CVPixelBufferRef dstPixelbuffer = NULL;
//
//    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
//    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
//    CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);
//
//    CVPixelBufferCreate(kCFAllocatorDefault, width, height, type, attrs, &dstPixelbuffer);
//    CFRelease(attrs);
//    CFRelease(empty);
//    return dstPixelbuffer;
//}

//- (void)convertFrame:(CVPixelBufferRef)pixelbuffer frameTime:(int)timestampMs
//{
//    LogDebug(@"frameDidConvert, %d", timestampMs);
//    CVPixelBufferLockBaseAddress(pixelbuffer, 0);
//    
//    int width = (int)CVPixelBufferGetWidth(pixelbuffer);
//    int height = (int)CVPixelBufferGetHeight(pixelbuffer);
//    int bytesPerRow = (int)CVPixelBufferGetBytesPerRow(pixelbuffer);
//    uint8_t* baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(pixelbuffer);
//    
//    int y_stride = width;
//    int u_stride = width/2;
//    int v_stride = width/2;
//    
//    uint8_t* dst = (uint8_t *)malloc(width*height*3/2*sizeof(uint8_t));
//    ARGBToI420(baseAddress, bytesPerRow,
//               dst, y_stride,
//               dst + width*height, u_stride,
//               dst + width*height*5/4, v_stride,
//               width, height);
//    
//    if (self.delegate && [self.delegate respondsToSelector:@selector(onFrameCallback:dataLen:width:height:timestamp:)])
//    {
//        [self.delegate onFrameCallback:dst dataLen:width*height*3/2 width:width height:height timestamp:timestampMs];
//    }
//    CVPixelBufferUnlockBaseAddress(pixelbuffer, 0);
//}

@end
