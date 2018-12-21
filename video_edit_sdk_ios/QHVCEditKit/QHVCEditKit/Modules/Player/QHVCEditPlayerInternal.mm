//
//  QHVCEditPlayerInternal.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/5/11.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditPlayerInternal.h"
#import <CoreVideo/CoreVideo.h>
#import "QHVCEditPlayer.h"
#import "QHVCEditCommonDef.h"
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditConfig.h"
#import "QHVCEditEditorManager.h"
#import "QHVCEditEffect.h"
#import "QHVCEditEffectManager.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditEditor+Track.h"
#import "QHVCEditEditor+Clip.h"
#import "QHVCEditTrackClipManager.h"
#import "QHVCEditCIPreview.h"
#import "QHVCEditRenderOperation.h"
#import "QHVCEditRenderParam.h"
#import "QHVCEditTrackClip.h"
#import "QHVCEditTrack.h"
#import "ve_interface.h"

void edit_player_frame_cb(HANDLE player_handle, ve_filter_callback_param* param, void* userExt)
{
    QHVCEditPlayerInternal* player = (__bridge QHVCEditPlayerInternal *)(userExt);
    [player onPlayerDataCB:player_handle data:param];
}

void edit_player_event_cb(HANDLE player_handle, int status, void* userExt)
{
    QHVCEditPlayerInternal* player = (__bridge QHVCEditPlayerInternal *)userExt;
    [player onPlayerEventCB:player_handle status:status];
}

typedef void(^QHVCEditPlayerCompletion)();
typedef void(^QHVCEditPlayerSeekCB)(NSInteger currentTime);

@interface QHVCEditPlayerInternal () <QHVCEditRenderOperationDelegate>
{
    HANDLE _playerHandle;
}

@property (nonatomic,   weak) id<QHVCEditPlayerDelegate> delegate;
@property (nonatomic, assign) BOOL isPlayingState;
@property (nonatomic, assign) BOOL firstFrameRendered;
@property (nonatomic,   weak) UIView* previewCanvas;
@property (nonatomic, retain) dispatch_queue_t playerQueue;
@property (nonatomic, assign) NSTimeInterval currentTimestamp;
@property (nonatomic, assign) NSTimeInterval totalDuration;
@property (nonatomic, assign) NSInteger inputCount;
@property (nonatomic, assign) NSInteger renderedCount;

//timeline
@property (nonatomic, retain) QHVCEditTimeline* timeline;
@property (nonatomic, retain) QHVCEditEditor* editor;

//render
@property (nonatomic, retain) QHVCEditRenderOperation* renderOperation;
@property (nonatomic, retain) QHVCEditCIPreview* playerCIPreview;

//seek && refresh
@property (nonatomic,   copy) QHVCEditPlayerSeekCB seekCallback;
@property (nonatomic,   copy) QHVCEditPlayerCompletion refreshCompletion;
@property (nonatomic, assign) BOOL isRefreshing;
@property (nonatomic, retain) QHVCEditRenderParam* lastRenderParam;

@end

@implementation QHVCEditPlayerInternal

#pragma mark - Life Circle Methods

- (void)dealloc
{
    if (self.previewCanvas)
    {
        [_previewCanvas removeObserver:self forKeyPath:@"bounds"];
    }
    [self ve_freePlayer];
}

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        if (!timeline)
        {
            LogError(@"player initWithTimeline error, timeline is nil");
            return nil;
        }
        
        self.editor = [[QHVCEditEditorManager sharedInstance] getEditor:[timeline timelineId]];
        if (self.editor)
        {
            self.timeline = timeline;
            self.isPlayingState = NO;
            self.playerQueue = dispatch_queue_create("QHVCEditPlayerQueue", NULL);
        }
    }
    
    return self;
}

- (QHVCEditError)setPlayerDelegate:(id<QHVCEditPlayerDelegate>)delegate
{
    _delegate = delegate;
    return QHVCEditErrorNoError;
}

