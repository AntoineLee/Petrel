//
//  QHVCEditOverlayItemView.m
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/8/7.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditOverlayItemView.h"
#import "QHVCEditMediaEditor.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditMediaEditorConfig.h"

@interface QHVCEditOverlayItemView ()

@end

@implementation QHVCEditOverlayItemView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
    }
    
    return self;
}

- (void)setPhotoItem:(QHVCPhotoItem *)item
{
    self.clipItem = [[QHVCEditTrackClipItem alloc] initWithPhotoItem:item];
    [[QHVCEditMediaEditor sharedInstance] addOverlayClip:self.clipItem];
    [self updateRenderRect];
    [self hideBorder:YES];
}

- (void)updateRenderRect
{
    QHVCEditTrackClip* clip = self.clipItem.clip;
    QHVCEditOverlayTrack* track = (QHVCEditOverlayTrack *)[clip superObj];
    if (track)
    {
        CGRect rect = [self rectToOutputRect];
        [track setRenderX:rect.origin.x
                  renderY:rect.origin.y
              renderWidth:rect.size.width
             renderHeight:rect.size.height];
        [track setRenderRadian:self.radian];
    }
}

//view尺寸转为画布尺寸
- (CGRect)rectToOutputRect
{
    UIView* view = self;
    CGRect rect = view.frame;
    CGFloat radian = self.radian;
    
    view.transform = CGAffineTransformRotate(view.transform, -radian);
    rect = CGRectMake(rect.origin.x, rect.origin.y, view.frame.size.width, view.frame.size.height);
    view.transform = CGAffineTransformRotate(view.transform, radian);
    
    CGSize outputSize = [QHVCEditMediaEditorConfig sharedInstance].outputSize;
    CGFloat scaleW = outputSize.width/CGRectGetWidth(self.superview.frame);
    CGFloat scaleH = outputSize.height/CGRectGetHeight(self.superview.frame);
    
    CGFloat x = rect.origin.x * scaleW;
    CGFloat y = rect.origin.y * scaleH;
    NSInteger w = rect.size.width * scaleW;
    NSInteger h = rect.size.height * scaleH;
    
    CGRect newRect = CGRectMake(x, y, w, h);
    return newRect;
}

#pragma mark - Gesture Action

- (void)tapGestureAction
{
    SAFE_BLOCK(self.tapAction, self);
}

- (void)moveGestureAction:(BOOL)isEnd
{
    [self updateRenderRect];
    SAFE_BLOCK(self.refreshPlayerBlock, isEnd);
}

- (void)rotateGestureAction:(BOOL)isEnd
{
    [self updateRenderRect];
    SAFE_BLOCK(self.refreshPlayerBlock, isEnd);
}

- (void)pinchGestureAction:(BOOL)isEnd
{
    [self updateRenderRect];
    SAFE_BLOCK(self.refreshPlayerBlock, isEnd);
}

@end
