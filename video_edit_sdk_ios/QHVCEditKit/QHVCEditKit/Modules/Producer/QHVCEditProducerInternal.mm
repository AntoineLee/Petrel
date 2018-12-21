//
//  QHVCEditProducerInternal.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/6/29.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditProducerInternal.h"
#import "QHVCEditLogger.h"
#import "QHVCEditConfig.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditCommonDef.h"
#import "QHVCEditTrack.h"
#import "QHVCEditEffect.h"
#import "QHVCEditEffectManager.h"
#import "QHVCEditProducerOperation.h"
#import "QHVCEditEditorManager.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditEditor+Track.h"
#import "QHVCEditEditor+Clip.h"
#import "QHVCEditRenderParam.h"
#import "QHVCEditTimeline.h"
#import "QHVCEditTrackClip.h"
#import "QHVCEditTrackClipManager.h"
#import "ve_interface.h"

@interface QHVCEditProducerOnEncodeFrame : NSObject
@property (nonatomic, assign) void* data;
@property (nonatomic, assign) int len;
@property (nonatomic, assign) int width;
@property (nonatomic, assign) int height;
@property (nonatomic, assign) int timestampMs;
@property (nonatomic, assign) int frameIndex;

@end

@implementation QHVCEditProducerOnEncodeFrame
@end

void producer_data_cb(HANDLE producer_handle, ve_filter_callback_param* data, void* userExt)
{
    QHVCEditProducerInternal* producer = (__bridge QHVCEditProducerInternal *)userExt;
    [producer onProducerDataCB:producer_handle data:data];
}

void producer_event_cb(ve_export_status* status)
{
    if (status)
    {
        QHVCEditProducerInternal* producer = (__bridge QHVCEditProducerInternal *)status->userExt;
        [producer onProducerEventCB:status];
    }
}

@interface QHVCEditProducerInternal () <QHVCEditProducerOperationDelegate>
{
    HANDLE _producerHandle;
}

@property (nonatomic,   weak) id<QHVCEditProducerDelegate> delegate;
@property (nonatomic, retain) QHVCEditEditor* editor;

@property (atomic,    assign) NSInteger producerInputCount;
@property (atomic,    assign) NSInteger producerOutputCount;
@property (nonatomic, retain) dispatch_queue_t producerQueue;
@property (atomic,    retain) NSCondition *condition;
@property (atomic,    retain) NSMutableArray<QHVCEditProducerOnEncodeFrame*>* outputFrames;
@property (nonatomic, retain) QHVCEditProducerOperation* producerOperation1;
@property (nonatomic, retain) QHVCEditProducerOperation* producerOperation2;
@property (nonatomic, retain) QHVCEditProducerOperation* producerOperation3;

@end

@implementation QHVCEditProducerInternal

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline
{
    self = [super init];
    if (self)
    {
        if (!timeline)
        {
            LogError(@"producer init error, timeline is nil");
            return nil;
        }
        
        self.editor = [[QHVCEditEditorManager sharedInstance] getEditor:[timeline timelineId]];
        if (self.editor)
        {
            self.producerQueue = dispatch_queue_create("QHVCEditProducerQueue", NULL);
            self.condition = [[NSCondition alloc] init];
            self.outputFrames = [[NSMutableArray alloc] initWithCapacity:0];
            self.operationCount = 1;
        }
    }
    
    return self;
}

- (QHVCEditError)setProducerDelegate:(id<QHVCEditProducerDelegate>)delegate
{
    _delegate = delegate;
    return QHVCEditErrorNoError;
}

- (QHVCEditError)free
{
    [self.producerOperation1 free];
    self.producerOperation1 = nil;
    
    [self.producerOperation2 free];
    self.producerOperation2 = nil;
    
    [self.producerOperation3 free];
    self.producerOperation3 = nil;
    
    QHVCEditError err = [self ve_freeProducerHandle];
    return err;
}

- (QHVCEditError)start
{
    //释放已有的
    [self stop];
    
    //创建producer句柄
    QHVCEditError err = [self ve_createProducerHandle];
    if (err != QHVCEditErrorNoError)
    {
        return err;
    }
    
    //配置参数
    self.producerInputCount = 0;
    self.producerOutputCount = 0;
    
    self.producerOperation1 = [[QHVCEditProducerOperation alloc] initWithEditor:self.editor];
    [self.producerOperation1 setDelegate:self];
    
    self.producerOperation2 = [[QHVCEditProducerOperation alloc] initWithEditor:self.editor];
    [self.producerOperation2 setDelegate:self];
    
    self.producerOperation3 = [[QHVCEditProducerOperation alloc] initWithEditor:self.editor];
    [self.producerOperation3 setDelegate:self];
    
    err = [self ve_startProducer];
    return err;
}

- (QHVCEditError)stop
{
    QHVCEditError err = [self ve_stopProducer];
    return err;
}

