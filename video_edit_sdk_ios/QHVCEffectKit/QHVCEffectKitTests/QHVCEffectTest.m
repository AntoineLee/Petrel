//
//  QHVCEffectTest.m
//  QHVCEffectKitTests
//
//  Created by liyue-g on 2018/12/4.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEffectTest.h"
#import <UIKit/UIKit.h>
#import "QHVCEffectBase+Process.h"
#import "QHVCEffect.h"
#import "QHVCEffectUtils.h"
#import "QHVCEffectVideoTransitionList.h"

#define QHVCEFFECT_TEST_RETURN_VALUE(expression) {\
if(expression != 0) \
return -1; \
}

#define QHVCEFFECT_TEST_OBJECT(expression) {\
if(!expression) \
return -1; \
}


int QHVCEffectTestAll(CIImage* inputImage)
{
    QHVCEFFECT_TEST_RETURN_VALUE(QHVCEffectTestUtils());
    QHVCEFFECT_TEST_RETURN_VALUE(QHVCEffectTestFilter(inputImage));
    QHVCEFFECT_TEST_RETURN_VALUE(QHVCEffectTestSticker(inputImage));
    QHVCEFFECT_TEST_RETURN_VALUE(QHVCEffectTestVideoTransfer(inputImage));
    QHVCEFFECT_TEST_RETURN_VALUE(QHVCEffectTestMix(inputImage));
    QHVCEFFECT_TEST_RETURN_VALUE(QHVCEffectTestVideoTransition(inputImage));
    QHVCEFFECT_TEST_RETURN_VALUE(QHVCEffectTestBase(inputImage));
    return 0;
}

NSArray<QHVCEffectVideoTransferParam *>* QHVCEffectTestVideoTransferParam(QHVCEffectVideoTransferCurveType curveTyp)
{
    QHVCEffectVideoTransferParam* alpha = [[QHVCEffectVideoTransferParam alloc] init];
    [alpha setTransferType:QHVCEffectVideoTransferTypeAlpha];
    [alpha setCurveType:curveTyp];
    [alpha setStartValue:0.5];
    [alpha setEndValue:1.0];
    [alpha setStartTime:0];
    [alpha setEndTime:1000];
    
    QHVCEffectVideoTransferParam* scale = [[QHVCEffectVideoTransferParam alloc] init];
    [scale setTransferType:QHVCEffectVideoTransferTypeScale];
    [scale setCurveType:curveTyp];
    [scale setStartValue:0.2];
    [scale setEndValue:1.0];
    [scale setStartTime:0];
    [scale setEndTime:1000];
    
    QHVCEffectVideoTransferParam* offsetX = [[QHVCEffectVideoTransferParam alloc] init];
    [offsetX setTransferType:QHVCEffectVideoTransferTypeOffsetX];
    [offsetX setCurveType:curveTyp];
    [offsetX setStartValue:20];
    [offsetX setEndValue:0];
    [offsetX setStartTime:0];
    [offsetX setEndTime:1000];
    
    QHVCEffectVideoTransferParam* offsetY = [[QHVCEffectVideoTransferParam alloc] init];
    [offsetY setTransferType:QHVCEffectVideoTransferTypeOffsetY];
    [offsetY setCurveType:curveTyp];
    [offsetY setStartValue:20];
    [offsetY setEndValue:0];
    [offsetY setStartTime:0];
    [offsetY setEndTime:1000];

    QHVCEffectVideoTransferParam* radian = [[QHVCEffectVideoTransferParam alloc] init];
    [radian setTransferType:QHVCEffectVideoTransferTypeRadian];
    [radian setCurveType:curveTyp];
    [radian setStartValue:M_PI_2];
    [radian setEndValue:0];
    [radian setStartTime:0];
    [radian setEndTime:1000];
    
    NSArray* array = @[alpha, scale, offsetX, offsetY, radian];
    return array;
}

