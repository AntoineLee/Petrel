//
//  QHVCEditProducerInternal.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/6/29.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditCommonDef.h"
#import "QHVCEditProducer.h"

@class QHVCEditTimeline;

@interface QHVCEditProducerInternal : NSObject

- (instancetype)initWithTimeline:(QHVCEditTimeline *)timeline;
- (QHVCEditError)setProducerDelegate:(id<QHVCEditProducerDelegate>)delegate;
- (QHVCEditError)free;
- (QHVCEditError)start;
- (QHVCEditError)stop;

//内部使用
@property (nonatomic, assign) NSInteger operationCount;
- (void)onProducerDataCB:(void *)producerHandle
                    data:(void *)data;
- (void)onProducerEventCB:(void *)status;

@end
