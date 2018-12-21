//
//  QHVCEditBgMusic.m
//  QHVideoCloudToolSet
//
//  Created by yinchaoyu on 2018/7/13.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditBgMusic.h"
#import "QHVCEditFrameView.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditAudioItem.h"
#import "AudioWaveForm.h"
#import "EZAudioFloatData.h"
#import "QHVCEditMainEditVC.h"
#import "QHVCEditMediaEditor.h"
#import "QHVCEditMediaEditorConfig.h"

#define MAX_VALUE 32768.0

@interface QHVCEditBgMusic ()
{
    QHVCEditFrameView *_frameView;
    QHVCEditAddAudioView *_audioView;
    float _originAudioVolume;
    float _musicAudioVolume;
    QHVCEditAudioProducer *_audioProducer;
    EZAudioFloatData *_waveformData;
    QHVCEditMainEditVC *_mainVC;
    NSTimeInterval _insertStartTime;
    NSTimeInterval _insertEndTime;
}

@property (nonatomic, strong) QHVCEditTrackClipItem *currentAudioItem;
@property (nonatomic, strong) NSMutableArray<QHVCEditTrackClipItem *> *audiosArray;
@property (nonatomic, strong) NSMutableArray<NSMutableArray *> *audioInfos;

@end

@implementation QHVCEditBgMusic

- (instancetype)init
{
    if (self = [super init]) {
        [self initParams];
    }
    
    return self;
}

- (void)initParams
{
    _originAudioVolume = [QHVCEditMediaEditorConfig sharedInstance].originAudioVolume;
    _musicAudioVolume = [QHVCEditMediaEditorConfig sharedInstance].musicAudioVolume;
    _audiosArray = [NSMutableArray array];
    _audioInfos = [NSMutableArray arrayWithArray:[QHVCEditMediaEditorConfig sharedInstance].audioTimestamp];
}

- (void)handleFrameView:(QHVCEditFrameView *)frameView mainVC:(QHVCEditMainEditVC *)mainVC
{
    _frameView = frameView;
    _mainVC = mainVC;
//    _frameView.timeStamp = [QHVCEditMediaEditorConfig sharedInstance].audioTimestamp;
    
//    WEAK_SELF
//    _frameView.addCompletion = ^(NSInteger insertStartMs) {
//        STRONG_SELF
//        [self handleAddAction:insertStartMs];
//    };
//    _frameView.doneCompletion = ^(NSInteger insertEndMs) {
//        STRONG_SELF
//        self->_insertEndTime = insertEndMs;
//        [self handleDoneAction];
//    };
//    _frameView.editCompletion = ^{
//        STRONG_SELF
//        [self handleEditAction];
//    };
}

- (void)handleAddAction:(NSTimeInterval)insertStartMs
{
    _insertStartTime = insertStartMs;
    _currentAudioItem = [[QHVCEditTrackClipItem alloc] init];
    _currentAudioItem.clipType = QHVCEditTrackClipTypeAudio;
    _currentAudioItem.insertMs = insertStartMs;
    _currentAudioItem.startMs = 0;
    
    _audioView = [[NSBundle mainBundle] loadNibNamed:[[QHVCEditAddAudioView class] description] owner:self options:nil][0];
    _audioView.frame = CGRectMake(0, 0, kScreenWidth, 200);
    _audioView.audioItem = self.currentAudioItem;
    WEAK_SELF
    _audioView.audioSelectBlock = ^(QHVCEditTrackClipItem *audioItem) {
    };
    _audioView.confirmBlock = ^{
        STRONG_SELF
        [self->_mainVC.funcViewHeightConstraint setConstant:self->_frameView.frame.size.height];
        [self handleDoneAction];
    };
    
    _audioView.cancelBlock = ^{
        STRONG_SELF
        [self->_mainVC.funcViewHeightConstraint setConstant:self->_frameView.frame.size.height];
    };
    [_mainVC.funcViewHeightConstraint setConstant:_audioView.frame.size.height + 30];
    [_mainVC.functionView addSubview:_audioView];
}

- (void)handleDoneAction
{
//    if(_viewType == QHVCEditFrameStatusEdit)
//    {
//        if (_currentAudioItem.filePath.length > 0) {
//            _audioView.hidden = YES;
//            _originAudioVolume = [QHVCEditMediaEditorConfig sharedInstance].originAudioVolume;
//            _musicAudioVolume = [QHVCEditMediaEditorConfig sharedInstance].musicAudioVolume;
//
//            NSString* filePath = [[NSBundle mainBundle] pathForResource:_currentAudioItem.filePath ofType:@"mp3"];
//            if (!filePath) {
//                filePath = [[NSBundle mainBundle] pathForResource:_currentAudioItem.filePath ofType:@"mp4"];
//            }
//            _currentAudioItem.filePath = filePath;
//            [self updateViewType:QHVCEditFrameStatusDone];
//        }
//        else
//        {
//            [_audioView removeFromSuperview];
//            _audioView = nil;
//
//            [self updateViewType:QHVCEditFrameStatusAdd];
//
//            if (![_audiosArray containsObject:_currentAudioItem]) {
//                [_audiosArray addObject:_currentAudioItem];
//            }
//            [[QHVCEditMediaEditor sharedInstance] mainAudioTrackAppendClips:_audiosArray];
//            [[QHVCEditMediaEditor sharedInstance] setMainTrackVolume:_originAudioVolume];
//            [[QHVCEditMediaEditor sharedInstance] setMainAduioTrackVolume:_musicAudioVolume];
//            [_mainVC resetPlayer:[_frameView fetchCurrentTimeStampMs]];
//        }
//    }
//    else if (_viewType == QHVCEditFrameStatusDone)
//    {
//        [_audioView removeFromSuperview];
//        _audioView = nil;
//
//        [self updateViewType:QHVCEditFrameStatusAdd];
//
//        self.currentAudioItem.endMs = [_frameView fetchCurrentTimeStampMs] - _currentAudioItem.insertMs;
//        if (![_audiosArray containsObject:_currentAudioItem]) {
//            [_audiosArray addObject:_currentAudioItem];
//        }
//        [[QHVCEditMediaEditor sharedInstance] mainAudioTrackAppendClips:_audiosArray];
//        [[QHVCEditMediaEditor sharedInstance] setMainTrackVolume:_originAudioVolume];
//        [[QHVCEditMediaEditor sharedInstance] setMainAduioTrackVolume:_musicAudioVolume];
//        [_mainVC resetPlayer:[_mainVC curPlayerTime]];
//
//        [self addWaveFormView];
//    }
}

