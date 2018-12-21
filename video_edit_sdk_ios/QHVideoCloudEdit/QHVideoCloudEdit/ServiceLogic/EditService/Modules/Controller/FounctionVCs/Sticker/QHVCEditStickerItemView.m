//
//  QHVCEditStickerItemView.m
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/8/30.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditStickerItemView.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditMediaEditorConfig.h"
#import "QHVCEditPlayerBaseVC.h"

@interface QHVCEditStickerItemView ()
@property (nonatomic, retain) UIImage* stickerImage;
@property (nonatomic, retain) UIImageView* imageView;

@end

@implementation QHVCEditStickerItemView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
    }
    
    return self;
}

- (void)setImage:(UIImage *)image
{
    _stickerImage = image;
}

- (void)addAnimationOfIndex:(NSInteger)animationIndex
{
    switch (animationIndex)
    {
        case 0:
        {
            //淡入淡出
            [self addAnimationFadeInOut];
            break;
        }
        case 1:
        {
            //滑入滑出
            [self addAnimationMoveInOut];
            break;
        }
        case 2:
        {
            //弹入弹出
            [self addAnimationJumpInOut];
            break;
        }
        case 3:
        {
            //旋入旋出
            [self addAnimationRotateInOut];
            break;
        }
        default:
            break;
    }
}

- (void)prepareSubviews
{
    self.imageView = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, CGRectGetWidth(self.frame), CGRectGetHeight(self.frame))];
    [self.imageView setImage:self.stickerImage];
    [self.imageView setContentMode:UIViewContentModeScaleAspectFit];
    [self.imageView setHidden:YES];
    [self addSubview:self.imageView];
    
    UIButton* deleteBtn = [[UIButton alloc] initWithFrame:CGRectMake(CGRectGetWidth(self.frame) - 18, 0, 18, 18)];
    [deleteBtn setImage:[UIImage imageNamed:@"edit_selected_delete"] forState:UIControlStateNormal];
    [deleteBtn addTarget:self action:@selector(deleteAction:) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:deleteBtn];
    
    if (!self.effectImage)
    {
        CGRect rect = [self rectToOutputRect];
        self.effectImage = [[QHVCEditMediaEditor sharedInstance] createStickerEffect];
        self.effectImage.startTime = 0;
        self.effectImage.endTime = [[QHVCEditMediaEditor sharedInstance] getTimelineDuration];
        self.effectImage.sticker = [self getStickerImage];
        self.effectImage.renderX = rect.origin.x;
        self.effectImage.renderY = rect.origin.y;
        self.effectImage.renderWidth = rect.size.width;
        self.effectImage.renderHeight = rect.size.height;
        self.effectImage.renderRadian = self.radian;
        [[QHVCEditMediaEditor sharedInstance] addStickerEffectToTimeline:self.effectImage];
        
        [self setHidden:YES];
        WEAK_SELF
        [self.playerBaseVC refreshPlayerForEffectBasicParams:^{
            STRONG_SELF
            [self setHidden:NO];
        }];
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
    self.effectImage.renderX = rect.origin.x;
    self.effectImage.renderY = rect.origin.y;
    self.effectImage.renderWidth = rect.size.width;
    self.effectImage.renderHeight = rect.size.height;
    self.effectImage.renderRadian = self.radian;
    [[QHVCEditMediaEditor sharedInstance] updateStickerEffect:self.effectImage];
}

#pragma mark - Animation Methods

