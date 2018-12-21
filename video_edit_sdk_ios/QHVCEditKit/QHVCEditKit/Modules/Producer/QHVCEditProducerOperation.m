//
//  QHVCEditProducerOperation.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/6/29.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditProducerOperation.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditLogger.h"
#import "QHVCEditMacroDefs.h"

#import "QHVCEditCommonDef.h"
#import "QHVCEditEditor.h"
#import "QHVCEditEditor+Timeline.h"
#import "QHVCEditCIOutput.h"
#import "QHVCEditRenderOperation.h"

@interface QHVCEditProducerOperation () <QHVCEditCIOutputDelegate, QHVCEditRenderOperationDelegate>
@property (nonatomic, retain) dispatch_queue_t processQueue;
@property (nonatomic, retain) QHVCEditEditor* editor;
@property (nonatomic,   weak) id<QHVCEditProducerOperationDelegate> delegate;
@property (nonatomic, retain) QHVCEditCIOutput* producerCIOutput;
@property (nonatomic, retain) QHVCEditRenderOperation* renderOperation;

@end

@implementation QHVCEditProducerOperation

- (instancetype)initWithEditor:(QHVCEditEditor *)editor
{
    self = [super init];
    if (self)
    {
        self.processQueue = dispatch_queue_create("QHVCEditProducerProcessQueue", NULL);
        self.editor = editor;
        self.inputCount = 0;
        self.outputCount = 0;
        
        self.producerCIOutput = [self createCIProducerOutput];
        self.renderOperation = [self createRenderOperation];
    }
    
    return self;
}

- (void)free
{
    [self.producerCIOutput setDisable];
    self.producerCIOutput = nil;
    self.renderOperation = nil;
}

- (void)setDelegate:(id<QHVCEditProducerOperationDelegate>)delegate
{
    _delegate = delegate;
}

- (void)producerParam:(QHVCEditRenderParam *)param
{
    QHVCEDIT_WEAK_SELF
    dispatch_async(self.processQueue, ^{
        QHVCEDIT_STRONG_SELF
        self.inputCount ++;
        [self.renderOperation processRenderParam:param];
    });
}

#pragma mark - output methods

- (QHVCEditCIOutput *)createCIProducerOutput
{
    QHVCEditCIOutput* makerOutput = [[QHVCEditCIOutput alloc] initWithOutputSize:CGSizeMake([self.editor outputSize].width, [self.editor outputSize].height)];
    [makerOutput setDelegate:self];
    return makerOutput;
}

- (void)onFrameCallback:(void *)data dataLen:(int)len width:(int)width height:(int)height timestamp:(NSInteger)timestamp userData:(id)userData
{
    self.outputCount ++;
    if (self.delegate && [self.delegate respondsToSelector:@selector(onFrameCallback:dataLen:width:height:timestamp:userData:)])
    {
        [self.delegate onFrameCallback:data dataLen:len width:width height:height timestamp:timestamp userData:userData];
    }
}

#pragma mark - render methods

- (QHVCEditRenderOperation *)createRenderOperation
{
    QHVCEditRenderOperation* renderOperation = [[QHVCEditRenderOperation alloc] initWithEditor:self.editor output:self.producerCIOutput ];
    [renderOperation setOutputSize:[self.editor outputSize] color:[self.editor outputBgColor]];
    [renderOperation setDelegate:self];
    return renderOperation;
}

- (void)frameDidStartProcessing:(NSInteger)timestampMs
{
    LogDebug(@"frameDidStartProcessing, %ld", (long)timestampMs);
}

- (void)frameDidStopProcessing:(NSInteger)timestampMs
{
    LogDebug(@"frameDidStopProcessing, %ld", (long)timestampMs);
}

@end
