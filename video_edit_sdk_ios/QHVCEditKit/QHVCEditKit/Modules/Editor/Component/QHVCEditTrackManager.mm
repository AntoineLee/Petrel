//
//  QHVCEditTrackManager.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/2.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditTrackManager.h"
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditConfig.h"

#import "QHVCEditTrack.h"
#import "QHVCEditTrackClipManager.h"
#import "QHVCEditTrackClip.h"
#import "QHVCEditEffect.h"
#import "ve_interface.h"
#import "QHVCEditConfig.h"

@interface QHVCEditTrackManager ()
{
    HANDLE _timelineHandle;
}

@property (nonatomic, retain) QHVCEditTrack* track;
@property (atomic,    strong) NSMutableArray<QHVCEditTrackClipManager *>* clips;
@property (atomic,    strong) NSMutableDictionary* effects;
@end

@implementation QHVCEditTrackManager

#pragma mark - 基础方法

- (instancetype)initWithTrack:(QHVCEditTrack *)track timelineHandle:(void *)timelineHandle
{
    self = [super init];
    if (self)
    {
        if (timelineHandle)
        {
            _timelineHandle = timelineHandle;
            self.track = track;
            [self initParams];
        }
    }
    
    return self;
}

- (void)initParams
{
    self.clips = [[NSMutableArray alloc] initWithCapacity:0];
    self.effects = [[NSMutableDictionary alloc] initWithCapacity:0];
}

- (QHVCEditTrack *)getTrack
{
    return self.track;
}

- (NSInteger)duration
{
    NSInteger duration = [self ve_getDuration];
    return duration;
}

- (QHVCEditError)setSpeed:(CGFloat)speed
{
    QHVCEditError err = [self ve_setSpeed:speed];
    return err;
}

- (QHVCEditError)setVolume:(NSInteger)volume
{
    QHVCEditError err = [self ve_setVolume:volume];
    return err;
}

#pragma mark - Clip 方法

- (QHVCEditError)appendClip:(QHVCEditTrackClipManager *)clipMgr
{
    QHVCEditTrackClip* clip = [clipMgr getClip];
    QHVCEditError err = [self ve_insertClip:clip atIndex:[self.clips count]];
    if (err == QHVCEditErrorNoError)
    {
        [self.clips addObject:clipMgr];
    }
    
    return err;
}

- (QHVCEditError)insertClip:(QHVCEditTrackClipManager *)clipMgr atIndex:(NSInteger)index
{
    QHVCEditTrackClip* clip = [clipMgr getClip];
    QHVCEditError err = [self ve_insertClip:clip atIndex:index];
    if (err == QHVCEditErrorNoError)
    {
        NSInteger count = [self.clips count];
        index = MIN(index, count);
        [self.clips insertObject:clipMgr atIndex:index];
    }

    return err;
}

- (QHVCEditError)addClip:(QHVCEditTrackClipManager *)clipMgr atTime:(NSInteger)timeMs
{
    QHVCEditTrackClip* clip = [clipMgr getClip];
    QHVCEditError err = [self ve_insertClip:clip atTime:timeMs];
    if (err == QHVCEditErrorNoError)
    {
        //按时间点顺序插入
        [self.clips addObject:clipMgr];
        [self sortClips];
        [clipMgr updateInsertTime:timeMs];
    }

    return err;
}

- (QHVCEditError)moveClip:(NSInteger)fromIndex toIndex:(NSInteger)index
{
    if (index != fromIndex && fromIndex < [self.clips count] && index < [self.clips count] && index >= 0 && fromIndex >= 0) {
        
        id obj = [self.clips objectAtIndex:fromIndex];
        [self.clips removeObjectAtIndex:fromIndex];
        [self.clips insertObject:obj atIndex:index];
    }
    
    QHVCEditError err = [self ve_moveClip:fromIndex toIndex:index];
    return err;
}

