//
//  QHVCEditConfig.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/23.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditConfig.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditEffect.h"
#import "QHVCEditCommonDef.h"
#import "ve_interface.h"

@interface QHVCEditConfig ()
@property (nonatomic, strong) NSString* cacheDirectory;
@property (atomic,    assign) NSInteger timelineIndex;
@property (nonatomic, strong) EAGLContext* imageProcessingContext;

@end

@implementation QHVCEditConfig

+ (instancetype)sharedInstance
{
    static QHVCEditConfig* s_instance = nil;
    static dispatch_once_t predic;
    dispatch_once(&predic, ^{
        s_instance = [[QHVCEditConfig alloc] init];
    });
    return s_instance;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        [self initParams];
    }
    return self;
}

- (void)initParams
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachesDir = [paths objectAtIndex:0];
    self.cacheDirectory = [cachesDir stringByAppendingString:@"/com.qihoo.videocloud/QHVCEdit/"];
    [QHVCEditUtils createDirectoryAtPath:self.cacheDirectory];
    
    //eaglContext
    if (!self.imageProcessingContext)
    {
        self.imageProcessingContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }
    
}

- (NSString *)getBundlePath
{
    NSBundle* bundle = [NSBundle bundleWithPath:[[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Frameworks"]
                                                 stringByAppendingPathComponent:QHVCEDIT_FRAMEWORK]];
    NSString* path = [[bundle resourcePath] stringByAppendingString:@"/"];
    return path;
}

- (NSString *)cacheDirectory
{
    return _cacheDirectory;
}

- (NSInteger)getTimelineIndex
{
    NSInteger index = self.timelineIndex;
    self.timelineIndex ++;
    return index;
}

- (NSString *)getErrorCodeInfo:(int)errorCode
{
    NSString* errorCodeStr = [NSString stringWithFormat:@"%d", errorCode];
    NSString* info = @"";
    
    VE_ERR err = (VE_ERR)errorCode;
    switch (err)
    {
        case VE_ERR_MALLOC_FAILED:
        {
            //-999
            info = @"VE_ERR_MALLOC_FAILED";
            break;
        }
        case VE_ERR_OPEN_FILE_FAILED:
        {
            info = @"VE_ERR_OPEN_FILE_FAILED";
            break;
        }
        case VE_ERR_FILE_CORRUPT:
        {
            info = @"VE_ERR_FILE_CORRUPT";
            break;
        }
        case VE_ERR_NO_BACKGOUND_MUSIC:
        {
            info = @"VE_ERR_NO_BACKGOUND_MUSIC";
            break;
        }
        case VE_ERR_V_FILTER_EXIST:
        {
            //-995
            info = @"VE_ERR_V_FILTER_EXIST";
            break;
        }
        case VE_ERR_V_FILTER_NOT_EXIST:
        {
            info = @"VE_ERR_V_FILTER_NOT_EXIST";
            break;
        }
        case VE_ERR_CREATE_THREAD_FAILED:
        {
            info = @"VE_ERR_CREATE_THREAD_FAILED";
            break;
        }
        case VE_ERR_INPUT_PARAM:
        {
            //-899
            info = @"VE_ERR_INPUT_PARAM";
            break;
        }
        case VE_ERR_PARAM_CONFIG_LOCKED:
        {
            //-799
            info = @"VE_ERR_PARAM_CONFIG_LOCKED";
            break;
        }
        case VE_ERR_EMPTY_PARAM_CONFIG:
        {
            info = @"VE_ERR_EMPTY_PARAM_CONFIG";
            break;
        }
        case VE_ERR_CANNOT_FIND_VIDEO_STREAM:
        {
            info = @"VE_ERR_CANNOT_FIND_VIDEO_STREAM";
            break;
        }
        case VE_ERR_CANNOT_FIND_AUDIO_STREAM:
        {
            info = @"VE_ERR_CANNOT_FIND_AUDIO_STREAM";
            break;
        }
        case VE_ERR_NEW_STREAM_FAILED:
        {
            //-795
            info = @"VE_ERR_NEW_STREAM_FAILED";
            break;
        }
        case VE_ERR_FIND_DECODER_FAILED:
        {
            info = @"VE_ERR_FIND_DECODER_FAILED";
            break;
        }
        case VE_ERR_OPEN_DECODER_FAILED:
        {
            info = @"VE_ERR_OPEN_DECODER_FAILED";
            break;
        }
        case VE_ERR_OPEN_ENCODER_FAILED:
        {
            info = @"VE_ERR_OPEN_ENCODER_FAILED";
            break;
        }
        case VE_ERR_ENCODE_FAILED:
        {
            info = @"VE_ERR_ENCODE_FAILED";
            break;
        }
        case VE_ERR_DECODE_FAILED:
        {
            //-790
            info = @"VE_ERR_DECODE_FAILED";
            break;
        }
        case VE_ERR_CREATE_FILTER_FAILED:
        {
            info = @"VE_ERR_CREATE_FILTER_FAILED";
            break;
        }
        case VE_ERR_AV_SEEK_FRAME_FAILED:
        {
            info = @"VE_ERR_AV_SEEK_FRAME_FAILED";
            break;
        }
        case VE_ERR_WRITE_FILE_FAILED:
        {
            info = @"VE_ERR_WRITE_FILE_FAILED";
            break;
        }
        case VE_ERR_FILL_AUDIO_FAILED:
        {
            info = @"VE_ERR_FILL_AUDIO_FAILED";
            break;
        }
        case VE_ERR_PARAM_ERR:
        {
            //-785
            info = @"VE_ERR_PARAM_ERR";
            break;
        }
        default:
            break;
    }
    
    char infoChar[1024];
    ve_get_err_info(infoChar, 1024);
    NSString* detail = [NSString stringWithCString:infoChar encoding:NSUTF8StringEncoding];
    NSString* outStr = [NSString stringWithFormat:@"error code[%@] info[%@]", errorCodeStr, info];
    outStr = [NSString stringWithFormat:@"detail: %@", detail];
    
    return outStr;
}

- (NSString *)getEffectBasicInfo:(QHVCEditEffect *)effect
{
    NSNumber* effectId  = [NSNumber numberWithInteger:[effect effectId]];
    NSNumber* effectType = [NSNumber numberWithInteger:[effect effectType]];
    NSNumber* startTime = [NSNumber numberWithInteger:[effect startTime]];
    NSNumber* endTime   = [NSNumber numberWithInteger:[effect endTime]];
    
    NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity:0];
    [dict setValue:effectId forKey:QHVCEDIT_DEFINE_EFFECT_ID];
    [dict setValue:effectType forKey:QHVCEDIT_DEFINE_EFFECT_TYPE];
    [dict setValue:startTime forKey:QHVCEDIT_DEFINE_START_TIME];
    [dict setValue:endTime forKey:QHVCEDIT_DEFINE_END_TIME];
    
    NSData* data = [QHVCEditUtils createJsonDataWithDictionary:dict];
    NSString* json = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    return json;
}

