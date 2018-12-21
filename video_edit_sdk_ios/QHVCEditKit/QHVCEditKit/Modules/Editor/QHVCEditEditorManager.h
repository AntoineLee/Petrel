//
//  QHVCEditEditorManager.h
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/27.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCEditEditor.h"

@interface QHVCEditEditorManager : NSObject

+ (instancetype)sharedInstance;

- (void)addEditor:(QHVCEditEditor *)editor editorId:(NSInteger)editorId;
- (QHVCEditEditor *)getEditor:(NSInteger)timelineId;
- (void)deleteEditor:(NSInteger)editorId;

@end
