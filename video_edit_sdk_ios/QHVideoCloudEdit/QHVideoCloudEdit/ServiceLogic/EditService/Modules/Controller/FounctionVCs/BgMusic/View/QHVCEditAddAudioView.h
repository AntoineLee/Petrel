//
//  QHVCEditAddAudioView.h
//  QHVideoCloudToolSet
//
//  Created by deng on 2018/1/18.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import <UIKit/UIKit.h>

@class QHVCEditTrackClipItem;
typedef void (^AudioSelectBlock)(QHVCEditTrackClipItem *audioItem);
typedef void (^ConfirmBlock)();
typedef void (^CancelBlock)();

@interface QHVCEditAddAudioView : UIView

@property (nonatomic, strong) QHVCEditTrackClipItem *audioItem;
@property (nonatomic, copy) AudioSelectBlock audioSelectBlock;
@property (nonatomic, copy) ConfirmBlock confirmBlock;
@property (nonatomic, copy) CancelBlock cancelBlock;

@end
