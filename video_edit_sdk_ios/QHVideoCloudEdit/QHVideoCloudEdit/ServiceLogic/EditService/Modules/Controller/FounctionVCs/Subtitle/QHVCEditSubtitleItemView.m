//
//  QHVCEditSubtitleItemView.m
//  QHVideoCloudEdit
//
//  Created by liyue-g on 2018/11/25.
//  Copyright © 2018 yangkui. All rights reserved.
//

#import "QHVCEditSubtitleItemView.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditMediaEditorConfig.h"

@interface QHVCEditSubtitleItemView ()

@property (nonatomic, retain) UIImage* subtitleImage;
@property (nonatomic, retain) UIImageView* imageView;

@end

@implementation QHVCEditSubtitleItemView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    return self;
}

- (void)setImage:(UIImage *)image
{
    _subtitleImage = image;
    [self.imageView setImage:_subtitleImage];
}

- (void)updateEffect
{
    if (self.effect)
    {
        self.effect.sticker = [self getStickerImage];
        SAFE_BLOCK(self.refreshPlayerBlock, YES);
    }
}

- (void)prepareSubviews
{
    self.imageView = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, CGRectGetWidth(self.frame), CGRectGetHeight(self.frame))];
    [self.imageView setImage:self.subtitleImage];
    [self.imageView setContentMode:UIViewContentModeScaleAspectFit];
    [self.imageView setHidden:YES];
    [self addSubview:self.imageView];
    
    self.textView = [[UITextView alloc] initWithFrame:CGRectMake(0, 0, CGRectGetWidth(self.imageView.frame), CGRectGetHeight(self.imageView.frame))];
    [self.textView setTextAlignment:NSTextAlignmentCenter];
    [self.textView setTextColor:[UIColor blackColor]];
    [self.textView setBackgroundColor:[UIColor clearColor]];
    [self.textView setFont:[UIFont systemFontOfSize:10]];
    [self.textView setContentSize:CGSizeMake(0, 10)];
    [self.imageView addSubview:self.textView];
    
    UIButton* deleteBtn = [[UIButton alloc] initWithFrame:CGRectMake(CGRectGetWidth(self.frame) - 18, 0, 18, 18)];
    [deleteBtn setImage:[UIImage imageNamed:@"edit_selected_delete"] forState:UIControlStateNormal];
    [deleteBtn addTarget:self action:@selector(deleteAction:) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:deleteBtn];
    
    if (!self.effect)
    {
        CGRect rect = [self rectToOutputRect];
        self.effect = [[QHVCEditMediaEditor sharedInstance] createStickerEffect];
        self.effect.startTime = 0;
        self.effect.endTime = [[QHVCEditMediaEditor sharedInstance] getTimelineDuration];
        self.effect.sticker = [self getStickerImage];
        self.effect.renderX = rect.origin.x;
        self.effect.renderY = rect.origin.y;
        self.effect.renderWidth = rect.size.width;
        self.effect.renderHeight = rect.size.height;
        self.effect.renderRadian = self.radian;
        [[QHVCEditMediaEditor sharedInstance] addStickerEffectToTimeline:self.effect];
        SAFE_BLOCK(self.refreshPlayerForBasicParamBlock);
    }
}

- (UIImage *)getStickerImage
{
    [self.imageView setHidden:NO];
    UIImage* image = [QHVCEditPrefs convertViewToImage:self.imageView];
    [self.imageView setHidden:YES];
    return image;
}

#pragma mark - Render Rect Methods

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

- (void)updateRenderRect
{
    CGRect rect = [self rectToOutputRect];
    self.effect.renderX = rect.origin.x;
    self.effect.renderY = rect.origin.y;
    self.effect.renderWidth = rect.size.width;
    self.effect.renderHeight = rect.size.height;
    self.effect.renderRadian = self.radian;
    [[QHVCEditMediaEditor sharedInstance] updateStickerEffect:self.effect];
}

#pragma mark - Event Methods

- (void)deleteAction:(UIButton *)sender
{
    [self removeFromSuperview];
    [[QHVCEditMediaEditor sharedInstance] deleteStickerEffect:self.effect];
    [[[QHVCEditMediaEditorConfig sharedInstance] subtitleItemArray] removeObject:self];
    SAFE_BLOCK(self.refreshPlayerForBasicParamBlock);
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

-(void)pinchGestureAction:(BOOL)isEnd
{
    [self updateRenderRect];
    SAFE_BLOCK(self.refreshPlayerBlock, isEnd);
}

- (void)tapGestureAction
{
    [[NSNotificationCenter defaultCenter] postNotificationName:QHVCEDIT_DEFINE_NOTIFY_SHOW_SUBTITLE_FUNCTION object:self];
}

@end
