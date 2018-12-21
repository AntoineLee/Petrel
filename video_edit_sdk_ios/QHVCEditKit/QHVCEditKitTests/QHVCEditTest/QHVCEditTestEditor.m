//
//  QHVCEditTestEditor.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/5.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEditTestEditor.h"
#import "QHVCEditTestMacroDefs.h"
#import <QHVCEffectKit/QHVCEffect.h>
#import <QHVCEffectKit/QHVCEffectBase+Process.h>
#import "QHVCEditCommonDef.h"
#import "QHVCEditTrackManager.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Clip.h"
#import "QHVCEditEditorManager.h"
#import "QHVCEditTrackClipManager.h"

int QHVCEditTestEditorAll(void)
{
    [QHVCEditTools setSDKLogLevel:QHVCEditLogLevelDebug];
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTimelineAll());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTrackAll());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorEffectAll());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorClipAll());
    return 0;
}

#pragma mark - Timeline

QHVCEditTimeline* QHVCEditTestEditorCreateTimeline()
{
    QHVCEditTimeline* timeline = [[QHVCEditTimeline alloc] initTimeline];
    [timeline setOutputWidth:1280 height:720];
    [timeline setOutputBgColor:@"FF000000"];
    [timeline setOutputFps:30];
    [timeline setOutputBitrate:4.5*1000*1000];
    [timeline setSpeed:1.0];
    [timeline setVolume:100];
    [timeline setOutputPath:[NSTemporaryDirectory() stringByAppendingString:@"output.mp4"]];
    
    return timeline;
}

void QHVCEditTestEditorCreateTimelineWithErrorParam()
{
    QHVCEditTimeline* timeline = [[QHVCEditTimeline alloc] initTimeline];
    [timeline setOutputWidth:0 height:0];
    [timeline setOutputWidth:100 height:0];
    [timeline setOutputFps:0];
    [timeline setOutputBitrate:0];
    [timeline setSpeed:0];
    [timeline setOutputPath:nil];
    
    QHVCEditTrack* track = QHVCEditTestEditorCreateSequenceTrack(timeline);
    [timeline appendTrack:track];
    [timeline appendTrack:track];
    [timeline appendTrack:nil];
    [timeline deleteTrackById:10];
    
    QHVCEditEffect* effect = QHVCEditTestEditorCreateEffect(timeline);
    [timeline addEffect:effect];
    [timeline addEffect:effect];
    [timeline addEffect:nil];
    
    [effect setEndTime:0];
    [timeline updateEffect:effect];
    [timeline deleteEffectById:effect.effectId];
    [timeline updateEffect:effect];
    [timeline updateEffect:nil];
    
    [timeline free];
    [timeline setOutputWidth:100 height:100];
    [timeline setOutputFps:30];
    [timeline setOutputBitrate:10];
    [timeline setSpeed:1];
    [timeline setOutputPath:@"test.mp4"];
    [timeline setVolume:100];
    
    QHVCEditTrack* track1 = QHVCEditTestEditorCreateSequenceTrack(timeline);
    [timeline appendTrack:track1];
    
    [timeline deleteTrackById:track.trackId];
    [timeline duration];
    
    QHVCEditEffect* effect1 = [[QHVCEditEffect alloc] initEffectWithTimeline:timeline];
    [timeline addEffect:effect1];
    [timeline updateEffect:effect];
    [timeline deleteEffectById:effect.effectId];
}

int QHVCEditTestEditorFreeTimeline(QHVCEditTimeline* timeline)
{
    [timeline free];
    return 0;
}