- (QHVCEditError)changeClipInsertTime:(NSInteger)timeMs clipId:(NSInteger)clipId
{
    QHVCEditTrackClipManager* clipMgr = [self getClipById:clipId];
    QHVCEditError err = [self ve_changeClipInsertTime:clipId insertTime:timeMs];
    if (err == QHVCEditErrorNoError)
    {
        [clipMgr updateInsertTime:timeMs];
        [self sortClips];
    }
    
    return err;
}

- (QHVCEditError)updateClipParams:(QHVCEditTrackClipManager *)clipMgr
{
    QHVCEditError err = [self ve_updateClipParams:[clipMgr getClip]];
    return err;
}

- (QHVCEditError)deleteClipById:(NSInteger)clipId
{
    QHVCEditTrackClipManager* clipMgr = [self getClipById:clipId];
    QHVCEditError err = [self ve_deleteClipById:clipId];
    if (err == QHVCEditErrorNoError)
    {
        [self.clips removeObject:clipMgr];
    }
    
    return err;
}

- (QHVCEditError)deleteClipAtIndex:(NSInteger)index
{
    QHVCEditError err = [self ve_deleteClipAtIndex:index];
    if (err == QHVCEditErrorNoError)
    {
        [self.clips removeObjectAtIndex:index];
    }
    
    return err;
}

- (QHVCEditTrackClipManager *)getClipById:(NSInteger)clipId
{
    __block QHVCEditTrackClipManager* mgr = nil;
    [self.clips enumerateObjectsUsingBlock:^(QHVCEditTrackClipManager * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
    {
        QHVCEditTrackClip* clip = [obj getClip];
        if (clip)
        {
            NSInteger objId = [clip clipId];
            if (objId == clipId)
            {
                mgr = obj;
                *stop = YES;
            }
        }
    }];
    
    return mgr;
}

- (QHVCEditTrackClipManager *)getClipAtIndex:(NSInteger)index
{
    NSInteger count = [self.clips count];
    if (index >= count)
    {
        LogError(@"track getClipAtIndex error, index %ld beyond bounds [0 ... %ld]", (long)index, (long)(count -1));
        return nil;
    }
    
    QHVCEditTrackClipManager* mgr = [self.clips objectAtIndex:index];
    return mgr;
}

- (NSArray<QHVCEditTrackClipManager *>*)getClips
{
    return self.clips;
}

- (void)sortClips
{
    [self.clips sortUsingComparator:^NSComparisonResult(id  _Nonnull obj1, id  _Nonnull obj2) {
        QHVCEditTrackClipManager *clipMgr1 = (QHVCEditTrackClipManager *)obj1;
        QHVCEditTrackClipManager *clipMgr2 = (QHVCEditTrackClipManager *)obj2;
        if (clipMgr1.insertTime > clipMgr2.insertTime) {
            return NSOrderedDescending;
        }else{
            return NSOrderedAscending;
        }
    }];
}

#pragma mark - 转场

- (QHVCEditError)addVideoTransitionToIndex:(NSInteger)clipIndex
                                  duration:(NSInteger)durationMs
                       videoTransitionName:(NSString *)transitionName
                              transitionId:(NSInteger)transitionId
                        easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    //AB间添加转场， index应为B的index，index>0
    if (clipIndex <= 0)
    {
        LogError(@"track addVideoTransition error, index <= 0");
        return QHVCEditErrorParamError;
    }
    
    if (durationMs <= 0)
    {
        LogError(@"track addVideoTransition error, duration <= 0");
        return QHVCEditErrorParamError;
    }
    
    if ([QHVCEditUtils stringIsNull:transitionName])
    {
        LogError(@"track addVideoTransition error, transitionName is nil");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self ve_addTransitionToIndex:clipIndex
                                             duration:durationMs
                                               action:[transitionName UTF8String]
                                         transitionId:transitionId];
    if (err == QHVCEditErrorNoError)
    {
        QHVCEditTrackClipManager* clipMgr = [self getClipAtIndex:clipIndex];
        [clipMgr addVideoTransition:transitionId duration:durationMs transitionName:transitionName easingFunctionType:easingFunctionType];
    }
    return err;
}