- (void)onProducerEventCB:(void *)status
{
    ve_export_status* value = (ve_export_status *)status;
    if (value->status == VE_EXPORT_ERR)
    {
        NSString* info = [[QHVCEditConfig sharedInstance] getErrorCodeInfo:value->status];
        LogError(@"on producer event, error %d, progress %f, %@", value->err_no, value->progress, info);
        
        if (self.delegate && [self.delegate respondsToSelector:@selector(onProducerError:)])
        {
            [self.delegate onProducerError:QHVCEditErrorProducingError];
        }
        return;
    }
    
    if (value->status == VE_EXPORT_CANCEL)
    {
        LogInfo(@"on producer event, cancel, progress = %f", value->progress);
        if (self.delegate && [self.delegate respondsToSelector:@selector(onProducerInterrupt)])
        {
            [self.delegate onProducerInterrupt];
        }
        return;
    }
    
    if (value->status == VE_EXPORT_END)
    {
        LogInfo(@"on producer event, complete");
        if (self.delegate && [self.delegate respondsToSelector:@selector(onProducerComplete)])
        {
            [self.delegate onProducerComplete];
        }
        return;
    }
    
    if (value->status == VE_EXPORT_PROCESSING)
    {
        LogInfo(@"on producer event, progress = %f", value->progress);
        if (self.delegate && [self.delegate respondsToSelector:@selector(onProducerProgress:)])
        {
            [self.delegate onProducerProgress:value->progress];
        }
        return;
    }
}

- (void)onProducerDataCB:(void *)producerHandle data:(void *)data
{
    @autoreleasepool
    {
        self.producerInputCount ++;
        QHVCEditProducerOperation* producerOperation = nil;
        if ((self.producerOperation1.inputCount - self.producerOperation1.outputCount < 1) && self.operationCount > 2)
        {
            //使用operation1处理
            LogDebug(@"producer test, process1, input = %ld, output = %ld", (long)self.producerOperation1.inputCount, (long)self.producerOperation1.outputCount);
            producerOperation = self.producerOperation1;
        }
        else if ((self.producerOperation2.inputCount - self.producerOperation2.outputCount < 1) && self.operationCount > 1)
        {
            //使用operation2处理
            LogDebug(@"producer test, process2, input = %ld, output = %ld", (long)self.producerOperation2.inputCount, (long)self.producerOperation2.outputCount);
            producerOperation = self.producerOperation2;
        }
        else
        {
            //使用operation3处理
            LogDebug(@"producer test, process3, input = %ld, output = %ld", (long)self.producerOperation3.inputCount, (long)self.producerOperation3.outputCount);
            producerOperation = self.producerOperation3;
        }
        
        QHVCEditRenderParam* renderParam = [self resolveRenderParams:data];
        LogDebug(@"on producer frame, timestamp [%ld], [%ld]track [%ld]timeline effects",
                 (long)renderParam.timestampMs,
                 (long)[renderParam.tracks count],
                 (long)[renderParam.effects count]);

        [producerOperation producerParam:renderParam];
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
            CVPixelBufferRelease((CVPixelBufferRef)frameParam.data);
            if (transitionParam.len > 0 && transitionParam.transition_frame == 1)
            {
                CVPixelBufferRelease((CVPixelBufferRef)transitionParam.data);
            }
            LogWarn(@"producer resolve render params, track not exist");
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
            LogWarn(@"producer resolve render params, clip not exist");
            CVPixelBufferRelease((CVPixelBufferRef)frameParam.data);
            if (transitionParam.len > 0 && transitionParam.transition_frame == 1)
            {
                CVPixelBufferRelease((CVPixelBufferRef)transitionParam.data);
            }
            continue;
        }
        
        //mainFrame
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
        if (transitionParam.len > 0 && transitionParam.transition_frame == 1)
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
                LogWarn(@"producer resolve render params, transition clip not exist");
                CVPixelBufferRelease((CVPixelBufferRef)transitionParam.data);
                continue;
            }
            QHVCEditTrackClipManager* clipMgr = [self.editor clipGetManager:transitionClip];
            
            //transition frame
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
        renderTrack.mainClip =  mainFrame;
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
    QHVCEditRenderParam* renderParam = [[QHVCEditRenderParam alloc] init];
    renderParam.tracks = tracks;
    renderParam.effects = timelineEffects;
    renderParam.timestampMs = timestampMs;
    renderParam.userData = [NSNumber numberWithLong:self.producerInputCount];
    return renderParam;
}

- (void)onFrameCallback:(void *)data dataLen:(int)len width:(int)width height:(int)height timestamp:(NSInteger)timestamp userData:(id)userData
{
    [self.condition lock];
    self.producerOutputCount ++;
    
    
    if (self.operationCount > 1)
    {
        [self sortFramesAndSendBack:data dataLen:len width:width height:height timestamp:timestamp userData:userData];
    }
    else
    {
        [self ve_sendDataBack:data len:len width:width height:height timestamp:timestamp];
    }
    
    [self.condition unlock];
}

