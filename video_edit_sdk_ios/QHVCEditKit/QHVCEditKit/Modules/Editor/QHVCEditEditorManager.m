//
//  QHVCEditEditorManager.m
//  QHVCEditKit
//
//  Created by liyue-g on 2018/4/27.
//  Copyright © 2018年 liyue-g. All rights reserved.
//

#import "QHVCEditEditorManager.h"

@interface QHVCEditEditorManager ()
@property (nonatomic, strong) NSMutableDictionary* editors;

@end

@implementation QHVCEditEditorManager

+ (instancetype)sharedInstance
{
    static QHVCEditEditorManager* s_instance = nil;
    static dispatch_once_t predic;
    dispatch_once(&predic, ^{
        s_instance = [[QHVCEditEditorManager alloc] init];
    });
    return s_instance;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.editors = [[NSMutableDictionary alloc] initWithCapacity:0];
    }

    return self;
}

- (void)addEditor:(QHVCEditEditor *)editor editorId:(NSInteger)editorId
{
    NSNumber* editorIdNum = [NSNumber numberWithInteger:editorId];
    [self.editors setObject:editor forKey:editorIdNum];
}

- (QHVCEditEditor *)getEditor:(NSInteger)timelineId
{
    NSNumber* editorIdNum = [NSNumber numberWithInteger:timelineId];
    QHVCEditEditor* editor = [self.editors objectForKey:editorIdNum];
    return editor;
}

- (void)deleteEditor:(NSInteger)editorId
{
    NSNumber* editorIdNum = [NSNumber numberWithInteger:editorId];
    [self.editors removeObjectForKey:editorIdNum];
}

@end
