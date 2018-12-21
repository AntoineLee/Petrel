
#import "QHVCEditBufferUtils.h"

@implementation QHVCEditUtils(QHVCEditBufferUtils)

+ (CVPixelBufferRef)createPixelbuffer:(int)width height:(int)height type:(QHVCEditUtilsPixelbufferType)bufferType
{
    OSType type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
    switch (bufferType)
    {
        case QHVCEditUtilsPixelbufferType_NV12:
        {
            type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            break;
        }
        case QHVCEditUtilsPixelbufferType_BGRA:
        {
            type = kCVPixelFormatType_32BGRA;
            break;
        }
    }
    
    CVPixelBufferRef dstPixelbuffer = NULL;
    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);
    
    CVPixelBufferCreate(kCFAllocatorDefault, width, height, type, attrs, &dstPixelbuffer);
    CFRelease(attrs);
    CFRelease(empty);
    return dstPixelbuffer;
}

@end