- (NSString *)printEffectType:(QHVCEditEffectType)type
{
    NSString* effectType = @"UnKnown";
    switch (type)
    {
        case QHVCEditEffectTypeVideo:
        {
            effectType = @"video";
            break;
        }
        case QHVCEditEffectTypeAudio:
        {
            effectType = @"audio";
            break;
        }
        case QHVCEditEffectTypeMix:
        {
            effectType = @"mix";
            break;
        }
        default:
            break;
    }
    return effectType;
}

- (NSString *)printEffectDetail:(QHVCEditEffect *)effect
{
    QHVCEditEffectType effectType = [effect effectType];
    NSString* effectName = [self printEffectType:effectType];
    NSInteger effectId  = [effect effectId];
    NSInteger startTime = [effect startTime];
    NSInteger endTime   = [effect endTime];
    
    NSString* basicInfo = [NSString stringWithFormat:@"effectType[%@] effectId[%ld] startTime[%ld] endTime[%ld]",
                          effectName,
                          effectId,
                          startTime,
                          endTime];
    return basicInfo;
}

- (int)getVEFilterType:(QHVCEditEffect *)effect
{
    VE_FILTER veFilter = VE_FILTER_VIDEO;
    switch (effect.effectType) {
        case QHVCEditEffectTypeAudio:
            veFilter = VE_FILTER_AUDIO;
            break;
            
        default:
            break;
    }
    
    return (int)veFilter;
}

- (EAGLContext *)sharedImageProcessingContext
{
    return self.imageProcessingContext;
}

@end