int QHVCEditTestEditorTimelineParam(QHVCEditTimeline* timeline)
{
    QHVCEditObjectType objType = [timeline objType];
    NSInteger timelindId = [timeline timelineId];
    NSInteger duration = [timeline duration];
    CGSize outputSize = [timeline outputSize];
    NSString* bgColor = [timeline outputBgColor];
    NSInteger fps = [timeline outputFps];
    NSInteger bps = [timeline outputBitrate];
    CGFloat speed = [timeline speed];
    NSInteger volume = [timeline volume];
    NSString* outputPath = [timeline outputPath];
    
    NSString* userData = @"timeline";
    [timeline setUserData:(__bridge void *)(userData)];
    NSString* checkData = [timeline userData];
    if (![userData isEqualToString:checkData])
    {
        return -1;
    }
    
    return 0;
}

int QHVCEditTestEditorTimelineWithTrack(QHVCEditTimeline* timeline)
{
    QHVCEditSequenceTrack* track = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(track);
    
    [timeline appendTrack:track];
    
    QHVCEditTrack* testTrack = [timeline getTrackById:track.trackId];
    if (testTrack != track)
    {
        return -1;
    }
    
    NSArray* tracks = [timeline getTracks];
    if ([tracks count] <= 0)
    {
        return -1;
    }
    
    [timeline deleteTrackById:track.trackId];
    
    QHVCEditSequenceTrack* audioTrack = QHVCEditTestEditorCreateAudioTrack(timeline);
    [timeline appendTrack:audioTrack];
    [timeline deleteTrackById:audioTrack.trackId];
    
    return 0;
}

int QHVCEditTestEditorTimelineWithEffect(QHVCEditTimeline* timeline)
{
    QHVCEditEffect* effect = QHVCEditTestEditorCreateEffect(timeline);
    QHVCEDIT_TEST_OBJECT(effect);
    
    [timeline addEffect:effect];
    [timeline updateEffect:effect];
    
    QHVCEditEffect* check = [timeline getEffectById:effect.effectId];
    if (check != effect)
    {
        return -1;
    }
    
    NSArray* effects = [timeline getEffects];
    if ([effects count] <= 0)
    {
        return -1;
    }
    
    [timeline deleteEffectById:effect.effectId];
    effect = nil;
    
    return 0;
}

int QHVCEditTestEditorTimelineAll()
{
    //create with error params
    QHVCEditTestEditorCreateTimelineWithErrorParam();
    
    //create
    QHVCEditTimeline* timeline = QHVCEditTestEditorCreateTimeline();
    QHVCEDIT_TEST_OBJECT(timeline);
    
    //params
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTimelineParam(timeline));
    
    //with track
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTimelineWithTrack(timeline));
    
    //with effect
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTimelineWithEffect(timeline));
    
    //free
    QHVCEditTestEditorFreeTimeline(timeline);
    
    return 0;
}

#pragma mark - Track

QHVCEditSequenceTrack* QHVCEditTestEditorCreateSequenceTrack(QHVCEditTimeline* timeline)
{
    QHVCEditSequenceTrack* track = [[QHVCEditSequenceTrack alloc] initWithTimeline:timeline type:QHVCEditTrackTypeVideo];
    [track setZOrder:0];
    [track setSpeed:1.0];
    [track setVolume:100];
    
    CGSize outputSize = [timeline outputSize];
    [track setRenderX:0 renderY:0 renderWidth:outputSize.width renderHeight:outputSize.height];
    [track setRenderRadian:0];
    [track setFillMode:QHVCEditFillModeAspectFit];
    
    QHVCEditBgParams* bgParam = [[QHVCEditBgParams alloc] init];
    [bgParam setMode:QHVCEditBgModeColor];
    [bgParam setBgInfo:@"FF00000000"];
    [track setBgParams:bgParam];
    
    return track;
}

QHVCEditOverlayTrack* QHVCEditTestEditorCreateOverlayTrack(QHVCEditTimeline* timeline)
{
    QHVCEditOverlayTrack* track = [[QHVCEditOverlayTrack alloc] initWithTimeline:timeline type:QHVCEditTrackTypeVideo];
    return track;
}

QHVCEditSequenceTrack* QHVCEditTestEditorCreateAudioTrack(QHVCEditTimeline* timeline)
{
    QHVCEditSequenceTrack* track = [[QHVCEditSequenceTrack alloc] initWithTimeline:timeline type:QHVCEditTrackTypeAudio];
    return track;
}

