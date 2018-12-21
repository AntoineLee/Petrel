//
//  QHVCEditEffect.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
//#import "QHVCEditCommonDef.h"
#import <QHVCEditKit/QHVCEditKit.h>
#import <QHVCEffectKit/QHVCEffect.h>

@class QHVCEditTimeline;

#pragma mark - 特效基类

@interface QHVCEditEffect : QHVCEffectBase

/**
 初始化

 @param timeline timeline对象
 @return 特效实例对象
 */
- (instancetype)initEffectWithTimeline:(QHVCEditTimeline *)timeline;

/**
 获取父对象
 
 @return 父对象
 */
- (QHVCEditObject *)superObj;

/**
 添加自定义数据
 
 @param userData 自定义数据
 @return 返回值
 */
- (QHVCEditError)setUserData:(void *)userData;


/**
 获取自定义数据
 
 @return 自定义数据
 */
- (void *)userData;

@property (nonatomic, readonly,  assign) NSInteger effectId;            //特效Id，每个特效唯一
@property (nonatomic, readonly,  assign) QHVCEditEffectType effectType; //特效类型
@property (nonatomic, readwrite, assign) NSInteger startTime;           //特效相对被添加对象的开始时间（单位：毫秒）
@property (nonatomic, readwrite, assign) NSInteger endTime;             //特效相对被添加对象的结束时间（单位：毫秒）

@end

#pragma mark - 滤镜

@interface QHVCEditFilterEffect : QHVCEditEffect

@property (nonatomic, strong) NSString* filePath;   //查色图本地路径
@property (nonatomic, assign) CGFloat intensity;    //滤镜程度（0.0~1.0），默认为1.0

@end

#pragma mark - 贴图

@interface QHVCEditStickerEffect : QHVCEditEffect
@property (nonatomic, strong) UIImage* sticker;                  //贴图
@property (nonatomic, strong) NSString* stickerPath;             //贴图物理路径
@property (nonatomic, assign) CGFloat renderX;                   //初始x坐标，相对输出画布
@property (nonatomic, assign) CGFloat renderY;                   //初始y坐标，相对输出画布
@property (nonatomic, assign) NSInteger renderWidth;             //初始宽度，以像素为单位
@property (nonatomic, assign) NSInteger renderHeight;            //初始高度，以像素为单位
@property (nonatomic, assign) CGFloat renderRadian;              //初始旋转弧度值，例如，90° = π/2

/**
 过渡效果
 */
@property (nonatomic, retain) NSArray<QHVCEffectVideoTransferParam *>* videoTransfer;

@end

#pragma mark - 图层混合效果

/**
 仅用于轨道效果，图层按alpha混合，同一时间点只能存在一种混合效果
 */
@interface QHVCEditMixEffect : QHVCEditEffect
@property (nonatomic, assign) CGFloat intensity;              //混合程度（0-1），默认为1，混合程度为0时不可见

@end


#pragma mark - 视频过渡效果

@interface QHVCEditVideoTransferEffect : QHVCEditEffect
@property (nonatomic, retain) NSArray<QHVCEffectVideoTransferParam *>* videoTransfer; //视频过渡效果

@end

#pragma mark - 音频过渡效果

@interface QHVCEditAudioTransferEffect : QHVCEditEffect
@property (nonatomic, assign) NSInteger gainMin;                                            //过渡声音激励最小值，默认0
@property (nonatomic, assign) NSInteger gainMax;                                            //过渡声音激励最大值， 默认100
@property (nonatomic, assign) QHVCEditAudioTransferType transferType;                       //过渡类型
@property (nonatomic, assign) QHVCEditAudioTransferCurveType transferCurveType;             //音量变化曲线类型

@end
