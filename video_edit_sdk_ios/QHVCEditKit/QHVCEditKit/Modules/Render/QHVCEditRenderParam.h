//
//  QHVCEditRenderParam.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/9.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>
#import <UIKit/UIKit.h>
#import "QHVCEditCommonDef.h"

#pragma mark - render effect

@interface QHVCEditRenderEffect : NSObject
@property (nonatomic, assign) NSInteger effectId;
@property (nonatomic, assign) NSInteger startTime;
@property (nonatomic, assign) NSInteger endTime;
@property (nonatomic, retain) NSString* action;
@end

#pragma mark - render clip

@interface QHVCEditRenderClip : NSObject
@property (nonatomic, assign) NSInteger clipId;
@property (nonatomic, retain) NSMutableArray<QHVCEditRenderEffect *>* effects; //clip特效
@property (nonatomic, retain) CIImage* inputImage;
@property (nonatomic, assign) NSInteger sourceRotate;    //解码器解析出来的数据源旋转属性
@property (nonatomic, retain) NSString* bgColor;         //ARGB值，背景填充颜色，背景样式为毛玻璃效果时颜色值无效
@property (nonatomic, assign) QHVCEditBgMode bgMode;     //背景样式，默认纯色
@property (nonatomic, assign) QHVCEditFillMode fillMode; //填充样式
@property (nonatomic, assign) CGRect renderRect;         //渲染尺寸
@property (nonatomic, assign) CGRect sourceRect;         //视频源尺寸
@property (nonatomic, assign) CGFloat frameRadian;       //视频内容旋转弧度值，例如，90° = π/2
@property (nonatomic, assign) BOOL flipX;                //水平翻转
@property (nonatomic, assign) BOOL flipY;                //垂直翻转
@property (nonatomic, assign) CGSize outputSize;         //输出尺寸
@property (nonatomic, assign) CGFloat previewRadian;     //输出渲染旋转弧度值
@end

#pragma mark - render transition

@interface QHVCEditRenderTransitionClip : QHVCEditRenderClip
@property (nonatomic, assign) NSInteger transitionId;
@property (nonatomic, retain) NSString* transitionName;
@property (nonatomic, assign) QHVCEditEasingFunctionType easingFunctionTyp;
@property (nonatomic, assign) NSInteger transitionStartTime;
@property (nonatomic, assign) NSInteger transitionEndTime;

@end

#pragma mark - render track

@interface QHVCEditRenderTrack : NSObject
@property (nonatomic, assign) NSInteger trackId;
@property (nonatomic, retain) QHVCEditRenderClip* mainClip;
@property (nonatomic, retain) QHVCEditRenderEffect* mixEffect;
@property (nonatomic, retain) QHVCEditRenderTransitionClip* transitionClip;
@property (nonatomic, retain) NSMutableArray<QHVCEditRenderEffect *>* effects;
@end

#pragma mark - render param

@interface QHVCEditRenderParam : NSObject
@property (nonatomic, retain) NSMutableArray<QHVCEditRenderTrack *>* tracks;
@property (nonatomic, retain) NSMutableArray<QHVCEditRenderEffect *>* effects;
@property (nonatomic, assign) NSInteger timestampMs;
@property (nonatomic, retain) id userData;
@end