int QHVCEditTestEditorCreateTrackWithErrorParams(QHVCEditTimeline* timeline)
{
    QHVCEditTrack* track = [[QHVCEditTrack alloc] initWithTimeline:timeline type:QHVCEditTrackTypeVideo];
    [timeline appendTrack:track];
    [track trackArrangement];
    [track setSpeed:-1];
    [track setVolume:-1];
    [track setRenderX:0 renderY:0 renderWidth:0 renderHeight:0];
    [track setRenderX:0 renderY:0 renderWidth:100 renderHeight:0];
    
    return 0;
}

int QHVCEditTestEditorTrackWithClip(QHVCEditTimeline* timeline)
{
    QHVCEditSequenceTrack* sequenceTrack = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(sequenceTrack);
    
    QHVCEditTrackClip* clip0 = QHVCEditTestEditorCreateVideoClip(timeline);
    [sequenceTrack appendClip:nil];
    [sequenceTrack appendClip:clip0];
    [sequenceTrack insertClip:nil atIndex:0];
    [sequenceTrack insertClip:clip0 atIndex:0];
    [sequenceTrack updateClipParams:clip0];
    [sequenceTrack deleteClipById:clip0.clipId];
    [sequenceTrack deleteClipAtIndex:0];
    [sequenceTrack getClipById:clip0.clipId];
    [sequenceTrack getClips];
    [sequenceTrack moveClip:0 toIndex:0];
    [sequenceTrack getClipAtIndex:0];
    [sequenceTrack addVideoTransitionToIndex:1 duration:500 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    
    [timeline appendTrack:sequenceTrack];
    
    [sequenceTrack getClipAtIndex:10];
    [sequenceTrack deleteClipAtIndex:10];
    [sequenceTrack updateClipParams:clip0];
    [sequenceTrack deleteClipById:clip0.clipId];
    [sequenceTrack getClipById:clip0.clipId];
    
    QHVCEditTrackClip* clip1 = QHVCEditTestEditorCreateVideoClip(timeline);
    [sequenceTrack appendClip:clip1];
    [sequenceTrack appendClip:clip1];
    
    QHVCEditTrackClip* clip2 = QHVCEditTestEditorCreateVideoClip(timeline);
    [sequenceTrack insertClip:clip2 atIndex:1];
    [sequenceTrack insertClip:clip2 atIndex:1];
    [sequenceTrack duration];
    
    QHVCEditTrackClip* clip3 = QHVCEditTestEditorCreateVideoClip(timeline);
    [sequenceTrack insertClip:clip3 atIndex:1];
    [sequenceTrack updateClipParams:nil];
    
    [sequenceTrack addVideoTransitionToIndex:0 duration:500 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack addVideoTransitionToIndex:1 duration:0 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack addVideoTransitionToIndex:1 duration:500 videoTransitionName:@"" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    
    [sequenceTrack updateVideoTransitionAtIndex:1 duration:1000 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack updateVideoTransitionAtIndex:1 duration:0 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack updateVideoTransitionAtIndex:1 duration:500 videoTransitionName:@"" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack deleteVideoTransition:10];
    
    
    [sequenceTrack addVideoTransitionToIndex:1 duration:500 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack updateVideoTransitionAtIndex:1 duration:1000 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack updateVideoTransitionAtIndex:1 duration:5000 videoTransitionName:@"LinearBlur" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack updateVideoTransitionAtIndex:1 duration:500 videoTransitionName:@"" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [sequenceTrack deleteVideoTransition:1];
    
    [sequenceTrack updateClipParams:clip1];
    [sequenceTrack deleteEffectById:clip3.clipId];
    [sequenceTrack getClipById:clip1.clipId];
    [sequenceTrack getClips];
    
    [sequenceTrack moveClip:0 toIndex:1];
    [sequenceTrack deleteClipAtIndex:0];
    [sequenceTrack deleteClipAtIndex:10];
    [sequenceTrack getClipAtIndex:0];
    [sequenceTrack deleteClipById:clip1.clipId];
    
    QHVCEditOverlayTrack* overlayTrack = QHVCEditTestEditorCreateOverlayTrack(timeline);
    QHVCEDIT_TEST_OBJECT(overlayTrack);
    
    QHVCEditTrackClip* clip4 = QHVCEditTestEditorCreateVideoClip(timeline);
    [overlayTrack changeClipInsertTime:0 clipId:clip4.clipId];
    
    [timeline appendTrack:overlayTrack];
    [overlayTrack trackArrangement];
    
    [overlayTrack changeClipInsertTime:0 clipId:clip4.clipId];
    [overlayTrack addClip:nil atTime:0];
    [overlayTrack addClip:clip4 atTime:-100];
    [overlayTrack addClip:clip4 atTime:0];
    [overlayTrack addClip:clip4 atTime:0];
    [overlayTrack changeClipInsertTime:-1000 clipId:clip4.clipId];
    [overlayTrack changeClipInsertTime:100 clipId:clip4.clipId];
    
    QHVCEditTrackClip* clip5 = QHVCEditTestEditorCreateVideoClip(timeline);
    [overlayTrack addClip:clip5 atTime:10000];
    
    QHVCEditTrackClip* clip6 = QHVCEditTestEditorCreateVideoClip(timeline);
    [overlayTrack addClip:clip6 atTime:5000];
    
    QHVCEditTrackClip* clip7 = QHVCEditTestEditorCreateVideoClip(timeline);
    [overlayTrack addClip:clip7 atTime:5000];
    
    QHVCEditTrackClip* clip8 = QHVCEditTestEditorCreateImageClip(timeline);
    QHVCEditSequenceTrack* audioTrack = QHVCEditTestEditorCreateAudioTrack(timeline);
    [timeline appendTrack:audioTrack];
    [audioTrack appendClip:clip8];
    [audioTrack duration];
    
    QHVCEditTrackClip* audioClip = QHVCEditTestEditorCreateAudioClip(timeline);
    [audioTrack appendClip:audioClip];
    
    return 0;
}

int QHVCEditTestEditorTrackParam(QHVCEditTrack* track)
{
    QHVCEditObjectType objType = [track objType];
    QHVCEditTrackType trackTyp = [track trackType];
    QHVCEditTrackArrangement arrangement = [track trackArrangement];
    NSInteger trackId = [track trackId];
    NSInteger duration = [track duration];
    QHVCEditObject* superObj = [track superObj];
    NSString* userData = @"track";
    [track setUserData:(__bridge void *)(userData)];
    NSString* checkData = [track userData];
    if (![userData isEqualToString:checkData])
    {
        return -1;
    }
    
    NSInteger zOrder = [track zOrder];
    CGFloat speed = [track speed];
    NSInteger volume = [track volume];
    CGRect renderRect = [track renderRect];
    CGFloat renderRadian = [track renderRadian];
    QHVCEditFillMode fillMode = [track fillMode];
    QHVCEditBgParams* bgParam = [track bgParams];
    
    return 0;
}

int QHVCEditTestEditorTrackWithEffect(QHVCEditTimeline* timeline)
{
    QHVCEditSequenceTrack* track = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(track);
    
    QHVCEditEffect* effect = QHVCEditTestEditorCreateEffect(timeline);
    [track addEffect:effect];
    [track updateEffect:effect];
    [track deleteEffectById:effect.effectId];
    [track getEffectById:effect.effectId];
    [track getEffects];
    [timeline appendTrack:track];
    
    [track addEffect:nil];
    [track addEffect:effect];
    [track addEffect:effect];
    [track updateEffect:nil];
    [track updateEffect:effect];
    [track getEffectById:effect.effectId];
    [track getEffects];
    [track deleteEffectById:effect.effectId];
    
    QHVCEditEffect* effectError = QHVCEditTestEditorCreateEffect(timeline);
    [track updateEffect:effectError];
    
    QHVCEditAudioTransferEffect* audioTransfer = [[QHVCEditAudioTransferEffect alloc] initEffectWithTimeline:timeline];
    [track addEffect:audioTransfer];
    
    return 0;
}

int QHVCEditTestEditorTrackOther()
{
    QHVCEditTimeline* timeline = QHVCEditTestEditorCreateTimeline();
    QHVCEDIT_TEST_OBJECT(timeline);
    
    QHVCEditOverlayTrack* track = QHVCEditTestEditorCreateOverlayTrack(timeline);
    QHVCEDIT_TEST_OBJECT(track);
    [timeline appendTrack:track];
    
    QHVCEditTrackClip* clip = QHVCEditTestEditorCreateVideoClip(timeline);
    QHVCEDIT_TEST_OBJECT(clip);
    [track addClip:clip atTime:0];
    
    QHVCEditTrackManager* trackMgr = [[QHVCEditTrackManager alloc] initWithTrack:track timelineHandle:nil];
    [track changeClipInsertTime:100 clipId:clip.clipId];
    [trackMgr duration];
    [trackMgr changeClipInsertTime:500 clipId:20];
    [trackMgr setVolume:0];
    [trackMgr moveClip:clip.clipId toIndex:0];
    [trackMgr updateClipParams:nil];
    [trackMgr deleteClipById:0];
    [trackMgr deleteClipAtIndex:0];
    [trackMgr addVideoTransitionToIndex:1 duration:500 videoTransitionName:@"error" transitionId:0 easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [trackMgr updateVideoTransitionAtIndex:0 duration:500 videoTransitionName:@"error" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [trackMgr deleteVideoTransition:1];
    
    QHVCEditEditor* editor = [[QHVCEditEditorManager sharedInstance] getEditor:timeline.timelineId];
    QHVCEDIT_TEST_OBJECT(editor);

    QHVCEditSequenceTrack* track2 = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(track2);
    [timeline appendTrack:track2];

    QHVCEditTrackClip* clip1 = QHVCEditTestEditorCreateVideoClip(timeline);
    QHVCEDIT_TEST_OBJECT(clip1);

    QHVCEditTrackClip* clip2 = QHVCEditTestEditorCreateVideoClip(timeline);
    QHVCEDIT_TEST_OBJECT(clip2);
    [track2 appendClip:clip1];
    [track2 appendClip:clip2];

    QHVCEditTrackManager* trackMgr2 = [editor.trackMgrs objectForKey:@(track2.trackId)];
    QHVCEditTrackClipManager* clip2Mgr = [editor clipGetManager:clip2];
    [clip2Mgr setHaveTransition:YES];
    [trackMgr2 updateVideoTransitionAtIndex:1 duration:50000 videoTransitionName:@"error" easingFunctionType:QHVCEditEasingFunctionTypeLinear];
    [trackMgr2 deleteVideoTransition:1];
    
    QHVCEditEffect* effect = QHVCEditTestEditorCreateEffect(timeline);
    [trackMgr2 addEffect:effect];
    
    [effect setEndTime:0];
    [trackMgr2 updateEffect:effect];
    
    return 0;
}

int QHVCEditTestEditorTrackAll()
{
    QHVCEditTimeline* timeline = QHVCEditTestEditorCreateTimeline();
    QHVCEDIT_TEST_OBJECT(timeline);
    
    //create track with error params
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorCreateTrackWithErrorParams(timeline));
    
    //create track with error timeline
    QHVCEditSequenceTrack* sequenceTrack = QHVCEditTestEditorCreateSequenceTrack(nil);
    
    //create track with true timeline
    sequenceTrack = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(sequenceTrack);
    
    //track params
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTrackParam(sequenceTrack));
    
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTrackWithClip(timeline));
    
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTrackWithEffect(timeline));
    
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorTrackOther());
    
    [timeline free];
    
    return 0;
}