int QHVCEffectTestUtils(void)
{
    [QHVCEffectUtils stringIsNull:@""];
    [QHVCEffectUtils stringIsNull:[NSNull null]];
    [QHVCEffectUtils stringIsNull:@"test"];
    
    NSArray<QHVCEffectVideoTransferParam *>* curveArray = QHVCEffectTestVideoTransferParam(QHVCEffectVideoTransferCurveTypeCurve);
    [QHVCEffectUtils computeAlpha:curveArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetX:curveArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetY:curveArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetScale:curveArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetScale:curveArray startTime:0 endTime:1000 currentTime:0];
    
    NSArray<QHVCEffectVideoTransferParam *>* linearArray = QHVCEffectTestVideoTransferParam(QHVCEffectVideoTransferCurveTypeLinear);
    [QHVCEffectUtils computeAlpha:linearArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetX:linearArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetY:linearArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetScale:linearArray startTime:0 endTime:1000 currentTime:0];
    [QHVCEffectUtils computeOffsetScale:linearArray startTime:0 endTime:1000 currentTime:0];
    
    [QHVCEffectUtils colorForHex:@"FFFFFF"];
    [QHVCEffectUtils colorForHex:@"FFFFFFFF"];
    [QHVCEffectUtils colorForHex:@"#FFFFFFFF"];
    [QHVCEffectUtils colorForHex:@"FFFFFFFFF"];
    
    CIImage* image = [QHVCEffectUtils createImageWithColor:[UIColor blackColor] frame:CGRectMake(0, 0, 100, 100)];
    QHVCEFFECT_TEST_OBJECT(image);
    
    UIImage* testImage = [UIImage imageNamed:@"test_sticker.png"];
    image = [CIImage imageWithCGImage:testImage.CGImage];
    QHVCEFFECT_TEST_OBJECT(image);
    
    CIImage* outImage = [QHVCEffectUtils imageTranslateToZeroPoint:image];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEffectTestFilter(CIImage* inputImage)
{
    UIImage* image = [UIImage imageNamed:@"test_lut.png"];
    CIImage* lutImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEFFECT_TEST_OBJECT(lutImage);
    
    QHVCEffectFilter* filter = [[QHVCEffectFilter alloc] init];
    QHVCEFFECT_TEST_OBJECT(lutImage);
    
    [filter setClutImage:lutImage];
    CIImage* outImage = [filter processImage:inputImage timestamp:0];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    CIImage* scaleImage = [inputImage imageByApplyingTransform:CGAffineTransformMakeScale(0.2, 0.2)];
    outImage = [filter processImage:scaleImage timestamp:0];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEffectTestSticker(CIImage* inputImage)
{
    QHVCEffectSticker* sticker = [[QHVCEffectSticker alloc] init];
    QHVCEFFECT_TEST_OBJECT(sticker);
    
    CIImage* outImage = [sticker processImage:inputImage timestamp:0];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    UIImage* image = [UIImage imageNamed:@"test_sticker.png"];
    CIImage* stickerImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEFFECT_TEST_OBJECT(stickerImage);
    
    [sticker setSticker:stickerImage];
    outImage = [sticker processImage:inputImage timestamp:0];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    NSArray<QHVCEffectVideoTransferParam *>* params = QHVCEffectTestVideoTransferParam(QHVCEffectVideoTransferCurveTypeLinear);
    QHVCEFFECT_TEST_OBJECT(params);
    
    [sticker setVideoTransfer:params];
    outImage = [sticker processImage:inputImage timestamp:0];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEffectTestVideoTransfer(CIImage* inputImage)
{
    NSArray<QHVCEffectVideoTransferParam *>* params = QHVCEffectTestVideoTransferParam(QHVCEffectVideoTransferCurveTypeLinear);
    QHVCEFFECT_TEST_OBJECT(params);
    
    QHVCEffectVideoTransfer* transfer = [[QHVCEffectVideoTransfer alloc] init];
    QHVCEFFECT_TEST_OBJECT(transfer);
    
    [transfer setVideoTransfer:params];
    CIImage* outImage = [transfer processImage:inputImage timestamp:0];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEffectTestMix(CIImage* inputImage)
{
    QHVCEffectMix* mix = [[QHVCEffectMix alloc] init];
    QHVCEFFECT_TEST_OBJECT(mix);
    
    [mix setIntensity:0.5];
    [mix setTopImage:inputImage];
    
    CIImage* outImage = [mix processImage:nil timestamp:0];
    [mix setOutputSize:inputImage.extent.size];
    
    outImage = [mix processImage:inputImage timestamp:0];
    QHVCEFFECT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEffectTestVideoTransition(CIImage* image)
{
    QHVCEffectVideoTransition* transition = [[QHVCEffectVideoTransition alloc] init];
    [transition setProgress:0.5];
    [transition setSecondImage:image];
    
    NSDictionary* dict = [QHVCEffectVideoTransitionList transitionList];
    [dict enumerateKeysAndObjectsUsingBlock:^(NSString* key, id  _Nonnull obj, BOOL * _Nonnull stop)
    {
        [transition setTransitionName:key];
        [transition processImage:image timestamp:0];
    }];
    
    return 0;
}

int QHVCEffectTestBase(CIImage* image)
{
    QHVCEffectBase* base = [[QHVCEffectBase alloc] init];
    [base processImage:image timestamp:0];
    
    return 0;
}
