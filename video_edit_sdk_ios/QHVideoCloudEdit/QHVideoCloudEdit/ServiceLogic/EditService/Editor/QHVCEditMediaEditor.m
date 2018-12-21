//
//  QHVCEditMediaEditor.m
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/6/26.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditMediaEditor.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditMediaEditorConfig.h"

@interface QHVCEditMediaEditor ()

@property (nonatomic, retain) QHVCEditTimeline* timeline;
@property (nonatomic, strong) QHVCEditThumbnail* thumbnail;

@property (nonatomic, retain) QHVCEditSequenceTrack* mainTrack;
@property (nonatomic, strong) NSMutableArray<QHVCEditTrackClipItem *>* mainTrackClips;

@property (nonatomic, retain) NSMutableArray<QHVCEditTrack *>* overlayTracks;
@property (nonatomic, retain) NSMutableArray<QHVCEditTrackClipItem *>* overlayTrackClips;

@property (nonatomic, strong) QHVCEditOverlayTrack *mainAudioTrack;
@property (nonatomic, strong) NSMutableArray<QHVCEditTrackClipItem *>* mainAudioTrackClips;

@end

@implementation QHVCEditMediaEditor

+ (instancetype)sharedInstance
{
    static QHVCEditMediaEditor* s_instance = nil;
    static dispatch_once_t predic;
    dispatch_once(&predic, ^{
        s_instance = [[QHVCEditMediaEditor alloc] init];
    });
    return s_instance;
}

- (instancetype)init
{
    if (!(self = [super init]))
    {
        return nil;
    }
    
    self.mainTrackClips = [[NSMutableArray alloc] initWithCapacity:0];
    [QHVCEditTools setSDKLogLevel:QHVCEditLogLevelDebug];
    return self;
}

#pragma mark - timeline

- (void)createTimeline
{
    CGSize outputSize = [[QHVCEditMediaEditorConfig sharedInstance] outputSize];
    NSInteger fps = [[QHVCEditMediaEditorConfig sharedInstance] outputFps];
    NSInteger bitrate = [[QHVCEditMediaEditorConfig sharedInstance] outputVideoBitrate];
    self.timeline = [[QHVCEditTimeline alloc] initTimeline];
    [self.timeline setOutputWidth:outputSize.width height:outputSize.height];
    [self.timeline setOutputFps:fps];
    [self.timeline setOutputBitrate:bitrate];
}

- (void)freeTimeline
{
    [self.timeline free];
    [self.mainTrackClips removeAllObjects];
    self.mainTrack = nil;
    [self.overlayTracks removeAllObjects];
    [self.overlayTrackClips removeAllObjects];
    [self.mainAudioTrackClips removeAllObjects];
    self.mainAudioTrack = nil;
}

- (QHVCEditTimeline *)getTimeline
{
    return self.timeline;
}

- (NSTimeInterval)getTimelineDuration
{
    return [self.timeline duration];
}

#pragma mark - main video track

- (void)createMainTrack
{
    self.mainTrack = [[QHVCEditSequenceTrack alloc] initWithTimeline:self.timeline type:QHVCEditTrackTypeVideo];
    [self.timeline appendTrack:self.mainTrack];
}

- (void)mainTrackAppendClip:(QHVCEditTrackClipItem *)item
{
    if (!item)
    {
        return;
    }
    
    QHVCEditTrackClip* clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:self.timeline];
    [clip setFilePath:item.filePath type:item.clipType];
    [clip setFileStartTime:0];
    [clip setFileEndTime:item.durationMs];
    [clip setSlowMotionVideoInfo:item.slowMotionInfo];
    item.clip = clip;
    [self.mainTrack appendClip:clip];
    [self.mainTrackClips addObject:item];
}

- (void)mainTrackDeleteClip:(QHVCEditTrackClipItem *)item
{
    if (!item)
    {
        return;
    }
    
    [self.mainTrack deleteClipById:item.clip.clipId];
    [self.mainTrackClips removeObject:item];
}