#pragma mark - Effect

QHVCEditEffect* QHVCEditTestEditorCreateEffect(QHVCEditTimeline* timeline)
{
    QHVCEditEffect* effect = [[QHVCEditEffect alloc] initEffectWithTimeline:timeline];
    [effect setStartTime:0];
    [effect setEndTime:1000];
    return effect;
}

int QHVCEditTestEditorEffectParams(QHVCEditEffect* effect)
{
    QHVCEditObject* superObj = [effect superObj];
    NSString* userData = @"effect";
    [effect setUserData:(__bridge void *)(userData)];
    NSString* checkData = [effect userData];
    if (![userData isEqualToString:checkData])
    {
        return -1;
    }
    
    return 0;
}

int QHVCEditTestEditorEffectFilter(QHVCEditTimeline* timeline)
{
    QHVCEditFilterEffect* filter = [[QHVCEditFilterEffect alloc] initEffectWithTimeline:timeline];
    QHVCEDIT_TEST_OBJECT(filter);
    
    NSString* lutPath = [[NSBundle mainBundle] pathForResource:@"test_lut" ofType:@"png"];
    [filter setFilePath:nil];
    [filter setFilePath:lutPath];
    [filter setIntensity:0.5];
    
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* ciImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEDIT_TEST_OBJECT(ciImage);
    
    CIImage* outImage = [filter processImage:ciImage timestamp:0];
    QHVCEDIT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEditTestEditorEffectSticker(QHVCEditTimeline* timeline)
{
    QHVCEditStickerEffect* sticker = [[QHVCEditStickerEffect alloc] initEffectWithTimeline:timeline];
    QHVCEDIT_TEST_OBJECT(sticker);
    
    UIImage* stickerImage = [UIImage imageNamed:@"test_sticker.png"];
    NSString* stickerPath = [[NSBundle mainBundle] pathForResource:@"test_sticker" ofType:@"png"];
    [sticker setSticker:stickerImage];
    [sticker setStickerPath:nil];
    [sticker setStickerPath:stickerPath];
    [sticker setRenderX:0];
    [sticker setRenderY:0];
    [sticker setRenderWidth:100];
    [sticker setRenderHeight:100];
    [sticker setRenderRadian:0];
    [sticker setVideoTransfer:nil];
    
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* ciImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEDIT_TEST_OBJECT(ciImage);
    
    CIImage* outImage = [sticker processImage:ciImage timestamp:0];
    QHVCEDIT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEditTestEditorEffectVideoTransfer(QHVCEditTimeline* timeline)
{
    QHVCEditVideoTransferEffect* videoTransfer = [[QHVCEditVideoTransferEffect alloc] initEffectWithTimeline:timeline];
    QHVCEDIT_TEST_OBJECT(videoTransfer);
    
    [videoTransfer setVideoTransfer:nil];
    
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* ciImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEDIT_TEST_OBJECT(ciImage);
    
    CIImage* outImage = [videoTransfer processImage:ciImage timestamp:0];
    QHVCEDIT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEditTestEditorEffectMix(QHVCEditTimeline* timeline)
{
    QHVCEditMixEffect* mix = [[QHVCEditMixEffect alloc] initEffectWithTimeline:timeline];
    QHVCEDIT_TEST_OBJECT(mix);
    
    [mix effectType];
    [mix setIntensity:0.5];
    
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* ciImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEDIT_TEST_OBJECT(ciImage);
    
    CIImage* outImage = [mix processImage:ciImage timestamp:0];
    [timeline addEffect:mix];
    [mix processImage:ciImage timestamp:0];
    [timeline deleteEffectById:mix.effectId];
    
    QHVCEditSequenceTrack* track = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(track);
    [timeline appendTrack:track];
    [track addEffect:mix];
    
    outImage = [mix processImage:ciImage timestamp:0];
    QHVCEDIT_TEST_OBJECT(outImage);
    
    [track setBgParams:nil];
    outImage = [mix processImage:ciImage timestamp:0];
    QHVCEDIT_TEST_OBJECT(outImage);
    
    return 0;
}

int QHVCEditTestEditorEffectAudioTransfer(QHVCEditTimeline* timeline)
{
    QHVCEditAudioTransferEffect* audioTransfer = [[QHVCEditAudioTransferEffect alloc] initEffectWithTimeline:timeline];
    QHVCEDIT_TEST_OBJECT(audioTransfer);
    
    [audioTransfer effectType];
    return 0;
}

int QHVCEditTestEditorEffectAll(void)
{
    //create timeline
    QHVCEditTimeline* timeline = QHVCEditTestEditorCreateTimeline();
    QHVCEDIT_TEST_OBJECT(timeline);
    
    //create with nil timeline
    QHVCEditEffect* effect = QHVCEditTestEditorCreateEffect(nil);
    
    //create with true timeline
    effect = QHVCEditTestEditorCreateEffect(timeline);
    QHVCEDIT_TEST_OBJECT(effect);
    
    //params
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorEffectParams(effect));
    
    //process image
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* ciImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEDIT_TEST_OBJECT(ciImage);
    [effect processImage:ciImage timestamp:0];
    
    //effects
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorEffectFilter(timeline));
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorEffectSticker(timeline));
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorEffectVideoTransfer(timeline));
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorEffectMix(timeline));
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorEffectAudioTransfer(timeline));
    
    [timeline free];
    effect = nil;
    
    return 0;
}

