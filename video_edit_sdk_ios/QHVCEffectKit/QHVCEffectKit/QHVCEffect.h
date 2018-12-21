//
//  QHVCEffect.h
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/10/30.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEffectBase.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark - 视频过渡效果（和画中画、贴图组合使用）

typedef NS_ENUM(NSUInteger, QHVCEffectVideoTransferType)
{
    QHVCEffectVideoTransferTypeAlpha,     //透明度（0-1），默认1.0
    QHVCEffectVideoTransferTypeScale,     //缩放，默认1.0
    QHVCEffectVideoTransferTypeOffsetX,   //x方向相对位移，默认0
    QHVCEffectVideoTransferTypeOffsetY,   //y方向相对位移，默认0
    QHVCEffectVideoTransferTypeRadian,    //旋转弧度值，默认0
};

typedef NS_ENUM(NSUInteger, QHVCEffectVideoTransferCurveType)
{
    QHVCEffectVideoTransferCurveTypeLinear,   //线性
    QHVCEffectVideoTransferCurveTypeCurve,    //曲线
};

@interface QHVCEffectVideoTransferParam : NSObject
@property (nonatomic, assign) QHVCEffectVideoTransferType transferType;    //参数类型
@property (nonatomic, assign) QHVCEffectVideoTransferCurveType curveType;  //曲线类型
@property (nonatomic, assign) CGFloat startValue;                          //初始值
@property (nonatomic, assign) CGFloat endValue;                            //结束值
@property (nonatomic, assign) NSInteger startTime;                         //初始时间，为相对时间(单位：毫秒)
@property (nonatomic, assign) NSInteger endTime;                           //结束时间，为相对时间(单位：毫秒)
@end

#pragma mark - 滤镜

@interface QHVCEffectFilter : QHVCEffectBase
@property (nonatomic, strong) CIImage* clutImage;   //查色图
@property (nonatomic, assign) CGFloat intensity;    //滤镜程度（0.0~1.0），默认为1.0

@end

#pragma mark - 贴纸

@interface QHVCEffectSticker : QHVCEffectBase
@property (nonatomic, strong) CIImage* sticker;      //贴纸图
@property (nonatomic, assign) CGFloat renderX;       //初始x坐标，相对输入背景图
@property (nonatomic, assign) CGFloat renderY;       //初始y坐标，相对输入背景图
@property (nonatomic, assign) CGFloat renderWidth;   //初始宽度，以像素为单位
@property (nonatomic, assign) CGFloat renderHeight;  //初始高度，以像素为单位
@property (nonatomic, assign) CGFloat renderRadian;  //初始旋转弧度值，例如，90° = π/2

/**
 视频过渡动画
 */
@property (nonatomic, strong) NSArray<QHVCEffectVideoTransferParam *>* videoTransfer;

@end

#pragma mark - 视频过渡效果

@interface QHVCEffectVideoTransfer : QHVCEffectBase
@property (nonatomic, strong) NSArray<QHVCEffectVideoTransferParam *>* videoTransfer; //视频关键帧动画

@end

#pragma mark - 图层混合

@interface QHVCEffectMix : QHVCEffectBase
@property (nonatomic, assign) CGFloat intensity;                //混合程度（0-1），默认为1，混合程度为0时不可
@property (nonatomic, strong) CIImage* topImage;                //需要和背景混合的图
@property (nonatomic, assign) CGSize outputSize;                //输出尺寸，若混合的图尺寸小于输出尺寸多余部分填充背景颜色；超过则按输出尺寸裁剪
@property (nonatomic, retain) NSString* backgroundColor;        //背景颜色，默认为黑色

@end

#pragma mark - 视频转场

@interface QHVCEffectVideoTransition : QHVCEffectBase
@property (nonatomic, assign) CGFloat progress;                 //转场进度
@property (nonatomic, strong) CIImage* secondImage;             //第二段视频帧
@property (nonatomic, strong) NSString* transitionName;         //转场名

@end

NS_ASSUME_NONNULL_END
