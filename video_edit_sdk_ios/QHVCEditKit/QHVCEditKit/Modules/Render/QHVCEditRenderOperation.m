//
//  QHVCEditRenderOperation.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/9.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditRenderOperation.h"
#import "QHVCEditMacroDefs.h"
#import "QHVCEditEffectProcesser.h"
#import "QHVCEditTwoInputEffectProcesser.h"

@interface QHVCEditRenderOperation ()
@property (nonatomic, retain) QHVCEditEditor* editor;
@property (nonatomic, assign) CGSize outputSize;
@property (nonatomic, retain) NSString* outputBgColor;
@property (nonatomic, retain) dispatch_queue_t processQueue;
@property (atomic,    retain) NSCondition *condition;
@property (nonatomic,   weak) id<QHVCEditRenderOperationDelegate> delegate;
@property (nonatomic, retain) id<QHVCEditCIOutputProtocol> output;
@property (nonatomic, retain) NSMutableDictionary* effectProcessDict;
@property (nonatomic, retain) QHVCEditEffectProcesser* timelineEffectProcesser;
@property (nonatomic, retain) QHVCEditTwoInputEffectProcesser* twoInputProcesser;

@end

@implementation QHVCEditRenderOperation

- (void)dealloc
{
    [self.effectProcessDict removeAllObjects];
    self.effectProcessDict = nil;
    self.twoInputProcesser = nil;
}

- (instancetype)initWithEditor:(QHVCEditEditor *)editor output:(id<QHVCEditCIOutputProtocol>)output
{
    if (!(self = [super init]))
    {
        return nil;
    }
    
    self.editor = editor;
    self.output = output;
    self.condition = [[NSCondition alloc] init];
    self.processQueue = dispatch_queue_create("QHVCEditRenderOperationQueue", NULL);
    self.effectProcessDict = [[NSMutableDictionary alloc] initWithCapacity:0];
    self.timelineEffectProcesser = [[QHVCEditEffectProcesser alloc] initWithEditor:self.editor];
    self.twoInputProcesser = [[QHVCEditTwoInputEffectProcesser alloc] initWithEditor:self.editor];
    
    return self;
}

- (void)setOutputSize:(CGSize)size color:(NSString *)color
{
    _outputSize = size;
    _outputBgColor = color;
}

- (void)setDelegate:(id<QHVCEditRenderOperationDelegate>)delegate
{
    _delegate = delegate;
}

- (void)processRenderParam:(QHVCEditRenderParam *)param
{
    [self.condition lock];
    //开始处理
    if (self.delegate && [self.delegate respondsToSelector:@selector(frameDidStartProcessing:)])
    {
        [self.delegate frameDidStartProcessing:param.timestampMs];
    }
    
    __block CIImage* renderedImg = nil;
    
    @autoreleasepool
    {
        for (QHVCEditRenderTrack* track in param.tracks)
        {
            //获取处理对象
            NSNumber* trackId = [NSNumber numberWithInteger:track.trackId];
            NSMutableDictionary<NSString*, QHVCEditEffectProcesser*>* trackProcesserDict = [self.effectProcessDict objectForKey:trackId];
            __block QHVCEditEffectProcesser* mainClipProcesser = nil;
            __block QHVCEditEffectProcesser* transitionFrameProcesser = nil;
            __block QHVCEditEffectProcesser* trackProcesser = nil;
            
            if (!trackProcesserDict)
            {
                mainClipProcesser = [[QHVCEditEffectProcesser alloc] initWithEditor:self.editor];
                transitionFrameProcesser = [[QHVCEditEffectProcesser alloc] initWithEditor:self.editor];
                trackProcesser = [[QHVCEditEffectProcesser alloc] initWithEditor:self.editor];
                
                trackProcesserDict = [[NSMutableDictionary alloc] initWithCapacity:0];
                [trackProcesserDict setObject:mainClipProcesser forKey:QHVCEDIT_DEFINE_MAIN_FRAME_PRCESSER];
                [trackProcesserDict setObject:transitionFrameProcesser forKey:QHVCEDIT_DEFINE_TRANSITION_FRAME_PRCESSER];
                [trackProcesserDict setObject:trackProcesser forKey:QHVCEDIT_DEFINE_TRANSITION_PRCESSER];
                [self.effectProcessDict setObject:trackProcesserDict forKey:trackId];
            }
            else
            {
                mainClipProcesser = [trackProcesserDict objectForKey:QHVCEDIT_DEFINE_MAIN_FRAME_PRCESSER];
                transitionFrameProcesser = [trackProcesserDict objectForKey:QHVCEDIT_DEFINE_TRANSITION_FRAME_PRCESSER];
                trackProcesser = [trackProcesserDict objectForKey:QHVCEDIT_DEFINE_TRANSITION_PRCESSER];
            }
            
            //处理main frame
            __block CIImage* mainImage = nil;
            __block CIImage* transitionImage = nil;
            __block CIImage* trackImage = nil;
            
            [mainClipProcesser processClipFrame:track.mainClip timestamp:param.timestampMs complete:^(CIImage *outputImage)
             {
                 [mainClipProcesser processEffect:outputImage effect:track.mainClip.effects timestamp:param.timestampMs complete:^(CIImage *outputImage)
                 {
                     mainImage = outputImage;
                 }];
            }];
            
            //处理transition frame
            if (track.transitionClip)
            {
                [transitionFrameProcesser processClipFrame:track.transitionClip timestamp:param.timestampMs complete:^(CIImage *outputImage)
                 {
                     [transitionFrameProcesser processEffect:outputImage effect:track.transitionClip.effects timestamp:param.timestampMs complete:^(CIImage *outputImage)
                      {
                          transitionImage = outputImage;
                      }];
                 }];
            }
            
            //处理转场效果
            if (transitionImage)
            {
                //转场效果
                CGFloat progress = (float)(param.timestampMs - track.transitionClip.transitionStartTime) / (track.transitionClip.transitionEndTime - track.transitionClip.transitionStartTime);
                trackImage = [self.twoInputProcesser processTransition:mainImage
                                                           secondFrame:transitionImage
                                                        transitionName:track.transitionClip.transitionName
                                                              progress:progress
                                                             timestamp:param.timestampMs
                                                    easingFunctionType:track.transitionClip.easingFunctionTyp];
            }
            else
            {
                trackImage = mainImage;
            }
            
            //处理track effect
            [transitionFrameProcesser processEffect:trackImage effect:track.effects timestamp:param.timestampMs complete:^(CIImage *outputImage)
             {
                 trackImage = outputImage;
            }];
            
            //处理track叠加效果
            renderedImg = [self.twoInputProcesser processMix:renderedImg
                                                    topImage:trackImage
                                                  outputSize:self.outputSize
                                             backgroundColor:self.outputBgColor
                                                   mixEffect:track.mixEffect
                                                   timestamp:param.timestampMs];
        }
    }
    
    //处理timeline effect
    QHVCEDIT_WEAK_SELF
    [self.timelineEffectProcesser processEffect:renderedImg effect:param.effects timestamp:param.timestampMs complete:^(CIImage *outputImage)
     {
        QHVCEDIT_STRONG_SELF
         if (self.output)
         {
             [self.output processImage:outputImage timestampMs:param.timestampMs userData:param.userData];
         }
         
         if (self.delegate && [self.delegate respondsToSelector:@selector(frameDidStopProcessing:)])
         {
             [self.delegate frameDidStopProcessing:param.timestampMs];
         }
    }];
    
    [self.condition unlock];
}


@end