#pragma mark - Clip

QHVCEditTrackClip* QHVCEditTestEditorCreateVideoClip(QHVCEditTimeline* timeline)
{
    QHVCEditTrackClip* clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:timeline];
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"video1" ofType:@"MOV"];
    [clip setFilePath:filePath type:QHVCEditTrackClipTypeVideo];
    [clip setFileStartTime:0];
    [clip setFileEndTime:3000];
    [clip setSpeed:1.0];
    [clip setVolume:100];
    [clip setPitch:0];
    [clip setFlipX:NO];
    [clip setFlipY:NO];
    [clip setSourceX:0 sourceY:0 sourceWidth:200 sourceHeight:200];
    [clip setSourceRadian:0];
    [clip setSlowMotionVideoInfo:nil];
    
    return clip;
}

QHVCEditTrackClip* QHVCEditTestEditorCreateImageClip(QHVCEditTimeline* timeline)
{
    QHVCEditTrackClip* clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:timeline];
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"test_heic" ofType:@"HEIC"];
    [clip setFilePath:filePath type:QHVCEditTrackClipTypeImage];
    [clip setFileStartTime:0];
    [clip setFileEndTime:3000];
    return clip;
}

QHVCEditTrackClip* QHVCEditTestEditorCreateAudioClip(QHVCEditTimeline* timeline)
{
    QHVCEditTrackClip* clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:timeline];
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"music" ofType:@"mp3"];
    [clip setFilePath:filePath type:QHVCEditTrackClipTypeAudio];
    [clip setFileStartTime:0];
    [clip setFileEndTime:3000];
    return clip;
}

