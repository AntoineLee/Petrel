
#import "QHVCEditTransformUtils.h"

@implementation QHVCEditUtils(QHVCEditTransformUtils)

+ (CIImage *)imageTranslateToZeroPoint:(CIImage *)inputImage
{
    CIImage* outputImage = inputImage;
    CGAffineTransform translation = CGAffineTransformMakeTranslation(-inputImage.extent.origin.x, -inputImage.extent.origin.y);
    outputImage = [inputImage imageByApplyingTransform:translation];
    
    return outputImage;
}

@end
