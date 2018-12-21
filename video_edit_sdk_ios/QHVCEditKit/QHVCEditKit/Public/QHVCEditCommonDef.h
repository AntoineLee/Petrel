//
//  QHVCEditCommonDef.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/20.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

typedef NS_ENUM(NSUInteger, QHVCEditError)
{
    QHVCEditErrorNoError,                         //无错误
    QHVCEditErrorParamError,                      //参数错误
    QHVCEditErrorAlreayExist,                     //已存在
    QHVCEditErrorNotExist,                        //不存在
    
    QHVCEditErrorRequestThumbnailError = 100,     //获取缩略图错误
    
    QHVCEditErrorInitPlayerError = 200,           //初始化播放器错误
    QHVCEditErrorPlayerStatusError,               //播放器状态错误
    
    QHVCEditErrorInitProducerError = 300,         //初始化合成器错误
    QHVCEditErrorProducerHandleIsNull,            //合成器句柄为空
    QHVCEditErrorProducingError,                  //合成中出错
};

typedef NS_ENUM(NSUInteger, QHVCEditObjectType)
{
    QHVCEditObjectTypeTimeline,    //timeline对象
    QHVCEditObjectTypeTrack,       //track对象
    QHVCEditObjectTypeClip,        //clip对象
};

typedef NS_ENUM(NSUInteger, QHVCEditFillMode)
{
    QHVCEditFillModeAspectFit,    //视频内容完全填充，可能会有黑边
    QHVCEditFillModeAspectFill,   //视频内容铺满画布，视频内容可能会被裁剪
    QHVCEditFillModeScaleToFill,  //视频内容铺满画布，视频内容可能会被拉伸
};

typedef NS_ENUM(NSUInteger, QHVCEditBgMode)
{
    QHVCEditBgModeColor,  //纯色背景
    QHVCEditBgModeBlur,   //毛玻璃背景
};

typedef NS_ENUM(NSUInteger, QHVCEditTrackType)
{
    QHVCEditTrackTypeVideo,    //视频轨道
    QHVCEditTrackTypeAudio,    //音频轨道
};

typedef NS_ENUM(NSUInteger, QHVCEditTrackArrangement)
{
    QHVCEditTrackArrangementOverlay,
    QHVCEditTrackArrangementSequence,
};

typedef NS_ENUM(NSUInteger, QHVCEditTrackClipType)
{
    QHVCEditTrackClipTypeVideo,    //视频文件
    QHVCEditTrackClipTypeAudio,    //音频文件
    QHVCEditTrackClipTypeImage,    //静态图片
};

typedef NS_ENUM(NSUInteger, QHVCEditEffectType)
{
    QHVCEditEffectTypeVideo,   //视频特效
    QHVCEditEffectTypeAudio,   //音频特效
    QHVCEditEffectTypeMix,     //图层混合特效
};

typedef NS_ENUM(NSUInteger, QHVCEditEasingFunctionType)
{
    QHVCEditEasingFunctionTypeLinear,
    QHVCEditEasingFunctionTypeCubicEaseIn,
    QHVCEditEasingFunctionTypeCubicEaseOut,
    QHVCEditEasingFunctionTypeCubicEaseInOut,
    QHVCEditEasingFunctionTypeQuintEaseInOut,
    QHVCEditEasingFunctionTypeQuartEaseInOut,
    QHVCEditEasingFunctionTypeQuadEaseInOut,
    QHVCEditEasingFunctionTypeQuadEaseOut,
};

#pragma mark - 声音淡入淡出

typedef NS_ENUM(NSUInteger, QHVCEditAudioTransferType)
{
    QHVCEditAudioTransferTypeNone,                //正常
    QHVCEditAudioTransferTypeFadeIn,              //淡入
    QHVCEditAudioTransferTypeFadeOut,             //淡出
};

typedef NS_ENUM(NSInteger, QHVCEditAudioTransferCurveType)
{
    QHVCEditAudioTransferCurveTypeTri,                    //线性
    QHVCEditAudioTransferCurveTypeQsin,                   //正弦波
    QHVCEditAudioTransferCurveTypeEsin,                   //指数正弦
    QHVCEditAudioTransferCurveTypeHsin,                   //正弦波的一半
    QHVCEditAudioTransferCurveTypeLog,                    //对数
    QHVCEditAudioTransferCurveTypeIpar,                   //倒抛物线
    QHVCEditAudioTransferCurveTypeQua,                    //二次方
    QHVCEditAudioTransferCurveTypeCub,                    //立方
    QHVCEditAudioTransferCurveTypeSqu,                    //平方根
    QHVCEditAudioTransferCurveTypeCbr,                    //立方根
    QHVCEditAudioTransferCurveTypePar,                    //抛物线
    QHVCEditAudioTransferCurveTypeExp,                    //指数
    QHVCEditAudioTransferCurveTypeIqsin,                  //正弦波反季
    QHVCEditAudioTransferCurveTypeIhsin,                  //倒一半的正弦波
    QHVCEditAudioTransferCurveTypeDese,                   //双指数差值
    QHVCEditAudioTransferCurveTypeDesi,                   //双指数S弯曲
};