- (QHVCEditError)free
{
    if (self.previewCanvas)
    {
        QHVCEDIT_WEAK_SELF
        dispatch_async(dispatch_get_main_queue(), ^{
            QHVCEDIT_STRONG_SELF
            [self.playerCIPreview removeFromSuperview];
            [self.previewCanvas removeObserver:self forKeyPath:@"bounds"];
            self.previewCanvas = nil;
            self.playerCIPreview = nil;
        });
    }
    
    self.delegate = nil;
    self.renderOperation = nil;
    self.isRefreshing = NO;
    self.refreshCompletion = nil;
    
    //释放播放器
    [self ve_freePlayer];
    return QHVCEditErrorNoError;
}

#pragma mark - Player Event Methods

- (QHVCEditError)playerPlay
{
    //播放
    QHVCEditError error = [self ve_playPlayer];
    return error;
}

- (QHVCEditError)playerStop
{
    //暂停
    [self ve_stopPlayer];
    return QHVCEditErrorNoError;
}

- (QHVCEditError)playerSeekToTime:(NSInteger)timestamp
                     forceRequest:(BOOL)forceRequest
                         complete:(void(^)(NSInteger currentTimeMs))block
{
    self.seekCallback = block;
    [self ve_playerSeekToTime:timestamp forceRequest:forceRequest];
    return QHVCEditErrorNoError;
}