- (void)handleEditAction
{
    [self backAction:nil];
}

- (void)handleDiscardAction
{
    [_audioView removeFromSuperview];
    _audioView = nil;
    if (_audioProducer) {
        [_audioProducer stopProducer];
        _audioProducer = nil;
    }
    [[QHVCEditMediaEditor sharedInstance] mainAudioTrackDeleteAllClips];
    [[QHVCEditMediaEditor sharedInstance] setMainTrackVolume:[[QHVCEditMediaEditorConfig sharedInstance] volume]];
//    [_mainVC resetPlayer:[_frameView fetchCurrentTimeStampMs]];
}

- (void)backAction:(UIButton *)btn
{
//    if(_viewType == QHVCEditFrameStatusEdit)
//    {
//        [_audioView removeFromSuperview];
//        _audioView = nil;
//
//        [self updateViewType:QHVCEditFrameStatusAdd];
//        [QHVCEditMediaEditorConfig sharedInstance].originAudioVolume = _originAudioVolume;
//    }
//    else if (_viewType == QHVCEditFrameStatusDone)
//    {
//        _audioView.hidden = NO;
//
//        [self updateViewType:QHVCEditFrameStatusEdit];
//    }
//    else
//    {
//        if (_audioProducer) {
//            [_audioProducer stopProducer];
//            _audioProducer = nil;
//        }
//    }
}


- (void)addWaveFormView
{
    if (!_audioProducer) {
        _audioProducer = [[QHVCEditAudioProducer alloc] initWithTimeline:[[QHVCEditMediaEditor sharedInstance] getTimeline]];
        QHVCEditTrack *mainAudioTrack = [[QHVCEditMediaEditor sharedInstance] getMainAudioTrack];
        [_audioProducer startProducerWithTrack:mainAudioTrack.trackId startTime:0  endTime:mainAudioTrack.duration];
    }
    
}

#pragma mark - <QHVCEditAudioProducerDelegate>

- (void)onPCMData:(unsigned char *)pcm size:(int)size
{
    [self dataWithNumberOfPoints:1024 data:pcm length:size];
    AudioWaveForm* waveForm = [AudioWaveForm sharedManager];
    EZAudioPlot* audioPlot = [waveForm generateWave:_waveformData frame:CGRectMake(0, 0, kScreenWidth, 50)];
    
    [_frameView addSubview:audioPlot];
}

- (EZAudioFloatData *)dataWithNumberOfPoints:(UInt32)numberOfPoints data:(unsigned char *)audioData length:(NSInteger)length
{
    UInt32 channels = 2;
    if (channels == 0)
    {
        return nil;
    }
    int16_t *audioData16 = (int16_t *)audioData;
    
    float **data = (float **)malloc( sizeof(float*) * channels );
    for (int i = 0; i < channels; i++)
    {
        data[i] = (float *)malloc( sizeof(float) * numberOfPoints );
    }
    
    // calculate the required number of frames per buffer
    SInt64 totalFrames = length/2;//两个unsigned char 为一个声音数据
    SInt64 framesPerBuffer = ((SInt64) totalFrames / numberOfPoints);//包含全部交叉存储声道数据
    SInt64 framesPerChannel = framesPerBuffer / channels;
    
    // read through file and calculate rms at each point
    for (SInt64 i = 0; i < numberOfPoints; i++)
    {
        int16_t *buffer = &audioData16[framesPerBuffer*i];
        for (int channel = 0; channel < channels; channel++)
        {
            float channelData[framesPerChannel];
            for (int frame = 0; frame < framesPerChannel; frame++)
            {
                channelData[frame] = buffer[frame * channels + channel]/MAX_VALUE;
            }
            float rms = [[self class] RMS:channelData length:framesPerChannel];
            data[channel][i] = rms;
        }
    }
    
    _waveformData = [EZAudioFloatData dataWithNumberOfChannels:channels
                                                       buffers:(float **)data
                                                    bufferSize:numberOfPoints];
    
    // cleanup
    for (int i = 0; i < channels; i++)
    {
        free(data[i]);
    }
    free(data);
    
    return _waveformData;
}

+ (float)RMS:(float *)buffer length:(SInt64)bufferSize
{
    float sum = 0.0;
    for(int i = 0; i < bufferSize; i++)
    {
        sum += buffer[i] * buffer[i];
    }
    
    float val = sqrtf( sum / bufferSize);
    
    return val;
}

@end