- (void)addAnimationFadeInOut
{
    NSTimeInterval duration = self.effectImage.endTime - self.effectImage.startTime;
    NSTimeInterval lastTime = MIN(500, duration/2.0);
    QHVCEffectVideoTransferParam* fadeIn = [[QHVCEffectVideoTransferParam alloc] init];
    fadeIn.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeIn.startTime = 0;
    fadeIn.endTime = lastTime;
    fadeIn.startValue = 0;
    fadeIn.endValue = 1.0;
    
    QHVCEffectVideoTransferParam* fadeOut = [[QHVCEffectVideoTransferParam alloc] init];
    fadeOut.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeOut.startTime = duration - lastTime;
    fadeOut.endTime = duration;
    fadeOut.startValue = 1.0;
    fadeOut.endValue = 0;
    
    NSArray* transfers = @[fadeIn, fadeOut];
    self.effectImage.videoTransfer = transfers;
    [[QHVCEditMediaEditor sharedInstance] updateStickerEffect:self.effectImage];
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)addAnimationMoveInOut
{
    NSTimeInterval duration = self.effectImage.endTime - self.effectImage.startTime;
    NSTimeInterval lastTime = MIN(500, duration/2.0);
    QHVCEffectVideoTransferParam* moveIn = [[QHVCEffectVideoTransferParam alloc] init];
    moveIn.transferType = QHVCEffectVideoTransferTypeOffsetX;
    moveIn.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    moveIn.startTime = 0;
    moveIn.endTime = lastTime;
    moveIn.startValue = -kOutputVideoWidth/10.0;
    moveIn.endValue = 0;
    
    QHVCEffectVideoTransferParam* moveOut = [[QHVCEffectVideoTransferParam alloc] init];
    moveOut.transferType = QHVCEffectVideoTransferTypeOffsetX;
    moveOut.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    moveOut.startTime = duration - lastTime;
    moveOut.endTime = duration;
    moveOut.startValue = 0;
    moveOut.endValue = kOutputVideoWidth/10.0;
    
    QHVCEffectVideoTransferParam* fadeIn = [[QHVCEffectVideoTransferParam alloc] init];
    fadeIn.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeIn.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    fadeIn.startTime = 0;
    fadeIn.endTime = lastTime;
    fadeIn.startValue = 0;
    fadeIn.endValue = 1.0;
    
    QHVCEffectVideoTransferParam* fadeOut = [[QHVCEffectVideoTransferParam alloc] init];
    fadeOut.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeOut.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    fadeOut.startTime = duration - lastTime;
    fadeOut.endTime = duration;
    fadeOut.startValue = 1.0;
    fadeOut.endValue = 0;
    
    QHVCEffectVideoTransferParam* scaleIn = [[QHVCEffectVideoTransferParam alloc] init];
    scaleIn.transferType = QHVCEffectVideoTransferTypeScale;
    scaleIn.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    scaleIn.startTime = 0;
    scaleIn.endTime = lastTime;
    scaleIn.startValue = 0.6;
    scaleIn.endValue = 1.0;
    
    QHVCEffectVideoTransferParam* scaleOut = [[QHVCEffectVideoTransferParam alloc] init];
    scaleOut.transferType = QHVCEffectVideoTransferTypeScale;
    scaleOut.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    scaleOut.startTime = duration - lastTime;
    scaleOut.endTime = duration;
    scaleOut.startValue = 1.0;
    scaleOut.endValue = 0.6;
    
    NSArray* transfers = @[moveIn, moveOut, scaleIn, scaleOut, fadeIn, fadeOut];
    self.effectImage.videoTransfer = transfers;
    [[QHVCEditMediaEditor sharedInstance] updateStickerEffect:self.effectImage];
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)addAnimationJumpInOut
{
    NSTimeInterval duration = self.effectImage.endTime - self.effectImage.startTime;
    NSTimeInterval lastTime = MIN(500, duration/2.0);
    QHVCEffectVideoTransferParam* jumpIn = [[QHVCEffectVideoTransferParam alloc] init];
    jumpIn.transferType = QHVCEffectVideoTransferTypeOffsetY;
    jumpIn.startTime = 0;
    jumpIn.endTime = lastTime;
    jumpIn.startValue = kOutputVideoHeight/10.0;
    jumpIn.endValue = 0;
    
    QHVCEffectVideoTransferParam* jumpOut = [[QHVCEffectVideoTransferParam alloc] init];
    jumpOut.transferType = QHVCEffectVideoTransferTypeOffsetY;
    jumpOut.startTime = duration - lastTime;
    jumpOut.endTime = duration;
    jumpOut.startValue = 0;
    jumpOut.endValue = -kOutputVideoHeight/10.0;
    
    QHVCEffectVideoTransferParam* scaleIn = [[QHVCEffectVideoTransferParam alloc] init];
    scaleIn.transferType = QHVCEffectVideoTransferTypeScale;
    scaleIn.startTime = 0;
    scaleIn.endTime = lastTime;
    scaleIn.startValue = 0.1;
    scaleIn.endValue = 1.0;
    
    QHVCEffectVideoTransferParam* scaleOut = [[QHVCEffectVideoTransferParam alloc] init];
    scaleOut.transferType = QHVCEffectVideoTransferTypeScale;
    scaleOut.startTime = duration - lastTime;
    scaleOut.endTime = duration;
    scaleOut.startValue = 1.0;
    scaleOut.endValue = 0.1;
    
    QHVCEffectVideoTransferParam* fadeIn = [[QHVCEffectVideoTransferParam alloc] init];
    fadeIn.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeIn.startTime = 0;
    fadeIn.endTime = lastTime;
    fadeIn.startValue = 0;
    fadeIn.endValue = 1.0;
    
    QHVCEffectVideoTransferParam* fadeOut = [[QHVCEffectVideoTransferParam alloc] init];
    fadeOut.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeOut.startTime = duration - lastTime;
    fadeOut.endTime = duration;
    fadeOut.startValue = 1.0;
    fadeOut.endValue = 0;
    
    NSArray* transfers = @[jumpIn, jumpOut, scaleIn, scaleOut, fadeIn, fadeOut];
    self.effectImage.videoTransfer = transfers;
    [[QHVCEditMediaEditor sharedInstance] updateStickerEffect:self.effectImage];
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)addAnimationRotateInOut
{
    NSTimeInterval duration = self.effectImage.endTime - self.effectImage.startTime;
    NSTimeInterval lastTime = MIN(500, duration/2.0);
    QHVCEffectVideoTransferParam* rotateIn = [[QHVCEffectVideoTransferParam alloc] init];
    rotateIn.transferType = QHVCEffectVideoTransferTypeRadian;
    rotateIn.startTime = 0;
    rotateIn.endTime = lastTime;
    rotateIn.startValue = 30.0/180.0*M_PI;
    rotateIn.endValue = 0;
    
    QHVCEffectVideoTransferParam* rotateOut = [[QHVCEffectVideoTransferParam alloc] init];
    rotateOut.transferType = QHVCEffectVideoTransferTypeRadian;
    rotateOut.startTime = duration - lastTime;
    rotateOut.endTime = duration;
    rotateOut.startValue = 0;
    rotateOut.endValue = -30./180.0*M_PI;
    
    QHVCEffectVideoTransferParam* scaleIn = [[QHVCEffectVideoTransferParam alloc] init];
    scaleIn.transferType = QHVCEffectVideoTransferTypeScale;
    scaleIn.startTime = 0;
    scaleIn.endTime = lastTime;
    scaleIn.startValue = 1.2;
    scaleIn.endValue = 1.0;
    
    QHVCEffectVideoTransferParam* scaleOut = [[QHVCEffectVideoTransferParam alloc] init];
    scaleOut.transferType = QHVCEffectVideoTransferTypeScale;
    scaleOut.startTime = duration - lastTime;
    scaleOut.endTime = duration;
    scaleOut.startValue = 1.0;
    scaleOut.endValue = 1.2;
    
    QHVCEffectVideoTransferParam* fadeIn = [[QHVCEffectVideoTransferParam alloc] init];
    fadeIn.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeIn.startTime = 0;
    fadeIn.endTime = lastTime;
    fadeIn.startValue = 0;
    fadeIn.endValue = 1.0;
    
    QHVCEffectVideoTransferParam* fadeOut = [[QHVCEffectVideoTransferParam alloc] init];
    fadeOut.transferType = QHVCEffectVideoTransferTypeAlpha;
    fadeOut.startTime = duration - lastTime;
    fadeOut.endTime = duration;
    fadeOut.startValue = 1.0;
    fadeOut.endValue = 0;
    
    NSArray* transfers = @[rotateIn, rotateOut, scaleIn, scaleOut, fadeIn, fadeOut];
    self.effectImage.videoTransfer = transfers;
    [[QHVCEditMediaEditor sharedInstance] updateStickerEffect:self.effectImage];
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

#pragma mark - Event Methods

- (void)deleteAction:(UIButton *)sender
{
    [self removeFromSuperview];
    [[QHVCEditMediaEditor sharedInstance] deleteStickerEffect:self.effectImage];
    [[[QHVCEditMediaEditorConfig sharedInstance] stickerItemArray] removeObject:self];
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
    [[NSNotificationCenter defaultCenter] postNotificationName:QHVCEDIT_DEFINE_NOTIFY_SHOW_STICKER_FUNCTION object:self];
}

@end
