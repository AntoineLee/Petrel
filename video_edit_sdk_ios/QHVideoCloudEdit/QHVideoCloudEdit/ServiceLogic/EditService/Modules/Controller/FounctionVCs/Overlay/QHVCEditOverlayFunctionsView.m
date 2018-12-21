//
//  QHVCEditOverlayFunctionsView.m
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/8/10.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditOverlayFunctionsView.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditMainFunctionCell.h"
#import "QHVCEditMediaEditor.h"
#import "QHVCEditMediaEditorConfig.h"
#import "QHVCEditOverlaySpeedView.h"
#import "QHVCEditOverlayVolumeView.h"

static NSString* overlayFunctionCellIdentifier = @"QHVCEditOverlayFunctionCell";

@interface QHVCEditOverlayFunctionsView () <UICollectionViewDelegate, UICollectionViewDataSource>

@property (weak,   nonatomic) IBOutlet UICollectionView *collectionView;
@property (nonatomic, retain) QHVCEditVideoTransferEffect* videoTransferEffect;

@end

@implementation QHVCEditOverlayFunctionsView

- (void)confirmAction
{
    [self.clipItemView hideBorder:YES];
    SAFE_BLOCK(self.confirmBlock, self);
}

- (void)prepareSubviews
{
    [super prepareSubviews];
    [self.collectionView registerNib:[UINib nibWithNibName:@"QHVCEditMainFunctionCell" bundle:nil] forCellWithReuseIdentifier:overlayFunctionCellIdentifier];
    [self.collectionView setDataSource:self];
    [self.collectionView setDelegate:self];
    
    self.overlayFuncsArray = @[
                               @[@"旋转", @"edit_overlay_rotate"],
                               @[@"上下镜像", @"edit_overlay_flipy"],
                               @[@"左右镜像", @"edit_overlay_flipx"],
                               @[@"变速", @"edit_overlay_speed"],
                               @[@"音量", @"edit_overlay_volume"],
                               @[@"删除", @"edit_overlay_delete"],
                               @[@"淡入淡出", @"edit_overlay_fade"],
                               @[@"滑入滑出", @"edit_overlay_fade"],
                               @[@"弹入弹出", @"edit_overlay_fade"],
                               @[@"旋入旋出", @"edit_overlay_fade"]];
    
    NSArray<QHVCEditEffect *>* clipEffects = [self.clipItemView.clipItem.clip getEffects];
    self.videoTransferEffect = (QHVCEditVideoTransferEffect *)[self getEffectWithClassName:[[QHVCEditVideoTransferEffect class] description] fromEffects:clipEffects];
    [self.clipItemView hideBorder:NO];
}

- (NSInteger)collectionView:(nonnull UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return [self.overlayFuncsArray count];
}

- (nonnull __kindof UICollectionViewCell *)collectionView:(nonnull UICollectionView *)collectionView cellForItemAtIndexPath:(nonnull NSIndexPath *)indexPath
{
    QHVCEditMainFunctionCell* cell = [collectionView dequeueReusableCellWithReuseIdentifier:overlayFunctionCellIdentifier forIndexPath:indexPath];
    NSArray* item = self.overlayFuncsArray[indexPath.row];
    if (item.count > 1)
    {
        [cell updateCell:item];
    }
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
    [self overlayFunctionDidSelected:indexPath.row];
}

- (void)overlayFunctionDidSelected:(NSInteger)index
{
    SAFE_BLOCK(self.pausePlayerBlock);
    switch (index)
    {
        case 0:
        {
            //旋转
            [self clickedRotateItem];
            break;
        }
        case 1:
        {
            //上下镜像
            [self clickedFlipYItem];
            break;
        }
        case 2:
        {
            //左右镜像
            [self clickedFlipXItem];
            break;
        }
        case 3:
        {
            //变速
            [self clickedSpeedItem];
            break;
        }
        case 4:
        {
            //音量
            [self clickedVolumeItem];
            break;
        }
        case 5:
        {
            //删除
            [self clickedDeleteItem];
            break;
        }
        case 6:
        {
            //淡入淡出
            [self clickedFadeInOutItem];
            break;
        }
        case 7:
        {
            //滑入滑出
            [self clickedMoveInOutItem];
            break;
        }
        case 8:
        {
            //弹入弹出
            [self clickedJumpInOutItem];
            break;
        }
        case 9:
        {
            //旋入旋出
            [self clickedRotateInOutItem];
            break;
        }
        default:
            break;
    }
}

- (void)clickedRotateItem
{
    CGFloat radian = self.clipItemView.clipItem.clip.sourceRadian;
    radian += M_PI_2;
    if (radian > M_PI * 2)
    {
        radian += M_PI * 2;
    }
    
    [self.clipItemView.clipItem.clip setSourceRadian:radian];
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)clickedFlipYItem
{
    BOOL flipY = self.clipItemView.clipItem.clip.flipY;
    flipY = !flipY;
    [self.clipItemView.clipItem.clip setFlipY:flipY];
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)clickedFlipXItem
{
    BOOL flipX = self.clipItemView.clipItem.clip.flipX;
    flipX = !flipX;
    [self.clipItemView.clipItem.clip setFlipX:flipX];
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)clickedSpeedItem
{
    QHVCEditOverlaySpeedView* view = [[NSBundle mainBundle] loadNibNamed:[[QHVCEditOverlaySpeedView class] description]
                                                                   owner:self
                                                                 options:nil][0];
    [view setSpeed:self.clipItemView.clipItem.clip.speed];
    [self addSubview:view];
    
    WEAK_SELF
    [view setChangeSpeedAction:^(CGFloat speed)
    {
        STRONG_SELF
        [self.clipItemView.clipItem.clip setSpeed:speed];
        [[QHVCEditMediaEditor sharedInstance] updateOverlayClipParams:self.clipItemView.clipItem];
        SAFE_BLOCK(self.resetPlayerBlock);
        SAFE_BLOCK(self.updatePlayerDuraionBlock);
    }];
}

