//
//  QHVCEffectUtils.h
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/10/30.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
#import "QHVCEffect.h"

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
#define KERNEL_STRING(text) @ STRINGIZE2(text)

@interface QHVCEffectUtils : NSObject

#pragma mark - String

/**
 * 当前系统版本号比较
 */
#define QHVCEFFECT_SYSTEM_VERSION_GREATER_THAN(v) ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

/**
 * 判断字符串是否为空
 */
+ (BOOL)stringIsNull:(NSString *)parameter;

#pragma mark - transfer

/**
 计算透明度偏移量

 @param transferParam 动画数组
 @param startTime 开始时间
 @param endTime 结束时间
 @param currentTime 当前时间
 @return 透明偏移量
 */
+ (CGFloat)computeAlpha:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
              startTime:(NSInteger)startTime
                endTime:(NSInteger)endTime
            currentTime:(NSInteger)currentTime;

/**
 计算x偏移量

 @param transferParam 动画数组
 @param startTime 开始时间
 @param endTime 结束时间
 @param currentTime 当前时间
 @return x偏移量
 */
+ (CGFloat)computeOffsetX:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                startTime:(NSInteger)startTime
                  endTime:(NSInteger)endTime
              currentTime:(NSInteger)currentTime;

/**
 计算y偏移量

 @param transferParam 动画数组
 @param startTime 开始时间
 @param endTime 结束时间
 @param currentTime 当前时间
 @return y偏移量
 */
+ (CGFloat)computeOffsetY:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                startTime:(NSInteger)startTime
                  endTime:(NSInteger)endTime
              currentTime:(NSInteger)currentTime;

/**
 计算缩放值

 @param transferParam 动画数组
 @param startTime 开始时间
 @param endTime 结束时间
 @param currentTime 当前时间
 @return 缩放值
 */
+ (CGFloat)computeOffsetScale:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                    startTime:(NSInteger)startTime
                      endTime:(NSInteger)endTime
                  currentTime:(NSInteger)currentTime;

/**
 计算弧度动画值

 @param transferParam 动画数组
 @param startTime 开始时间
 @param endTime 结束时间
 @param currentTime 当前时间
 @return 弧度值
 */
+ (CGFloat)computeOffsetRadian:(NSArray<QHVCEffectVideoTransferParam *>*)transferParam
                     startTime:(NSInteger)startTime
                       endTime:(NSInteger)endTime
                   currentTime:(NSInteger)currentTime;

#pragma mark - Color

/**
 16进制ARGB颜色转UIColor
 
 @param hexColor 16进制ARGB颜色
 @return UIColor对象
 */
+ (UIColor *)colorForHex:(NSString *)hexColor;


/**
 生成纯色图

 @param color 颜色
 @param frame 输出尺寸
 @return 纯色图
 */
+ (CIImage *)createImageWithColor:(UIColor *)color frame:(CGRect)frame;

#pragma mark - Transform


/**
 图片坐标归零

 @param inputImage 输入图
 @return 归零图
 */
+ (CIImage *)imageTranslateToZeroPoint:(CIImage *)inputImage;

@end
