
#import "QHVCEditUtils.h"

NS_ASSUME_NONNULL_BEGIN

@interface QHVCEditUtils(QHVCEditTransformUtils)


/**
 图片坐标归零
 
 @param inputImage 输入图
 @return 归零图
 */
+ (CIImage *)imageTranslateToZeroPoint:(CIImage *)inputImage;

@end

NS_ASSUME_NONNULL_END
