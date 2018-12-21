//
//  QHVCEditRenderOperation.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/7/9.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIkit.h>
#import "QHVCEditCIOutput.h"
#import "QHVCEditRenderParam.h"
#import "QHVCEditEditor.h"

@protocol QHVCEditRenderOperationDelegate <NSObject>
@optional
- (void)frameDidStartProcessing:(NSInteger)timestampMs;
- (void)frameDidStopProcessing:(NSInteger)timestampMs;

@end

@interface QHVCEditRenderOperation : NSObject

- (instancetype)initWithEditor:(QHVCEditEditor *)editor output:(id<QHVCEditCIOutputProtocol>)output;

- (void)setOutputSize:(CGSize)size color:(NSString *)color;
- (void)setDelegate:(id<QHVCEditRenderOperationDelegate>)delegate;
- (void)processRenderParam:(QHVCEditRenderParam *)param;

@end

