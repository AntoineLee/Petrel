//
//  QHVCEditMediaEditor.h
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/6/26.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditTrackClipItem.h"
#import <QHVCEditKit/QHVCEditKit.h>

@interface QHVCEditMediaEditor : NSObject

+ (instancetype)sharedInstance;

#pragma mark -  timeline

- (void)createTimeline;
- (void)freeTimeline;
- (QHVCEditTimeline *)getTimeline;
- (NSTimeInterval)getTimelineDuration;

#pragma mark - main video track

- (void)createMainTrack;
- (void)mainTrackAppendClip:(QHVCEditTrackClipItem *)item;
- (void)mainTrackDeleteClip:(QHVCEditTrackClipItem *)item;
- (void)mainTrackInsertClip:(QHVCEditTrackClipItem *)item atIndex:(NSInteger)index;
- (void)mainTrackUpdateClip:(QHVCEditTrackClipItem *)item;
- (void)mainTrackMoveClip:(NSInteger)fromIndex toIndex:(NSInteger)index;
- (QHVCEditSequenceTrack *)getMainVideoTrack;
- (NSMutableArray<QHVCEditTrackClipItem *>*)getMainTrackClips;

- (void)mainTrackAddTransition:(NSInteger)index
                      duration:(NSInteger)duration
                transitionName:(NSString *)transitionName
            easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (void)mainTrackUpdateTransition:(NSInteger)index
                         duration:(NSInteger)duration
                   transitionName:(NSString *)transitionName
               easingFunctionType:(QHVCEditEasingFunctionType)easingFunctionType;

- (void)mainTrackDeleteTransition:(NSInteger)index;

- (void)setMainTrackVolume:(NSInteger)volume;
- (void)setMainTrackPitch:(NSInteger)pitch;

#pragma mark - video overlay track

- (void)addOverlayClip:(QHVCEditTrackClipItem *)item;
- (void)deleteOverlayClip:(QHVCEditTrackClipItem *)item;
- (void)updateOverlayClipParams:(QHVCEditTrackClipItem *)item;

- (QHVCEditVideoTransferEffect *)createEffectVideoAnimation;
- (void)overlayClip:(QHVCEditTrackClipItem *)item addVideoAnimation:(QHVCEditVideoTransferEffect *)effect;
- (void)overlayClip:(QHVCEditTrackClipItem *)item updateVideoAnimation:(QHVCEditVideoTransferEffect *)effect;

#pragma mark - main audio track

- (void)createMainAudioTrack;
- (void)deleteMainAudioTrack;
- (void)mainAudioTrackAppendClip:(QHVCEditTrackClipItem *)item;
- (void)mainAudioTrackAppendClips:(NSArray<QHVCEditTrackClipItem *>*)items;
- (void)mainAudioTrackDeleteClip:(QHVCEditTrackClipItem *)item;
- (void)mainAudioTrackDeleteClips:(NSArray<QHVCEditTrackClipItem *>*)items;
- (void)mainAudioTrackDeleteAllClips;
- (NSMutableArray<QHVCEditTrackClipItem *>*)getMainAduioTrackClips;
- (void)setMainAduioTrackVolume:(NSInteger)volume;
- (void)setMainAduioTrackPitch:(NSInteger)pitch;
- (QHVCEditTrack *)getMainAudioTrack;

#pragma mark - player

- (QHVCEditPlayer *)createPlayerWithView:(UIView *)view delegate:(id<QHVCEditPlayerDelegate>)delegate;
- (void)freePlayer:(QHVCEditPlayer *)player;

#pragma mark - thumbnail

//单张缩略图
- (QHVCEditError)requestThumb:(NSString *)filePath
            timestamp:(NSTimeInterval)timeMs
            thumbSize:(CGSize)size
        thumbCallback:(void (^)(QHVCEditThumbnailItem* thumbnail))callback;

//多张缩略图
- (QHVCEditError)requestThumbs:(NSString *)filePath
                start:(NSTimeInterval)startMs
                  end:(NSTimeInterval)endMs
             frameCnt:(int)count
            thumbSize:(CGSize)size
                 thumbCallback:(void (^)(QHVCEditThumbnailItem* thumbnail))callback;

- (QHVCEditError)cancelThumbnailRequest;
- (QHVCEditError)freeThumbnailBuilder;

#pragma mark - Producer

- (QHVCEditProducer *)createProducerWithDelegate:(id<QHVCEditProducerDelegate>)delegate;
- (void)freeProducer:(QHVCEditProducer *)producer;

#pragma mark - Effect

//滤镜
- (QHVCEditFilterEffect *)createEffectFilter;
- (void)addEffectFilterToTimeline:(QHVCEditFilterEffect *)filter;
- (void)updateEffectFilter:(QHVCEditFilterEffect *)filter;
- (void)deleteEffectFilter:(QHVCEditFilterEffect *)filter;

//贴纸
- (QHVCEditStickerEffect *)createStickerEffect;
- (void)addStickerEffectToTimeline:(QHVCEditStickerEffect *)effect;
- (void)updateStickerEffect:(QHVCEditStickerEffect *)effect;
- (void)deleteStickerEffect:(QHVCEditStickerEffect *)effect;

//水印
- (QHVCEditStickerEffect *)createWatermarkEffect;
- (void)addWatermarkEffectToTimeline:(QHVCEditStickerEffect *)effect;
- (void)deleteWatermarkEffect:(QHVCEditStickerEffect *)effect;

#pragma mark - Propertys

@property (nonatomic, readonly, retain) QHVCEditTimeline* timeline;
@property (nonatomic, readonly, strong) QHVCEditThumbnail* thumbnail;

@property (nonatomic, readonly, retain) QHVCEditSequenceTrack* mainTrack;
@property (nonatomic, readonly, strong) NSMutableArray<QHVCEditTrackClipItem *>* mainTrackClips;

@property (nonatomic, readonly, retain) NSMutableArray<QHVCEditTrack *>* overlayTracks;
@property (nonatomic, readonly, retain) NSMutableArray<QHVCEditTrackClipItem *>* overlayTrackClips;

@property (nonatomic, readonly, strong) QHVCEditOverlayTrack *mainAudioTrack;
@property (nonatomic, readonly, strong) NSMutableArray<QHVCEditTrackClipItem *>* mainAudioTrackClips;

@end

