//
//  CIDRotateFilter.h
//  CoreImageDemo
//
//  Created by liyue-g on 2017/12/5.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QHVCEffectKit/QHVCEffectBase+Process.h>
#import "QHVCEditCommonDef.h"

typedef NS_ENUM(NSUInteger, QHVCEditCIRotateFilterFillMode)
{
    QHVCEditCIRotateFilterFillMode_AspectFit,    //视频内容完全填充，可能会有黑边
    QHVCEditCIRotateFilterFillMode_AspectFill,   //视频内容铺满画布，视频内容可能会被裁剪
    QHVCEditCIRotateFilterFillMode_ScaleToFill,  //视频内容铺满画布，视频内容可能会被拉伸
};

typedef NS_ENUM(NSUInteger, QHVCEditCIRotateFilterBGMode)
{
    QHVCEditCIRotateFilterBGMode_Color,    //填充背景样式，纯色
    QHVCEditCIRotateFilterBGMode_Blur,     //填充背景样式，毛玻璃
};

@interface QHVCEditCIRotateFilter : QHVCEffectBase

@property (nonatomic, assign) QHVCEditCIRotateFilterBGMode bgMode; //背景样式，默认纯色
@property (nonatomic, retain) NSString* bgColor; //背景填充颜色，16进制ARGB值,背景样式为毛玻璃效果时颜色值无效
@property (nonatomic, assign) QHVCEditCIRotateFilterFillMode fillMode; //填充方式
@property (nonatomic, assign) CGRect renderRect;
@property (nonatomic, assign) CGRect sourceRect;
@property (nonatomic, assign) CGFloat sourceRadian; //视频源旋转属性，弧度值
@property (nonatomic, assign) CGFloat contentRadian; //业务调用，视频内容旋转弧度值，例如，90° = π/2
@property (nonatomic, assign) BOOL flipX; //水平翻转
@property (nonatomic, assign) BOOL filpY; //垂直翻转
@property (nonatomic, assign) CGSize outputSize;
@property (nonatomic, assign) CGFloat previewRadian; //渲染旋转弧度值，仅做裁剪计算用，此处不真正旋转

@end