int QHVCEditTestEditorClipParams(QHVCEditTrackClip* clip)
{
    QHVCEditObjectType objType = [clip objType];
    NSInteger clipId = [clip clipId];
    QHVCEditObject* superObj = [clip superObj];
    
    NSString* userData = @"clip";
    [clip setUserData:(__bridge void *)(userData)];
    NSString* checkData = [clip userData];
    if (![userData isEqualToString:checkData])
    {
        return -1;
    }
    
    NSString* filePath = [clip filePath];
    QHVCEditTrackClipType clipType = [clip clipType];
    NSInteger startTime = [clip fileStartTime];
    NSInteger endTime = [clip fileEndTime];
    NSInteger insertTime = [clip insertTime];
    CGFloat speed = [clip speed];
    NSInteger volume = [clip volume];
    NSInteger pitch = [clip pitch];
    BOOL filpX = [clip flipX];
    BOOL flipY = [clip flipY];
    CGRect sourceRect = [clip sourceRect];
    CGFloat sourceRadian = [clip sourceRadian];
    NSArray* slowMotionVideoInfo = [clip slowMotionVideoInfo];
    NSInteger duration = [clip duration];
    
    return 0;
}

void QHVCEditTestEditorCreateClipWithErrorParam(QHVCEditTimeline* timeline)
{
    QHVCEditTrackClip* clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:nil];
    clip = [[QHVCEditTrackClip alloc] initClipWithTimeline:timeline];
    [clip setFilePath:@"" type:QHVCEditTrackClipTypeVideo];
    [clip setSourceX:0 sourceY:0 sourceWidth:0 sourceHeight:0];
    [clip setSourceX:0 sourceY:0 sourceWidth:100 sourceHeight:0];
    [clip setSpeed:0];
    [clip setVolume:-10];
    [clip setVolume:500];
    [clip setPitch:50];
    [clip setPitch:-50];
    
    [clip addEffect:nil];
    [clip deleteEffectById:0];
}