- (QHVCEditError)resetPlayer:(NSInteger)seekTimestamp
{
    //释放已有播放器
    QHVCEditError error = [self ve_freePlayer];
    
    //创建新播放器
    error = [self ve_createPlayer:seekTimestamp];
    if (error != QHVCEditErrorNoError)
    {
        return error;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)refreshPlayer:(BOOL)forBasicParams forceRefresh:(BOOL)forceRefresh completion:(void (^)())completion
{
    self.isRefreshing = YES;
    self.refreshCompletion = completion;
    if (forBasicParams)
    {
        [self ve_refreshPlayer];
        forceRefresh = YES;
        if (!self.isPlayingState)
        {
            [self playerSeekToTime:self.currentTimestamp forceRequest:forceRefresh complete:_seekCallback];
        }
    }
    else
    {
        if (self.isPlayingState)
        {
            return QHVCEditErrorNoError;
        }
        
        self.inputCount ++;
        QHVCEDIT_WEAK_SELF
        dispatch_async(self.playerQueue, ^{
            QHVCEDIT_STRONG_SELF
            [self refreshRenderParams];
            [self.renderOperation processRenderParam:self.lastRenderParam];
        });
    }
    
    return QHVCEditErrorNoError;
}

#pragma mark - Player Status Methods

- (BOOL)isPlaying
{
    return _isPlayingState;
}

- (NSInteger)getCurrentTimestamp
{
    return _currentTimestamp;
}

- (NSInteger)getPlayerDuration
{
    return _totalDuration;
}

- (UIImage *)getCurrentFrame
{
    CIImage* ciImage = [self.playerCIPreview getCurrentFrame];
    CIContext *context = [CIContext contextWithOptions:nil];
    CGImageRef cgImage = [context createCGImage:ciImage fromRect:[ciImage extent]];
    UIImage* image = nil;
    if (cgImage)
    {
        image = [[UIImage alloc] initWithCGImage:cgImage];
        CFRelease(cgImage);
    }
    return image;
}

#pragma mark - Preview Methods

- (QHVCEditError)setPreview:(UIView *)preview
{
    //参数检测
    if (self.editor.outputSize.width <= 0 || self.editor.outputSize.height <= 0)
    {
        LogError(@"player setPreview error, output size <= 0, need set [QHVCEditTimeline setOutputWidth:height:] first");
        return QHVCEditErrorParamError;
    }

    if (!preview)
    {
        LogError(@"player setPreview error, preview is nil");
        return QHVCEditErrorParamError;
    }

    //移除对旧画布的监听
    if (self.previewCanvas)
    {
        [self.previewCanvas removeObserver:self forKeyPath:@"bounds"];
        self.previewCanvas = nil;
    }

    //从旧画布移除渲染窗口
    if (self.playerCIPreview)
    {
        [self.playerCIPreview removeFromSuperview];
    }

    //渲染窗口添加至新画布
    self.previewCanvas = preview;
    [self.playerCIPreview setFrame:CGRectMake(0, 0, CGRectGetWidth(preview.frame), CGRectGetHeight(preview.frame))];
    [preview addSubview:self.playerCIPreview];
    [self.previewCanvas addObserver:self forKeyPath:@"bounds" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionInitial context:nil];

    //创建 render operation
    [self createRenderOperation];

    //创建播放器
    QHVCEditError err = [self ve_createPlayer:0];
    if (err != QHVCEditErrorNoError)
    {
        LogError(@"player setPreview error, create player error");
        return err;
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)setPreviewFillMode:(QHVCEditFillMode)fillMode
{
    [self.playerCIPreview setFillMode:fillMode];
    return QHVCEditErrorNoError;
}

- (QHVCEditError)setPreviewBgColor:(NSString *)color
{
    UIColor* argb = [QHVCEditUtils colorForHex:color];
    CGFloat red, green, blue, alpha;
    BOOL ret = [argb getRed:&red green:&green blue:&blue alpha:&alpha];
    if (ret)
    {
        [self.playerCIPreview setBackgroundColorRed:red green:green blue:blue alpha:alpha];
    }
    
    return QHVCEditErrorNoError;
}

- (QHVCEditCIPreview *)playerCIPreview
{
    if (!_playerCIPreview)
    {
        _playerCIPreview = [[QHVCEditCIPreview alloc] init];
    }
    
    return _playerCIPreview;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (object == self.previewCanvas && [keyPath isEqualToString:@"bounds"])
    {
        UIView* view = (UIView *)object;
        [self.playerCIPreview setFrame:CGRectMake(0, 0, CGRectGetWidth(view.frame), CGRectGetHeight(view.frame))];
    }
}

#pragma mark - Render Operation Methods

- (QHVCEditError)createRenderOperation
{
    //重新创建render operation
    self.renderOperation = [[QHVCEditRenderOperation alloc] initWithEditor:self.editor output:self.playerCIPreview];
    [self.renderOperation setOutputSize:[self.editor outputSize] color:[self.editor outputBgColor]];
    [self.renderOperation setDelegate:self];
    return QHVCEditErrorNoError;
}

#pragma mark - video_edit Callback Methods

- (void)onPlayerEventCB:(void *)playerHandle status:(int)status
{
    LogInfo(@"on player event, status [%d]", status);
    if (status == VE_PLAYER_SEEK_COMPLETE)
    {
        //seek 完成
        if (_seekCallback)
        {
            _seekCallback(self.currentTimestamp);
            _seekCallback = nil;
        }
    }
    else if (status == VE_PLAYER_PLAYBACK_COMPLETE)
    {
        //播放完成
        self.isPlayingState = NO;
        self.currentTimestamp = self.totalDuration;
        [self ve_stopPlayer];
        if (self.delegate && [self.delegate respondsToSelector:@selector(onPlayerPlayComplete)])
        {
            [self.delegate onPlayerPlayComplete];
        }
    }
    else
    {
        LogError(@"player status error, status [%d]", status);
        if (self.delegate && [self.delegate respondsToSelector:@selector(onPlayerError:detail:)])
        {
            [self.delegate onPlayerError:QHVCEditErrorPlayerStatusError detail:[NSString stringWithFormat:@"%d", status]];
        }
    }
}

- (void)onPlayerDataCB:(void *)playerHandle data:(void *)data
{
    @autoreleasepool
    {
        QHVCEditRenderParam* renderParam = [self resolveRenderParams:data];
        
        LogDebug(@"on player frame, timestamp [%ld], [%ld]track [%ld]timeline effects",
                (long)renderParam.timestampMs,
                (long)[renderParam.tracks count],
                (long)[renderParam.effects count]);
        
        //绘制慢，需要做丢帧处理
        if (self.inputCount - self.renderedCount >= 2)
        {
            LogDebug(@"player drop frame");
            return;
        }
        
        self.lastRenderParam = renderParam;
        self.inputCount ++;
        QHVCEDIT_WEAK_SELF
        dispatch_async(self.playerQueue, ^{
            QHVCEDIT_STRONG_SELF
            [self.renderOperation processRenderParam:renderParam];
        });
    }
}

- (CIImage *)pixelbufferToCIImage:(CVPixelBufferRef)pixelbuffer
{
    CIImage *sourceImage = nil;
    if (pixelbuffer)
    {
        //pixelbuffer 转 CIImage
        sourceImage = [CIImage imageWithCVPixelBuffer:pixelbuffer options:nil];
        CVPixelBufferRelease(pixelbuffer);
    }
    
    return sourceImage;
}

- (void)refreshRenderParams
{
    for (QHVCEditRenderTrack* renderTrack in self.lastRenderParam.tracks)
    {
        //获取对应track对象
        QHVCEditTrack* track = [self.editor timelineGetTrackById:renderTrack.trackId];
        if (!track)
        {
            LogWarn(@"player resolve render params, track not exist");
            continue;
        }
        
        //获取track渲染属性
        CGSize outputSize = [self.editor outputSize];
        CGRect renderRect = [track renderRect];
        if (CGRectEqualToRect(renderRect, CGRectZero))
        {
            renderRect = CGRectMake(0, 0, outputSize.width, outputSize.height);
        }
        CGFloat renderRadian = [track renderRadian];
        QHVCEditFillMode fillMode = [track fillMode];
        QHVCEditBgParams* bgParams = [track bgParams];
        NSString* bgColor = @"FF000000";
        if (!bgParams)
        {
            bgColor = [self.editor outputBgColor];
        }
        else
        {
            if (bgParams.mode == QHVCEditBgModeColor)
            {
                bgColor = bgParams.bgInfo;
            }
        }
        
        //获取main clip对象
        QHVCEditTrackClip* clip = [self.editor track:track getClipById:renderTrack.mainClip.clipId];
        if (!clip)
        {
            LogError(@"plyer resolve render params, clip not exist");
            continue;
        }
        
        //mainFrame
        QHVCEditRenderClip* mainFrame = renderTrack.mainClip;
        mainFrame.bgMode =  bgParams.mode;
        mainFrame.bgColor = bgColor;
        mainFrame.fillMode = fillMode;
        mainFrame.renderRect = renderRect;
        mainFrame.outputSize = outputSize;
        mainFrame.previewRadian = renderRadian;
        mainFrame.frameRadian = [clip sourceRadian];
        mainFrame.sourceRect = [clip sourceRect];
        mainFrame.flipX = [clip flipX];
        mainFrame.flipY = [clip flipY];
        
        //transition
        QHVCEditRenderTransitionClip* transitionFrame = renderTrack.transitionClip;
        if (transitionFrame)
        {            
            //获取transition clip对象
            QHVCEditTrackClip* transitionClip = [self.editor track:track getClipById:renderTrack.transitionClip.clipId];
            if (!transitionClip)
            {
                LogWarn(@"player resolve render params, transition clip not exist");
                continue;
            }
            QHVCEditTrackClipManager* clipMgr = [self.editor clipGetManager:transitionClip];
            
            //transition frame
            transitionFrame = renderTrack.transitionClip;
            transitionFrame.bgMode =  bgParams.mode;
            transitionFrame.bgColor = bgColor;
            transitionFrame.fillMode = fillMode;
            transitionFrame.renderRect = renderRect;
            transitionFrame.outputSize = outputSize;
            transitionFrame.previewRadian = renderRadian;
            transitionFrame.frameRadian = [transitionClip sourceRadian];
            transitionFrame.sourceRect = [transitionClip sourceRect];
            transitionFrame.flipX = [transitionClip flipX];
            transitionFrame.flipY = [transitionClip flipY];
            transitionFrame.easingFunctionTyp = [clipMgr transitionEasingFunctionType];
        }
    }
}

- (QHVCEditRenderParam *)resolveRenderParams:(void *)data
{
    ve_filter_callback_param* value = (ve_filter_callback_param *)data;
    ve_multitrack_callback_param &ve_multiTrack = value->multitracks[0];
    NSInteger timestampMs = (NSInteger)value->cur_time;
    
    //timeline
    NSMutableArray<QHVCEditRenderTrack*>* tracks = [[NSMutableArray alloc] initWithCapacity:0];
    for (int i = 0; i < ve_multiTrack.tracks_num; i++)
    {
        ve_track_callback_param &trackParam = ve_multiTrack.tracks[i];
        ve_v_frame_callback_param &frameParam = trackParam.frame_data[0];
        ve_v_frame_callback_param &transitionParam = trackParam.frame_data[1];
        
        //获取对应track对象
        QHVCEditTrack* track = [self.editor timelineGetTrackById:trackParam.track_id];
        if (!track)
        {
            LogWarn(@"player resolve render params, track not exist");
            continue;
        }
        
        //获取track渲染属性
        CGSize outputSize = [self.editor outputSize];
        CGRect renderRect = [track renderRect];
        if (CGRectEqualToRect(renderRect, CGRectZero))
        {
            renderRect = CGRectMake(0, 0, outputSize.width, outputSize.height);
        }
        CGFloat renderRadian = [track renderRadian];
        QHVCEditFillMode fillMode = [track fillMode];
        QHVCEditBgParams* bgParams = [track bgParams];
        NSString* bgColor = @"FF000000";
        if (!bgParams)
        {
            bgColor = [self.editor outputBgColor];
        }
        else
        {
            if (bgParams.mode == QHVCEditBgModeColor)
            {
                bgColor = bgParams.bgInfo;
            }
        }
        
        //mainFrame effect
        NSMutableArray<QHVCEditRenderEffect *>* mainFrameEffects = [[NSMutableArray alloc] initWithCapacity:0];
        for (int j = 0; j < frameParam.clip_filters_len; j++)
        {
            ve_filter_param &filterParam = frameParam.clip_filters[j];
            QHVCEditRenderEffect* effect = [[QHVCEditRenderEffect alloc] init];
            effect.effectId = (NSInteger)filterParam.filter_id;
            effect.startTime = (NSInteger)filterParam.start_time;
            effect.endTime = (NSInteger)filterParam.end_time;
            effect.action = [NSString stringWithUTF8String:filterParam.action];
            [mainFrameEffects addObject:effect];
        }
        
        //获取main clip对象
        QHVCEditTrackClip* clip = [self.editor track:track getClipById:frameParam.clip_id];
        if (!clip)
        {
            LogError(@"plyer resolve render params, clip not exist");
            continue;
        }
        
        //mainFrame
        CVPixelBufferRetain((CVPixelBufferRef)frameParam.data);
        QHVCEditRenderClip* mainFrame = [[QHVCEditRenderClip alloc] init];
        mainFrame.clipId = frameParam.clip_id;
        mainFrame.inputImage = [self pixelbufferToCIImage:(CVPixelBufferRef)frameParam.data];
        mainFrame.effects = mainFrameEffects;
        mainFrame.sourceRotate = frameParam.rotate;
        mainFrame.bgMode =  bgParams.mode;
        mainFrame.bgColor = bgColor;
        mainFrame.fillMode = fillMode;
        mainFrame.renderRect = renderRect;
        mainFrame.outputSize = outputSize;
        mainFrame.previewRadian = renderRadian;
        mainFrame.frameRadian = [clip sourceRadian];
        mainFrame.sourceRect = [clip sourceRect];
        mainFrame.flipX = [clip flipX];
        mainFrame.flipY = [clip flipY];
        
        //transition
        QHVCEditRenderTransitionClip* transitionFrame = nil;
        if (transitionParam.transition_frame == 1)
        {
            //transition effect
            NSMutableArray<QHVCEditRenderEffect *>* transitionFrameEffects = [[NSMutableArray alloc] initWithCapacity:0];
            for (int k = 0; k < transitionParam.clip_filters_len; k++)
            {
                ve_filter_param &filterParam = transitionParam.clip_filters[k];
                QHVCEditRenderEffect* effect = [[QHVCEditRenderEffect alloc] init];
                effect.effectId = (NSInteger)filterParam.filter_id;
                effect.startTime = (NSInteger)filterParam.start_time;
                effect.endTime = (NSInteger)filterParam.end_time;
                effect.action = [NSString stringWithUTF8String:filterParam.action];
                [transitionFrameEffects addObject:effect];
            }
            
            //获取transition clip对象
            QHVCEditTrackClip* transitionClip = [self.editor track:track getClipById:transitionParam.clip_id];
            if (!transitionClip)
            {
                LogWarn(@"player resolve render params, transition clip not exist");
                continue;
            }
            QHVCEditTrackClipManager* clipMgr = [self.editor clipGetManager:transitionClip];
            
            //transition frame
            CVPixelBufferRetain((CVPixelBufferRef)transitionParam.data);
            transitionFrame = [[QHVCEditRenderTransitionClip alloc] init];
            transitionFrame.clipId = transitionParam.clip_id;
            transitionFrame.sourceRotate = transitionParam.rotate;
            transitionFrame.inputImage = [self pixelbufferToCIImage:(CVPixelBufferRef)transitionParam.data];
            transitionFrame.effects = transitionFrameEffects;
            transitionFrame.transitionId = (NSInteger)transitionParam.transition_id;
            transitionFrame.transitionName = [NSString stringWithUTF8String:transitionParam.transition_action];
            transitionFrame.easingFunctionTyp = [clipMgr transitionEasingFunctionType];
            transitionFrame.transitionStartTime = transitionParam.transition_start_time;
            transitionFrame.transitionEndTime = transitionParam.transition_end_time;
            transitionFrame.bgMode =  bgParams.mode;
            transitionFrame.bgColor = bgColor;
            transitionFrame.fillMode = fillMode;
            transitionFrame.renderRect = renderRect;
            transitionFrame.outputSize = outputSize;
            transitionFrame.previewRadian = renderRadian;
            transitionFrame.frameRadian = [transitionClip sourceRadian];
            transitionFrame.sourceRect = [transitionClip sourceRect];
            transitionFrame.flipX = [transitionClip flipX];
            transitionFrame.flipY = [transitionClip flipY];
        }
        
        //track effect
        QHVCEditRenderEffect* mixEffect = nil;
        NSMutableArray<QHVCEditRenderEffect *>* trackEffects = [[NSMutableArray alloc] initWithCapacity:0];
        for (int l = 0; l < trackParam.track_filters_len; l++)
        {
            ve_filter_param &filterParam = trackParam.track_filters[l];
            QHVCEditRenderEffect* renderEffect = [[QHVCEditRenderEffect alloc] init];
            renderEffect.effectId = (NSInteger)filterParam.filter_id;
            renderEffect.startTime = (NSInteger)filterParam.start_time;
            renderEffect.endTime = (NSInteger)filterParam.end_time;
            renderEffect.action = [NSString stringWithUTF8String:filterParam.action];
            
            QHVCEditEffectManager* effectMgr = [self.editor getEffectOfId:(NSInteger)filterParam.filter_id];
            QHVCEditEffect* effect = [effectMgr getEffect];
            if (effect.effectType == QHVCEditEffectTypeMix)
            {
                mixEffect = renderEffect;
            }
            else
            {
                [trackEffects addObject:renderEffect];
            }
        }
        
        QHVCEditRenderTrack* renderTrack = [[QHVCEditRenderTrack alloc] init];
        renderTrack.trackId = trackParam.track_id;
        renderTrack.mainClip = mainFrame;
        renderTrack.transitionClip = transitionFrame;
        renderTrack.effects = trackEffects;
        renderTrack.mixEffect = mixEffect;
        [tracks addObject:renderTrack];
    }
    
    //timeline effect
    NSMutableArray<QHVCEditRenderEffect *>* timelineEffects = [[NSMutableArray alloc] initWithCapacity:0];
    for (int k = 0; k < ve_multiTrack.multitrack_filters_len; k++)
    {
        ve_filter_param &filterParam = ve_multiTrack.multitrack_filters[k];
        QHVCEditRenderEffect* effect = [[QHVCEditRenderEffect alloc] init];
        effect.effectId = (NSInteger)filterParam.filter_id;
        effect.startTime = (NSInteger)filterParam.start_time;
        effect.endTime = (NSInteger)filterParam.end_time;
        effect.action = [NSString stringWithUTF8String:filterParam.action];
        [timelineEffects addObject:effect];
    }
    
    //renderParam
    self.currentTimestamp = timestampMs;
    QHVCEditRenderParam* renderParam = [[QHVCEditRenderParam alloc] init];
    renderParam.tracks = tracks;
    renderParam.effects = timelineEffects;
    renderParam.timestampMs = timestampMs;
    return renderParam;
}

- (void)frameDidStartProcessing:(NSInteger)timestampMs
{
    if (!self.firstFrameRendered)
    {
        self.firstFrameRendered = YES;
        if (self.delegate && [self.delegate respondsToSelector:@selector(onPlayerFirstFrameDidRendered)])
        {
            [self.delegate onPlayerFirstFrameDidRendered];
        }
    }
}

- (void)frameDidStopProcessing:(NSInteger)timestampMs
{
    if (self.isRefreshing)
    {
        self.isRefreshing = NO;
        QHVCEDIT_SAFE_BLOCK_IN_MAIN_QUEUE(self.refreshCompletion);
    }
    
    self.renderedCount ++;
}

#pragma mark - video_edit Methods

- (QHVCEditError)ve_createPlayer:(NSInteger)seekTimestamp
{
    //timeline handle检测
    __block HANDLE timelineHandle = [self.editor getTimelineHandle];
    if (!timelineHandle)
    {
        LogError(@"player create player error, get timeline error");
        return QHVCEditErrorInitPlayerError;
    }

    self.firstFrameRendered = NO;
    __block QHVCEditError err = QHVCEditErrorNoError;
    
    QHVCEDIT_WEAK_SELF
    dispatch_sync(self.playerQueue, ^{
        QHVCEDIT_STRONG_SELF
        
        //初始化播放器 handle
        self->_playerHandle = ve_player_create(timelineHandle, edit_player_frame_cb, edit_player_event_cb, (__bridge void *)(self));
        if (!self->_playerHandle)
        {
            LogError(@"player create player error, create player handle error");
            err = QHVCEditErrorInitPlayerError;
            
            if (self.delegate && [self.delegate respondsToSelector:@selector(onPlayerError:detail:)])
            {
                [self.delegate onPlayerError:err detail:@"player create player error, create player handle error"];
            }
        }
        else
        {
            //启动播放器
            ve_player_seek(self->_playerHandle, (int)seekTimestamp, 0);
            ve_player_start(self->_playerHandle);

            //更新播放器参数
            self.totalDuration = [self.editor timelineDuration];
            self.currentTimestamp = seekTimestamp;
        }
    });

    return err;
}

- (QHVCEditError)ve_freePlayer
{
    if (_playerHandle)
    {
        ve_player_free(_playerHandle);
        _playerHandle = NULL;
    }
    self.isPlayingState = NO;
    
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_playPlayer
{
    self.isPlayingState = YES;
    __block QHVCEditError error = QHVCEditErrorNoError;
    
    QHVCEDIT_WEAK_SELF
    dispatch_sync(self.playerQueue, ^{
        QHVCEDIT_STRONG_SELF
        ve_player_play(self->_playerHandle);
    });
    
    return error;
}

- (void)ve_stopPlayer
{
    self.isPlayingState = NO;
    if (_playerHandle)
    {
        QHVCEDIT_WEAK_SELF
        dispatch_sync(self.playerQueue, ^{
            QHVCEDIT_STRONG_SELF
            ve_player_pause(self->_playerHandle);
        });
    }
}

- (void)ve_refreshPlayer
{
    QHVCEDIT_WEAK_SELF
    dispatch_sync(self.playerQueue, ^{
        QHVCEDIT_STRONG_SELF
        HANDLE timelineHandle = [self.editor getTimelineHandle];
        ve_player_refresh_config(timelineHandle, self->_playerHandle);
    });
}

- (void)ve_playerSeekToTime:(NSInteger)timestamp forceRequest:(BOOL)forceRequest
{
    QHVCEDIT_WEAK_SELF
    dispatch_sync(self.playerQueue, ^{
        QHVCEDIT_STRONG_SELF
        ve_player_seek(self->_playerHandle, (int)timestamp, forceRequest ? 1:0);
    });
}

@end