- (void)mainTrackInsertClip:(QHVCEditTrackClipItem *)item atIndex:(NSInteger)index
{
    if (!item)
    {
        return;
    }
    
    [self.mainTrack insertClip:item.clip atIndex:index];
    [self.mainTrackClips insertObject:item atIndex:index];
}

- (void)mainTrackUpdateClip:(QHVCEditTrackClipItem *)item
{
    if (!item)
    {
        return;
    }
    
    [self.mainTrack updateClipParams:item.clip];
}

- (void)mainTrackMoveClip:(NSInteger)fromIndex toIndex:(NSInteger)index
{
    if (fromIndex >= 0 && index >= 0 && fromIndex != index)
    {
        [self.mainTrack moveClip:fromIndex toIndex:index];
    }
}

- (QHVCEditSequenceTrack *)getMainVideoTrack
{
    return self.mainTrack;
}

- (NSMutableArray<QHVCEditTrackClipItem *>*)getMainTrackClips
{
    return self.mainTrackClips;
}

- (void)mainTrackAddTransition:(NSInteger)index
                      duration:(NSInteger)duration
                transitionName:(NSString *)transitionName
            easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    [self.mainTrack addVideoTransitionToIndex:index duration:duration videoTransitionName:transitionName easingFunctionType:easingFunctionType];
}

- (void)mainTrackUpdateTransition:(NSInteger)index
                         duration:(NSInteger)duration
                   transitionName:(NSString *)transitionName
               easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType
{
    [self.mainTrack updateVideoTransitionAtIndex:index duration:duration videoTransitionName:transitionName easingFunctionType:easingFunctionType];
}

- (void)mainTrackDeleteTransition:(NSInteger)index
{
    [self.mainTrack deleteVideoTransition:index];
}

- (void)setMainTrackVolume:(NSInteger)volume
{
    if (_mainTrack)
    {
        [_mainTrack setVolume:volume];
    }
}

- (void)setMainTrackPitch:(NSInteger)pitch
{
    if (_mainTrack)
    {
        [self.mainTrackClips enumerateObjectsUsingBlock:^(QHVCEditTrackClipItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
        {
            [obj.clip setPitch:pitch];
            [self.mainTrack updateClipParams:obj.clip];
        }];
    }
}

#pragma mark - video overlay track

- (void)addOverlayClip:(QHVCEditTrackClipItem *)item
{
    if (!item)
    {
        return;
    }
    
    QHVCEditOverlayTrack* overlayTrack = [[QHVCEditOverlayTrack alloc] initWithTimeline:self.timeline type:QHVCEditTrackTypeVideo];
    [self.timeline appendTrack:overlayTrack];
    
    QHVCEditTrackClip* clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:self.timeline];
    [clip setFilePath:item.filePath type:item.clipType];
    [clip setFileStartTime:0];
    [clip setFileEndTime:item.durationMs];
    [clip setSlowMotionVideoInfo:item.slowMotionInfo];
    item.clip = clip;
    [overlayTrack addClip:clip atTime:0];
    [self.overlayTracks addObject:overlayTrack];
    [self.overlayTrackClips addObject:item];
}

- (void)deleteOverlayClip:(QHVCEditTrackClipItem *)item
{
    if (!item)
    {
        return;
    }
    
    if (!item.clip)
    {
        return;
    }
    
    QHVCEditOverlayTrack* track = (QHVCEditOverlayTrack *)[item.clip superObj];
    [track deleteClipById:item.clip.clipId];
    [self.overlayTrackClips removeObject:item];
    [self.overlayTracks removeObject:track];
}

- (void)updateOverlayClipParams:(QHVCEditTrackClipItem *)item
{
    if (!item)
    {
        return;
    }
    
    if (!item.clip)
    {
        return;
    }
    
    QHVCEditOverlayTrack* track = (QHVCEditOverlayTrack *)[item.clip superObj];
    [track updateClipParams:item.clip];
}

- (QHVCEditVideoTransferEffect *)createEffectVideoAnimation
{
    QHVCEditVideoTransferEffect* effect = [[QHVCEditVideoTransferEffect alloc] initEffectWithTimeline:self.timeline];
    return effect;
}