- (void)clickedVolumeItem
{
    QHVCEditOverlayVolumeView* view = [[NSBundle mainBundle] loadNibNamed:[[QHVCEditOverlayVolumeView class] description]
                                                                    owner:self
                                                                  options:nil][0];
    [view setVolume:self.clipItemView.clipItem.clip.volume];
    [self addSubview:view];
    
    WEAK_SELF
    [view setChangeVolumeAction:^(NSInteger volume)
    {
        STRONG_SELF
        [self.clipItemView.clipItem.clip setVolume:volume];
        [[QHVCEditMediaEditor sharedInstance] updateOverlayClipParams:self.clipItemView.clipItem];
        SAFE_BLOCK(self.resetPlayerBlock);
    }];
}

- (void)clickedDeleteItem
{
    [self.clipItemView removeFromSuperview];
    [[QHVCEditMediaEditor sharedInstance] deleteOverlayClip:self.clipItemView.clipItem];
    [[[QHVCEditMediaEditorConfig sharedInstance] overlayItemArray] removeObject:self.clipItemView];
    SAFE_BLOCK(self.resetPlayerBlock);
    SAFE_BLOCK(self.updatePlayerDuraionBlock);
    [self confirmAction];
}

- (void)clickedFadeInOutItem
{
    NSTimeInterval duration = self.clipItemView.clipItem.clip.fileEndTime - self.clipItemView.clipItem.clip.fileStartTime;
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
    self.videoTransferEffect.videoTransfer = transfers;
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)clickedMoveInOutItem
{
    NSTimeInterval duration = self.clipItemView.clipItem.clip.fileEndTime - self.clipItemView.clipItem.clip.fileStartTime;
    NSTimeInterval lastTime = MIN(500, duration/2.0);
    QHVCEffectVideoTransferParam* moveIn = [[QHVCEffectVideoTransferParam alloc] init];
    moveIn.transferType = QHVCEffectVideoTransferTypeOffsetX;
    moveIn.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    moveIn.startTime = 0;
    moveIn.endTime = lastTime;
    moveIn.startValue = -kOutputVideoWidth/5.0;
    moveIn.endValue = 0;
    
    QHVCEffectVideoTransferParam* moveOut = [[QHVCEffectVideoTransferParam alloc] init];
    moveOut.transferType = QHVCEffectVideoTransferTypeOffsetX;
    moveOut.curveType = QHVCEffectVideoTransferCurveTypeCurve;
    moveOut.startTime = duration - lastTime;
    moveOut.endTime = duration;
    moveOut.startValue = 0;
    moveOut.endValue = kOutputVideoWidth/5.0;
    
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
    self.videoTransferEffect.videoTransfer = transfers;
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)clickedJumpInOutItem
{
    NSTimeInterval duration = self.clipItemView.clipItem.clip.fileEndTime - self.clipItemView.clipItem.clip.fileStartTime;
    NSTimeInterval lastTime = MIN(500, duration/2.0);

    QHVCEffectVideoTransferParam* jumpIn = [[QHVCEffectVideoTransferParam alloc] init];
    jumpIn.transferType = QHVCEffectVideoTransferTypeOffsetY;
    jumpIn.startTime = 0;
    jumpIn.endTime = lastTime;
    jumpIn.startValue = kOutputVideoHeight/5.0;
    jumpIn.endValue = 0;

    QHVCEffectVideoTransferParam* jumpOut = [[QHVCEffectVideoTransferParam alloc] init];
    jumpOut.transferType = QHVCEffectVideoTransferTypeOffsetY;
    jumpOut.startTime = duration - lastTime;
    jumpOut.endTime = duration;
    jumpOut.startValue = 0;
    jumpOut.endValue = -kOutputVideoHeight/5.0;

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
    self.videoTransferEffect.videoTransfer = transfers;
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (void)clickedRotateInOutItem
{
    NSTimeInterval duration = self.clipItemView.clipItem.clip.fileEndTime - self.clipItemView.clipItem.clip.fileStartTime;
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
    self.videoTransferEffect.videoTransfer = transfers;
    SAFE_BLOCK(self.refreshPlayerBlock, YES);
}

- (QHVCEditVideoTransferEffect *)videoTransferEffect
{
    if (!_videoTransferEffect)
    {
        QHVCEditVideoTransferEffect* effect = [[QHVCEditMediaEditor sharedInstance] createEffectVideoAnimation];
        effect.startTime = 0;
        effect.endTime = self.clipItemView.clipItem.clip.fileEndTime - self.clipItemView.clipItem.clip.fileStartTime;
        _videoTransferEffect = effect;
        [[QHVCEditMediaEditor sharedInstance] overlayClip:self.clipItemView.clipItem addVideoAnimation:effect];
        SAFE_BLOCK(self.refreshForEffectBasicParamsBlock);
    }
    return _videoTransferEffect;
}


- (QHVCEditEffect *)getEffectWithClassName:(NSString *)className fromEffects:(NSArray<QHVCEditEffect *>*)effects
{
    __block QHVCEditEffect* effect = nil;
    [effects enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
     {
         if ([[[obj class] description] isEqualToString:className])
         {
             effect = obj;
             *stop = YES;
         }
     }];
    return effect;
}

@end
