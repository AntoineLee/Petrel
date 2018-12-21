//
//  QHVCEditSubtitle.h
//  QHVideoCloudToolSet
//
//  Created by yinchaoyu on 2018/7/4.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditPlayerBaseVC.h"

@class QHVCEditFrameView;
@interface QHVCEditSubtitle : NSObject

- (void)subtitleAction:(QHVCEditFrameView *)view withVC:(QHVCEditPlayerBaseVC *)vc;

@end