int QHVCEditTestEitorClipWithEffect(QHVCEditTimeline* timeline)
{
    QHVCEditTrackClip* clip = QHVCEditTestEditorCreateVideoClip(timeline);
    QHVCEDIT_TEST_OBJECT(clip);
    
    QHVCEditSequenceTrack* track = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(track);
    [timeline appendTrack:track];
    [track appendClip:clip];
    
    QHVCEditEffect* effect = QHVCEditTestEditorCreateEffect(timeline);
    [effect setStartTime:0];
    [effect setEndTime:3000];
    QHVCEDIT_TEST_OBJECT(effect);
    
    [clip addEffect:effect];
    QHVCEditEffect* check = [clip getEffectById:[effect effectId]];
    if (check != effect)
    {
        return -1;
    }
    
    NSArray* effects = [clip getEffects];
    if ([effects count] <= 0)
    {
        return -1;
    }
    
    [clip addEffect:effect];
    [clip updateEffect:nil];
    [clip updateEffect:effect];
    [clip deleteEffectById:effect.effectId];
    
    return 0;
}

int QHVCEditTestEditorClipAll(void)
{
    //create timeline
    QHVCEditTimeline* timeline = QHVCEditTestEditorCreateTimeline();
    QHVCEDIT_TEST_OBJECT(timeline);
    
    //create track
    QHVCEditSequenceTrack* track = QHVCEditTestEditorCreateSequenceTrack(timeline);
    QHVCEDIT_TEST_OBJECT(track);
    [timeline appendTrack:track];
    
    //create clip with error param
    QHVCEditTestEditorCreateClipWithErrorParam(timeline);
    
    //create true clip
    QHVCEditTrackClip* clip = QHVCEditTestEditorCreateVideoClip(timeline);
    QHVCEDIT_TEST_OBJECT(clip);
    [track appendClip:clip];
    
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditorClipParams(clip));
    
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEitorClipWithEffect(timeline));
    
    QHVCEditEffect* effect = QHVCEditTestEditorCreateEffect(timeline);
    QHVCEDIT_TEST_OBJECT(effect);
    [clip addEffect:effect];
    [effect setEndTime:0];
    [clip updateEffect:effect];

    QHVCEditAudioTransferEffect* audioTransfer = [[QHVCEditAudioTransferEffect alloc] initEffectWithTimeline:timeline];
    QHVCEDIT_TEST_OBJECT(audioTransfer);
    [clip addEffect:audioTransfer];
    
    [timeline deleteTrackById:track.trackId];
    [clip duration];
    [clip deleteEffectById:effect.effectId];
    
    [timeline free];
    [clip superObj];
    [clip addEffect:effect];
    [clip updateEffect:effect];
    [clip getEffectById:effect.effectId];
    [clip getEffects];
    [clip duration];
    
    return 0;
}

