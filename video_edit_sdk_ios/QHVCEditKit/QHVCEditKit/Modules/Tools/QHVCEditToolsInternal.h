//
//  QHVCEditToolsInternal.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/6/26.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>

@class QHVCEditFileInfo;
@interface QHVCEditToolsInternal : NSObject

+ (QHVCEditFileInfo *)getFileInfo:(NSString *)filePath;

@end
