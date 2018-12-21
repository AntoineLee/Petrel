//
//  QHVCEditProducerOperation.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/6/29.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>

@class QHVCEditEditor;
@class QHVCEditRenderParam;

@protocol QHVCEditProducerOperationDelegate <NSObject>
@optional
- (void)onFrameCallback:(void *)data dataLen:(int)len width:(int)width height:(int)height timestamp:(NSInteger)timestamp userData:(id)userData;
@end

@interface QHVCEditProducerOperation : NSObject

- (instancetype)initWithEditor:(QHVCEditEditor *)editor;
- (void)free;

- (void)setDelegate:(id<QHVCEditProducerOperationDelegate>)delegate;
- (void)producerParam:(QHVCEditRenderParam *)param;

@property (nonatomic, assign) int inputCount;
@property (nonatomic, assign) int outputCount;

@end