- (QHVCEditError)updateVideoTransitionAtIndex:(NSInteger)clipIndex
                                     duration:(NSInteger)durationMs
                          videoTransitionName:(NSString *)transitionName
                           easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    //检测是否存在转场
    QHVCEditTrackClipManager* clipMgr = [self getClipAtIndex:clipIndex];
    BOOL haveTransition = [clipMgr haveTransition];
    if (!haveTransition)
    {
        LogError(@"track updateVideoTransition error, transition not exist");
        return QHVCEditErrorNotExist;
    }
    
    if ([QHVCEditUtils stringIsNull:transitionName])
    {
        LogError(@"track updateVideoTransition error, transitionName is nil");
        return QHVCEditErrorParamError;
    }
    
    QHVCEditError err = [self ve_editTransitionAtIndex:clipIndex
                                              duration:durationMs
                                                action:[transitionName UTF8String]
                                          transitionId:[clipMgr transitionId]];
    if (err == QHVCEditErrorNoError)
    {
        [clipMgr updateVideoTransition:durationMs transitionName:transitionName easingFunctionType:easingFunctionType];
    }
    
    return err;
}

- (QHVCEditError)deleteVideoTransition:(NSInteger)index
{
    //检测是否存在转场
    QHVCEditTrackClipManager* clipMgr = [self getClipAtIndex:index];
    BOOL haveTransition = [clipMgr haveTransition];
    if (!haveTransition)
    {
        LogError(@"track deleteVideoTransition error, transition not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [self ve_deleteTransition:[clipMgr transitionId]];
    if (err == QHVCEditErrorNoError)
    {
        [clipMgr deleteVideoTransition];
    }
    
    return err;
}

#pragma mark - 特效

- (QHVCEditError)addEffect:(QHVCEditEffect *)effect
{
    //检测特效是否已存在
    NSNumber* effectId = [NSNumber numberWithInteger:[effect effectId]];
    QHVCEditEffect* obj = [self.effects objectForKey:effectId];
    if (obj)
    {
        LogError(@"track addEffect error, effect alreay exist");
        return QHVCEditErrorAlreayExist;
    }
    
    QHVCEditError err = [self ve_addEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects setObject:effect forKey:effectId];
    }
    
    return err;
}

- (QHVCEditError)updateEffect:(QHVCEditEffect *)effect
{
    //检测特效是否存在
    NSNumber* effectId = [NSNumber numberWithInteger:[effect effectId]];
    QHVCEditEffect* obj = [self.effects objectForKey:effectId];
    if (!obj)
    {
        LogError(@"track updateEffect error, effect not exist");
        return QHVCEditErrorNotExist;
    }
    
    QHVCEditError err = [self ve_updateEffect:effect];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects setObject:effect forKey:effectId];
    }
    
    return err;
}

- (QHVCEditError)deleteEffectById:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    QHVCEditError err = [self ve_deleteEffect:effectId];
    if (err == QHVCEditErrorNoError)
    {
        [self.effects removeObjectForKey:effectIdNum];
    }
    
    return err;
}

- (QHVCEditEffect *)getEffectById:(NSInteger)effectId
{
    NSNumber* effectIdNum = [NSNumber numberWithInteger:effectId];
    QHVCEditEffect* effect = [self.effects objectForKey:effectIdNum];
    return effect;
}

- (NSArray<QHVCEditEffect *>*)getEffects:(QHVCEditTrack *)track
{
    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:0];
    [self.effects enumerateKeysAndObjectsUsingBlock:^(NSNumber* key, QHVCEditEffect* obj, BOOL * _Nonnull stop)
    {
        [array addObject:obj];
    }];
    
    return array;
}