- (void)overlayClip:(QHVCEditTrackClipItem *)item addVideoAnimation:(QHVCEditVideoTransferEffect *)effect
{
    if (!item || !item.clip)
    {
        return;
    }
    
    [item.clip addEffect:effect];
}

- (void)overlayClip:(QHVCEditTrackClipItem *)item updateVideoAnimation:(QHVCEditVideoTransferEffect *)effect
{
    if (!item || !item.clip)
    {
        return;
    }
    
    [item.clip updateEffect:effect];
}

#pragma mark - main audio track

- (void)createMainAudioTrack
{
    self.mainAudioTrack = [[QHVCEditOverlayTrack alloc] initWithTimeline:self.timeline type:QHVCEditTrackTypeAudio];
    [self.timeline appendTrack:self.mainAudioTrack];
    _mainAudioTrackClips = [[NSMutableArray alloc] init];
}

- (void)deleteMainAudioTrack
{
    [self.timeline deleteTrackById:self.mainAudioTrack.trackId];
}

- (void)mainAudioTrackAppendClip:(QHVCEditTrackClipItem *)item
{
    if (!item) {
        return;
    }
    
    if (!self.mainAudioTrack) {
        [self createMainAudioTrack];
    }
    
    QHVCEditTrackClip *clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:self.timeline];
    [clip setFilePath:item.filePath type:item.clipType];
    [clip setFileStartTime:0];
    [clip setFileEndTime:item.endMs];
    item.clip = clip;
    [self.mainAudioTrack addClip:clip atTime:item.insertMs];
    [self.mainAudioTrackClips addObject:item];
}

- (void)mainAudioTrackAppendClips:(NSArray<QHVCEditTrackClipItem *> *)items
{
    if (!items && items.count == 0) {
        return;
    }
    
    [items enumerateObjectsUsingBlock:^(QHVCEditTrackClipItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        obj.clipIndex = idx;
        [self mainAudioTrackAppendClip:obj];
    }];
}

- (void)mainAudioTrackDeleteClip:(QHVCEditTrackClipItem *)item
{
    if (!item)
    {
        return;
    }
    
    [self.mainAudioTrack deleteClipById:item.clip.clipId];
    [self.mainAudioTrackClips removeObject:item];
}

- (void)mainAudioTrackDeleteClips:(NSArray<QHVCEditTrackClipItem *> *)items
{
    if (!items)
    {
        return;
    }
    
    [items enumerateObjectsUsingBlock:^(QHVCEditTrackClipItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [self mainAudioTrackDeleteClip:obj];
    }];
    
    [self.mainAudioTrackClips removeObjectsInArray:items];
}

- (void)mainAudioTrackDeleteAllClips
{
    [_mainAudioTrackClips enumerateObjectsUsingBlock:^(QHVCEditTrackClipItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [self mainAudioTrackDeleteClip:obj];
    }];
    
    [_mainAudioTrackClips removeAllObjects];
}


- (NSMutableArray<QHVCEditTrackClipItem *>*)getMainAduioTrackClips
{
    return self.mainAudioTrackClips;
}

- (QHVCEditTrack *)getMainAudioTrack
{
    return self.mainAudioTrack;
}


- (void)setMainAduioTrackVolume:(NSInteger)volume
{
    if (self.mainAudioTrack) {
        [self.mainAudioTrack setVolume:volume];
    }
}

- (void)setMainAduioTrackPitch:(NSInteger)pitch
{
    if (self.mainAudioTrack) {
        [self.mainAudioTrackClips enumerateObjectsUsingBlock:^(QHVCEditTrackClipItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
            [obj.clip setPitch:pitch];
        }];
    }
}

#pragma mark - player

- (QHVCEditPlayer *)createPlayerWithView:(UIView *)view delegate:(id<QHVCEditPlayerDelegate>)delegate
{
    QHVCEditPlayer* player = [[QHVCEditPlayer alloc] initWithTimeline:self.timeline];
    [player setDelegate:delegate];
    [player setPreview:view];
    return player;
}

- (void)freePlayer:(QHVCEditPlayer *)player
{
    [player free];
}

#pragma mark - thumbnail
    