- (void)sortFramesAndSendBack:(void *)data dataLen:(int)len width:(int)width height:(int)height timestamp:(NSInteger)timestamp userData:(id)userData
{
    //多渲染做数据排序处理
    QHVCEditProducerOnEncodeFrame* frame = [[QHVCEditProducerOnEncodeFrame alloc] init];
    frame.data = data;
    frame.len = len;
    frame.width = width;
    frame.height = height;
    frame.timestampMs = (int)timestamp;
    frame.frameIndex = [(NSNumber *)userData intValue];
    
    //按时间戳排序
    if ([self.outputFrames count] == 0)
    {
        [self.outputFrames addObject:frame];
    }
    else
    {
        __block NSInteger index = -1;
        [self.outputFrames enumerateObjectsUsingBlock:^(QHVCEditProducerOnEncodeFrame * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
         {
             if (obj.timestampMs < frame.timestampMs)
             {
                 index = idx;
             }
             else if (obj.timestampMs == frame.timestampMs)
             {
                 if (obj.frameIndex > frame.frameIndex)
                 {
                     index = idx - 1;
                 }
                 else
                 {
                     index = idx;
                 }
             }
         }];
        
        index = index + 1;
        if (index < [self.outputFrames count])
        {
            [self.outputFrames insertObject:frame atIndex:index];
        }
        else
        {
            [self.outputFrames addObject:frame];
        }
    }
    
    int maxIndex = [self.outputFrames lastObject].frameIndex;
    LogDebug(@"on encode frame,\n"
            "width = %d,\n"
            "height = %d,\n"
            "timstamp = %ld,\n"
            "maxIndex = %d,\n"
            "frameIndex = %d,\n"
            "producerInputCount = %ld,\n"
            "producerOutputCount = %ld,\n"
            "outputFramesCount = %ld\n", width, height, (long)timestamp, maxIndex, frame.frameIndex, (long)self.producerInputCount, (long)self.producerOutputCount, (long)[self.outputFrames count]);
    
    if (maxIndex == self.producerInputCount && self.producerInputCount == self.producerOutputCount)
    {
        for (int i = 0; i < [self.outputFrames count]; i++)
        {
            QHVCEditProducerOnEncodeFrame* obj = self.outputFrames[i];
            [self ve_sendDataBack:obj.data len:obj.len width:obj.width height:obj.height timestamp:obj.timestampMs];
        }
        
        [self.outputFrames removeAllObjects];
    }
}

#pragma mark - video_edit Methods

- (HANDLE)ve_getTimelineHandle
{
    HANDLE handle = [self.editor getTimelineHandle];
    return handle;
}

- (QHVCEditError)ve_createProducerHandle
{
    __block QHVCEditError error = QHVCEditErrorNoError;
    
    QHVCEDIT_WEAK_SELF
    dispatch_sync(self.producerQueue, ^{
        QHVCEDIT_STRONG_SELF
        HANDLE timelineHandle = [self ve_getTimelineHandle];
        if (timelineHandle)
        {
            ve_export_param exportParam;
            exportParam.userExt = (__bridge void *)self;
            self->_producerHandle = ve_export_create(timelineHandle, producer_data_cb, producer_event_cb, exportParam);
            if (!self->_producerHandle)
            {
                error = QHVCEditErrorProducerHandleIsNull;
            }
        }
        else
        {
            error = QHVCEditErrorInitProducerError;
        }
    });
    
    return error;
}

- (QHVCEditError)ve_freeProducerHandle
{
    if (_producerHandle)
    {
        QHVCEDIT_WEAK_SELF
        dispatch_sync(self.producerQueue, ^{
            QHVCEDIT_STRONG_SELF
            ve_export_free(self->_producerHandle);
            self->_producerHandle = nil;
        });
    }
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_startProducer
{
    if (!_producerHandle)
    {
        LogError(@"start producer error, handle is nil");
        return QHVCEditErrorProducerHandleIsNull;
    }
    
    QHVCEDIT_WEAK_SELF
    dispatch_sync(self.producerQueue, ^{
        QHVCEDIT_STRONG_SELF
        ve_export_start(self->_producerHandle);
    });
    return QHVCEditErrorNoError;
}

- (QHVCEditError)ve_stopProducer
{
    if (!_producerHandle)
    {
        LogError(@"stop producer error, handle is nil");
        return QHVCEditErrorProducerHandleIsNull;
    }
    
    QHVCEDIT_WEAK_SELF
    dispatch_async(self.producerQueue, ^{
        QHVCEDIT_STRONG_SELF
        ve_export_cancel(self->_producerHandle);
    });
    
    return QHVCEditErrorNoError;
}

- (void)ve_sendDataBack:(void *)data len:(int)len width:(int)width height:(int)height timestamp:(NSInteger)timestampMs
{
    ve_filtered_data dataParam;
    dataParam.data = data;
    dataParam.len = len;
    dataParam.width = width;
    dataParam.height = height;
    dataParam.cur_time = (int)timestampMs;
    
    LogDebug(@"producer send data to encoder,\n"
            "input = %ld,\n"
            "output = %ld,\n"
            "width = %d,\n"
            "height = %d,\n"
            "len = %d,\n"
            "timestamp = %d\n",
            (long)self.producerInputCount,
            (long)self.producerOutputCount,
            width,
            height,
            len,
            (int)timestampMs);
    
    ve_export_send_filtered_data_back(_producerHandle, &dataParam);
}

@end