#pragma mark - video_edit Methods

- (NSInteger)ve_getDuration
{
    int duration = 0;
    //实际该轨道播放的时长，会受变速转场影响
    int trackId = (int)self.track.trackId;
    VE_ERR err = ve_track_get_overall_duration(_timelineHandle, trackId, 1, &duration);
    
    if (err != VE_ERR_OK)
    {
        NSString* info = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err];
        LogError(@"track get duration error, trackid[%d], errorInfo[%@]", trackId, info);
        return duration;
    }
        
    return duration;
}

- (QHVCEditError)ve_setSpeed:(CGFloat)speed
{
    ve_track veTrack;
    int trackId = (int)self.track.trackId;
    ve_track_get(_timelineHandle, trackId, &veTrack);
    veTrack.speed = speed;
    [self ve_editTrackParams:&veTrack];
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_setVolume:(NSInteger)volume
{
    ve_track veTrack;
    int trackId = (int)self.track.trackId;
    ve_track_get(_timelineHandle, trackId, &veTrack);
    veTrack.volume = (int)volume;
    [self ve_editTrackParams:&veTrack];
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_editTrackParams:(ve_track *)track
{
    VE_ERR ve_err = ve_track_mod(_timelineHandle, track);
    if (ve_err != VE_ERR_OK)
    {
        NSString* info = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:ve_err];
        LogError(@"track edit param error, %@", info);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_insertClip:(QHVCEditTrackClip *)clip atIndex:(NSInteger)index
{
    ve_clip param = [self ve_setClipParams:clip];
    VE_ERR err = ve_clip_insert(_timelineHandle, &param, (int)index);
    if (err != VE_ERR_OK)
    {
        LogError(@"track insert clip at index %ld error, %@", (long)index, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_insertClip:(QHVCEditTrackClip *)clip atTime:(NSInteger)timeMs
{
    ve_clip param = [self ve_setClipParams:clip];
    param.insert_time = (int)timeMs;
    VE_ERR err = ve_clip_insert(_timelineHandle, &param);
    if (err != VE_ERR_OK)
    {
        LogError(@"track insert clip at time %ld error, %@", (long)timeMs, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_moveClip:(NSInteger)fromIndex toIndex:(NSInteger)index
{
    VE_ERR ve_err = ve_clip_move(_timelineHandle, (int)self.track.trackId, (int)fromIndex, (int)index);
    if (ve_err != VE_ERR_OK) {
        LogError(@"track move clip error trackId[%ld] fromIndex[%ld] toIndex[%ld]",self.track.trackId, fromIndex, index);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (ve_clip)ve_setClipParams:(QHVCEditTrackClip *)clip
{
    VE_CLIP clipType = VE_CLIP_VIDEO;
    switch ([clip clipType])
    {
        case QHVCEditTrackClipTypeVideo:
        {
            clipType = VE_CLIP_VIDEO;
            break;
        }
        case QHVCEditTrackClipTypeAudio:
        {
            clipType = VE_CLIP_AUDIO;
            break;
        }
        case QHVCEditTrackClipTypeImage:
        {
            clipType = VE_CLIP_PICTURE;
            break;
        }
    }
    
    ve_clip param;
    param.track_id = (int)[self.track trackId];
    param.clip_id = (int)[clip clipId];
    param.type = clipType;
    
    //HEIF文件预处理
    NSString* originFilePath = [clip filePath];
    NSString* filePath = [clip filePath];
    BOOL isHeif = [QHVCEditUtils isHEICFile:[clip filePath]];
    if (isHeif)
    {
        originFilePath = [clip filePath];
        filePath = [QHVCEditUtils heicToJPEG:[clip filePath]];
    }
    param.filename = [filePath UTF8String];
    param.original_filename = [originFilePath UTF8String];
    param.duration = (int)[clip fileEndTime] - (int)[clip fileStartTime];
    param.insert_time = (int)[clip insertTime];
    if ([clip clipType] != QHVCEditTrackClipTypeImage)
    {
        param.start_time = (int)[clip fileStartTime];
        param.end_time = (int)[clip fileEndTime];
    }
    
    //计算图片旋转角度
    VE_ROTATE rotate = VE_ROTATE_0;
    UIImage* image = [UIImage imageWithContentsOfFile:filePath];
    if (image)
    {
        switch (image.imageOrientation)
        {
            case UIImageOrientationUp:
            {
                rotate = VE_ROTATE_0;
                break;
            }
            case UIImageOrientationRight:
            {
                rotate = VE_ROTATE_90;
                break;
            }
            case UIImageOrientationDown:
            {
                rotate = VE_ROTATE_180;
                break;
            }
            case UIImageOrientationLeft:
            {
                rotate = VE_ROTATE_270;
                break;
            }
            default:
            {
                rotate = VE_ROTATE_0;
                break;
            }
        }
    }
    param.picture_rotate = rotate;
    param.volume = (int)[clip volume];
    param.speed = [clip speed];
    param.pitch = (int)clip.pitch;
    
    //慢视频信息
    slv_info slvInfo;
    if ([[clip slowMotionVideoInfo] count] > 0)
    {
        slvInfo.active = true;
        slvInfo.len = (int)[[clip slowMotionVideoInfo] count];
        
        for (int i = 0; i < (int)[[clip slowMotionVideoInfo] count]; i++)
        {
            QHVCEditSlowMotionVideoInfo* info = [[clip slowMotionVideoInfo] objectAtIndex:i];
            slvInfo.start_time[i] = (int)info.startTime;
            slvInfo.end_time[i] = (int)info.endTime;
            slvInfo.speed[i] = (float)info.speed;
        }
        
        param.slv = slvInfo;
    }
    else
    {
        slvInfo.active = false;
    }
    
    return param;
}

- (QHVCEditError)ve_changeClipInsertTime:(NSInteger)clipId insertTime:(NSInteger)timeMs
{
    ve_clip veClip;
    VE_ERR err = ve_clip_get(_timelineHandle, (int)clipId, &veClip);
    if (err != VE_ERR_OK)
    {
        LogError(@"track change Clip InsertTime get clip error clipId[%d] error, [%@]", (int)clipId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    veClip.insert_time = (int)timeMs;
    err = ve_clip_mod(_timelineHandle, &veClip);
    
    if (err != VE_ERR_OK)
    {
        LogError(@"track change Clip InsertTime error clipId[%d] error, [%@]", (int)clipId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_updateClipParams:(QHVCEditTrackClip *)clip
{
    ve_clip veClip = [self ve_setClipParams:clip];
    VE_ERR err = ve_clip_mod(_timelineHandle, &veClip);
    if (err != VE_ERR_OK)
    {
        LogError(@"track update clip params error clipId[%d] error, [%@]", (int)clip.clipId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_deleteClipById:(NSInteger)clipId
{
    VE_ERR err = ve_clip_del(_timelineHandle,(int) clipId);
    if (err != VE_ERR_OK)
    {
        LogError(@"track delete Clip By Id error clipId[%d] error, [%@]", (int)clipId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_deleteClipAtIndex:(NSInteger)index
{
    int trackId = (int)self.track.trackId;
    VE_ERR err = ve_clip_del_by_index(_timelineHandle, trackId, (int)index);
    if (err != VE_ERR_OK)
    {
        LogError(@"track delete ClipAtIndex error index[%d] error, [%@]", (int)index, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_addTransitionToIndex:(NSInteger)index
                                duration:(NSInteger)durationMs
                                  action:(const char*)action
                            transitionId:(NSInteger)transitionId
{
    ve_transition veTransition;
    veTransition.transition_id = (int)transitionId;
    veTransition.track_id = (int)self.track.trackId;
    veTransition.clip_index_b = (int)index;
    veTransition.duration = (int)durationMs;
    veTransition.action = action;
    
    VE_ERR err = ve_transition_add(_timelineHandle, &veTransition);
    
    if (err != VE_ERR_OK)
    {
        LogError(@"track add TransitionToIndex error index[%d], durationMs [%d],  action[%s],  transitionId[%d], error[%@]", (int)index, (int)durationMs, action, (int)transitionId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_editTransitionAtIndex:(NSInteger)index
                                 duration:(NSInteger)durationMs
                                   action:(const char*)action
                             transitionId:(NSInteger)transitionId
{
    ve_transition veTransiton;
    VE_ERR err = ve_transition_get(_timelineHandle, (int)transitionId, &veTransiton);
    if (err != VE_ERR_OK)
    {
        LogError(@"track edit TransitionAtIndex get transition error transitionId[%d] error[%@]", (int)transitionId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    veTransiton.transition_id = (int)transitionId;
    veTransiton.clip_index_b = (int)index;
    veTransiton.duration = (int)durationMs;
    veTransiton.action = action;
    
    err = ve_transition_mod(_timelineHandle, &veTransiton);
    if (err != VE_ERR_OK)
    {
        LogError(@"track edit TransitionAtIndex mod transition transitionId[%d] error[%@]", (int)transitionId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_deleteTransition:(NSInteger)transitionId
{
    VE_ERR err = ve_transition_del(_timelineHandle, (int)transitionId);
    if (err != VE_ERR_OK)
    {
        LogError(@"track delete Transition error transitionId[%d] errorInfo[%@]", (int)transitionId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_addEffect:(QHVCEditEffect *)effect
{
    ve_filter veFilter;
    [self getVEFiterByEffect:effect veFilter:&veFilter];
    VE_ERR err = ve_filter_add(_timelineHandle, &veFilter);
    if (err != VE_ERR_OK)
    {
        LogError(@"track add effect error effectId[%d] errorInfo[%@]", (int)effect.effectId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_updateEffect:(QHVCEditEffect *)effect
{
    ve_filter veFilter;
    [self getVEFiterByEffect:effect veFilter:&veFilter];
    VE_ERR err = ve_filter_mod(_timelineHandle, &veFilter);
    if (err != VE_ERR_OK)
    {
        LogError(@"track update effect mod filter error effectId[%d] errorInfo[%@]", (int)effect.effectId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_deleteEffect:(NSInteger)effectId
{
    VE_ERR err = ve_filter_del(_timelineHandle, (int)effectId);
    if (err != VE_ERR_OK)
    {
        LogError(@"track delete effect error effectId[%d] errorInfo[%@]", (int)effectId, [[QHVCEditConfig sharedInstance] getErrorCodeInfo:err]);
        return QHVCEditErrorParamError;
    }
    return QHVCEditErrorNoError;
}

- (void)getVEFiterByEffect:(QHVCEditEffect *)effect veFilter:(ve_filter *)veFilter
{
    veFilter->track_id = (int)self.track.trackId;
    veFilter->filter_id = (int)effect.effectId;
    veFilter->type = (VE_FILTER)[[QHVCEditConfig sharedInstance] getVEFilterType:effect];
    veFilter->loc_type = VE_FILTER_LOC_TRACK;
    veFilter->start_time = (int)effect.startTime;
    veFilter->end_time = (int)effect.endTime;
    veFilter->action = "";
    
    if (effect.effectType == QHVCEditEffectTypeAudio)
    {
        QHVCEditAudioTransferEffect *audioTransfer = (QHVCEditAudioTransferEffect *)effect;
        veFilter->af_type = (VE_AUDIO_FILTER_TYPE)audioTransfer.transferType;
        veFilter->fade_curve = (VE_AF_FADE_CURVE)audioTransfer.transferCurveType;
        veFilter->gain_min = (int)audioTransfer.gainMin;
        veFilter->gain_max = (int)audioTransfer.gainMax;
    }
}

@end