- (QHVCEditError)requestThumbs:(NSString *)filePath start:(NSTimeInterval)startMs end:(NSTimeInterval)endMs frameCnt:(int)count thumbSize:(CGSize)size thumbCallback:(void (^)(QHVCEditThumbnailItem* thumbnail))callback
{
    if (!_thumbnail) {
        _thumbnail = [[QHVCEditThumbnail alloc] init];
    }
    __block QHVCEditError error = QHVCEditErrorNoError;
    [_thumbnail requestThumbnailFromFile:filePath width:size.width height:size.height startTime:startMs endTime:endMs count:count dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {
        if (error == QHVCEditErrorNoError) {
            SAFE_BLOCK(callback, thumbnail);
        }else{
            error = QHVCEditErrorRequestThumbnailError;
        }
    }];
    
    return error;
}

- (QHVCEditError)requestThumb:(NSString *)filePath timestamp:(NSTimeInterval)timeMs thumbSize:(CGSize)size thumbCallback:(void (^)(QHVCEditThumbnailItem* thumbnail))callback
{
    if (!_thumbnail) {
        _thumbnail = [[QHVCEditThumbnail alloc] init];
    }
    __block QHVCEditError error = QHVCEditErrorNoError;
    [_thumbnail requestThumbnailFromFile:filePath width:size.width height:size.height timestamp:timeMs dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {
        if (error == QHVCEditErrorNoError) {
            SAFE_BLOCK(callback, thumbnail);
        }else{
            error = QHVCEditErrorRequestThumbnailError;
        }
    }];
    
    return error;
}

- (QHVCEditError)cancelThumbnailRequest
{
    if (_thumbnail) {
        [_thumbnail cancelAllThumbnailRequest];
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)freeThumbnailBuilder
{
    if (_thumbnail) {
        [_thumbnail free];
        _thumbnail = nil;
    }
    
    return QHVCEditErrorNoError;
}

#pragma mark - Producer

- (QHVCEditProducer *)createProducerWithDelegate:(id<QHVCEditProducerDelegate>)delegate
{
    QHVCEditProducer* producer = [[QHVCEditProducer alloc] initWithTimeline:self.timeline];
    [producer setDelegate:delegate];
    return producer;
}

- (void)freeProducer:(QHVCEditProducer *)producer
{
    [producer free];
}

#pragma mark - Effect

//滤镜
- (QHVCEditFilterEffect *)createEffectFilter
{
    QHVCEditFilterEffect* filter = [[QHVCEditFilterEffect alloc] initEffectWithTimeline:self.timeline];
    return filter;
}

- (void)addEffectFilterToTimeline:(QHVCEditFilterEffect *)filter
{
    [self.timeline addEffect:filter];
}

- (void)updateEffectFilter:(QHVCEditFilterEffect *)filter
{
    [self.timeline updateEffect:filter];
}

- (void)deleteEffectFilter:(QHVCEditFilterEffect *)filter
{
    [self.timeline deleteEffectById:filter.effectId];
}

//贴纸
- (QHVCEditStickerEffect *)createStickerEffect
{
    QHVCEditStickerEffect* effect = [[QHVCEditStickerEffect alloc] initEffectWithTimeline:self.timeline];
    return effect;
}

- (void)addStickerEffectToTimeline:(QHVCEditStickerEffect *)effect
{
    [self.timeline addEffect:effect];
}

- (void)updateStickerEffect:(QHVCEditStickerEffect *)effect
{
    [self.timeline updateEffect:effect];
}

- (void)deleteStickerEffect:(QHVCEditStickerEffect *)effect
{
    [self.timeline deleteEffectById:effect.effectId];
}

//水印
- (QHVCEditStickerEffect *)createWatermarkEffect
{
    QHVCEditStickerEffect* effect = [[QHVCEditStickerEffect alloc] initEffectWithTimeline:self.timeline];
    return effect;
}

- (void)addWatermarkEffectToTimeline:(QHVCEditStickerEffect *)effect
{
    [self.timeline addEffect:effect];
}

- (void)deleteWatermarkEffect:(QHVCEditStickerEffect *)effect
{
    [self.timeline deleteEffectById:effect.effectId];
}

@end
