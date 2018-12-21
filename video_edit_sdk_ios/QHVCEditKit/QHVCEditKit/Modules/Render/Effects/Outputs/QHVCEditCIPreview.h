//
//  QHVCEditCoreImagePreview.h
//  QHVCEditKit
//
//  Created by liyue-g on 2017/12/11.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "QHVCEditCIOutput.h"
#import "QHVCEditConfig.h"
#import "QHVCEditCommonDef.h"


@interface QHVCEditCIPreview : UIView <QHVCEditCIOutputProtocol>

- (void)free;
- (void)setBackgroundColorRed:(CGFloat)redComponent
                        green:(CGFloat)greenComponent
                         blue:(CGFloat)blueComponent
                        alpha:(CGFloat)alphaComponent;
- (void)setDisable;
- (CIImage *)getCurrentFrame;

- (void)renderFrame;
- (void)screenLocked:(NSNotification *)notification;
- (void)willEnterForeground:(NSNotification *)notification;
- (void)willResignActive:(NSNotification *)notification;
- (void)didBecomeActive:(NSNotification *)notification;

@property (nonatomic, assign) QHVCEditFillMode fillMode;
@property (nonatomic, assign) CGRect previewFrame;
@property (atomic,    assign) NSInteger inputCount;
@property (atomic,    assign) NSInteger renderedCount;

@end
