//
//  QHVCEditCoreImageOutput.h
//  QHVCEditKit
//
//  Created by liyue-g on 2017/12/11.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreImage/CoreImage.h>

@protocol QHVCEditCIOutputProtocol <NSObject>
@optional
- (void)processImage:(CIImage *)image timestampMs:(NSInteger)timestampMs userData:(id)userData;

@end

@protocol QHVCEditCIOutputDelegate <NSObject>
@optional
- (void)onFrameCallback:(void *)data dataLen:(int)len width:(int)width height:(int)height timestamp:(NSInteger)timestamp userData:(id)userData;
@end

@interface QHVCEditCIOutput : NSObject <QHVCEditCIOutputProtocol>

- (instancetype)initWithOutputSize:(CGSize)size;
- (void)setDelegate:(id<QHVCEditCIOutputDelegate>)delegate;
- (void)setDisable;

@end