#pragma mark - 慢视频信息

@interface QHVCEditSlowMotionVideoInfo : NSObject
@property (nonatomic, assign) NSInteger startTime;    //物理文件开始时间点（单位：毫秒）
@property (nonatomic, assign) NSInteger endTime;      //物理文件结束时间点（单位：毫秒）
@property (nonatomic, assign) CGFloat speed;            //物理文件原始速率

@end

#pragma mark - 背景样式

@interface QHVCEditBgParams : NSObject

/**
 视频背景画布样式，默认黑色背景
 */
@property (nonatomic, assign) QHVCEditBgMode mode;

/**
 背景信息
 背景样式为 QHVCEditOutputBackgroudMode_Color时，背景信息为16进制ARGB值
 */
@property (nonatomic, strong) NSString* bgInfo;

@end

#pragma mark - QHVCEditObject

@interface QHVCEditObject : NSObject
@property (nonatomic, readonly, assign) QHVCEditObjectType objType;

@end

#pragma mark - Common

typedef NS_ENUM(NSUInteger, QHVCEditLogLevel)
{
    QHVCEditLogLevelNone,   //关闭日志
    QHVCEditLogLevelError,  //仅包含错误
    QHVCEditLogLevelWarn,   //错误+警告
    QHVCEditLogLevelInfo,   //错误+警告+状态信息
    QHVCEditLogLevelDebug,  //错误+警告+状态信息+调试信息
};

typedef NS_ENUM(NSUInteger, QHVCEditVideoCodec)
{
    QHVCEditVideoCodecH264,
    QHVCEditVideoCodecHEVC,
};

typedef NS_ENUM(NSUInteger, QHVCEditAudioCodec)
{
    QHVCEditAudioCodecAAC,
};

@interface QHVCEditFileInfo : NSObject
@property (nonatomic, assign) BOOL isPicture;                //是否是图片
@property (nonatomic, assign) NSInteger width;               //宽度
@property (nonatomic, assign) NSInteger height;              //高度
@property (nonatomic, assign) NSInteger durationMs;          //文件总时长(单位：毫秒)
@property (nonatomic, assign) NSInteger videoBitrate;        //视频码率
@property (nonatomic, assign) NSInteger fps;                 //视频帧率
@property (nonatomic, assign) NSInteger audioBitrate;        //音频码率
@property (nonatomic, assign) NSInteger audioChannels;       //音频声道数
@property (nonatomic, assign) NSInteger audioSamplerate;     //音频采样率

@end

@interface QHVCEditTools : NSObject

/**
 获取文件信息

 @param filePath 文件物理路径
 @return 文件信息
 */
+ (QHVCEditFileInfo *)getFileInfo:(NSString *)filePath;

/**
 获取版本号
 */
+ (NSString *)getVersion;

/**
 设置SDK日志控制台输出级别
 
 @param level SDK日志控制台输出级别
 */
+ (void)setSDKLogLevel:(QHVCEditLogLevel)level;

/**
 设置SDK日志写文件级别
 
 @param level SDK日志写文件级别
 */
+ (void)setSDKLogLevelForFile:(QHVCEditLogLevel)level;

/**
 设置用户自定义日志控制台输出级别
 
 @param level 用户自定义控制台输出级别
 */
+ (void)setUserLogLevel:(QHVCEditLogLevel)level;

/**
 设置用户自定义日志写文件级别
 
 @param level 用户自定义日志写文件级别
 */
+ (void)setUserLogLevelForFile:(QHVCEditLogLevel)level;

/**
 输出用户自定义日志，若开启了写文件，则日志同时写入文件
 
 @param level 日志级别
 @param prefix 用户自定义日志前缀
 @param content 日志内容
 */
+ (void)printUserLog:(QHVCEditLogLevel)level prefix:(NSString*)prefix content:(NSString *)content;

/**
 设置日志存放路径，默认存于Library/Caches/com.qihoo.videocloud/QHVCEdit/Log/”
 
 @param path 日志存放路径
 */
+ (void)setLogFilePath:(NSString *)path;

/**
 日志是否写文件
 
 @param writeToLocal 是否写文件
 */
+ (void)writeLogToLocal:(BOOL)writeToLocal;

/**
 设置日志文件相关参数
 
 @param singleSize 单个日志文件最大大小（单位MB），默认1M
 @param count 日志文件循环写入文件，文件个数，默认3个
 */
+ (void)setLogFileParams:(NSInteger)singleSize count:(NSInteger)count;

@end
