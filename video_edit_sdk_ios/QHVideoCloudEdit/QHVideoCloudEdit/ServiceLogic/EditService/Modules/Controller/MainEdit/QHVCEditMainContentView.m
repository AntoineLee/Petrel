//
//  QHVCEditOverlayContentView.m
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/8/7.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditMainContentView.h"
#import "QHVCEditPrefs.h"
#import "QHVCPhotoManager.h"
#import "QHVCEditMediaEditorConfig.h"
#import "QHVCEditMediaEditor.h"

@interface QHVCEditMainContentView ()
@property (nonatomic, retain) QHVCEditPlayerBaseVC* playerBaseVC;

@end

@implementation QHVCEditMainContentView

- (void)layoutSubviews
{
    [super layoutSubviews];
}

- (void)clear
{
    //overlay
    [[[QHVCEditMediaEditorConfig sharedInstance] overlayItemArray] enumerateObjectsUsingBlock:^(QHVCEditOverlayItemView * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
    {
        [[QHVCEditMediaEditor sharedInstance] deleteOverlayClip:obj.clipItem];
        [obj removeFromSuperview];
    }];
    
    //sticker
    [[[QHVCEditMediaEditorConfig sharedInstance] stickerItemArray] enumerateObjectsUsingBlock:^(QHVCEditStickerItemView * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
    {
        [[QHVCEditMediaEditor sharedInstance] deleteStickerEffect:obj.effectImage];
        [obj removeFromSuperview];
    }];
    
    //subtitle
    [[[QHVCEditMediaEditorConfig sharedInstance] subtitleItemArray] enumerateObjectsUsingBlock:^(QHVCEditSubtitleItemView * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
    {
        [[QHVCEditMediaEditor sharedInstance] deleteStickerEffect:obj.effect];
        [obj removeFromSuperview];
    }];
}

- (void)setBasePlayerVC:(QHVCEditPlayerBaseVC *)playerBaseVC
{
    _playerBaseVC = playerBaseVC;
}

#pragma mark - 画中画

- (void)addOverlays:(NSArray<QHVCPhotoItem *> *)items complete:(void(^)(void))complete
{
    WEAK_SELF
    [[QHVCPhotoManager manager] writeAssetsToSandbox:items complete:^{
        [items enumerateObjectsUsingBlock:^(QHVCPhotoItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
         {
             STRONG_SELF
             CGRect rect = [self createRandomRect:kDefaultOverlayWidth targetHeight:kDefaultOverlayHeight
                                      sourceWidth:obj.assetWidth sourceHeight:obj.assetHeight];
             QHVCEditOverlayItemView* overlayItemView = [[QHVCEditOverlayItemView alloc] initWithFrame:rect];
             [self addSubview:overlayItemView];
             [overlayItemView setPhotoItem:obj];
             [[[QHVCEditMediaEditorConfig sharedInstance] overlayItemArray] addObject:overlayItemView];
             SAFE_BLOCK(self.resetPlayerAction);
             SAFE_BLOCK(complete);
             
             [overlayItemView setResetPlayerBlock:^{
                 SAFE_BLOCK(self.resetPlayerAction);
             }];
             
             [overlayItemView setRefreshPlayerBlock:^(BOOL forceRefresh)
             {
                 SAFE_BLOCK(self.refreshPlayerAction, forceRefresh);
             }];
             
             [overlayItemView setTapAction:^(QHVCEditOverlayItemView *item)
             {
                 NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
                 [center postNotificationName:QHVCEDIT_DEFINE_NOTIFY_SHOW_OVERLAY_FUNCTION object:item];
             }];
         }];
    }];
}

#pragma mark - 贴纸

- (QHVCEditStickerItemView *)addSticker:(UIImage *)image
{
    CGRect rect = [self createRandomRect:kDefaultStickerWidth targetHeight:kDefaultStickerHeight
                             sourceWidth:kDefaultStickerWidth sourceHeight:kDefaultStickerHeight];
    QHVCEditStickerItemView* stickerItemView = [[QHVCEditStickerItemView alloc] initWithFrame:rect];
    [stickerItemView setImage:image];
    [stickerItemView setPlayerBaseVC:self.playerBaseVC];
    [self addSubview:stickerItemView];
    [[[QHVCEditMediaEditorConfig sharedInstance] stickerItemArray] addObject:stickerItemView];
    
    WEAK_SELF
    [stickerItemView setRefreshPlayerForBasicParamBlock:^{
        STRONG_SELF
        SAFE_BLOCK(self.refreshPlayerForBasicParamAction);
    }];
    
    [stickerItemView setRefreshPlayerBlock:^(BOOL forceRefresh)
    {
        STRONG_SELF
        SAFE_BLOCK(self.refreshPlayerAction, forceRefresh);
    }];
    
    return stickerItemView;
}

#pragma mark - 字幕

- (QHVCEditSubtitleItemView *)addSubtitle:(UIImage *)image
{
    CGRect rect = [self createRandomRect:kDefaultSubtitleWidth targetHeight:kDefaultSubtitleHeight
                             sourceWidth:kDefaultSubtitleWidth sourceHeight:kDefaultSubtitleHeight];
    QHVCEditSubtitleItemView* subtitleItemView = [[QHVCEditSubtitleItemView alloc] initWithFrame:rect];
    [subtitleItemView setImage:image];
    [self addSubview:subtitleItemView];
    [[[QHVCEditMediaEditorConfig sharedInstance] subtitleItemArray] addObject:subtitleItemView];
    
    WEAK_SELF
    [subtitleItemView setRefreshPlayerForBasicParamBlock:^{
        STRONG_SELF
        SAFE_BLOCK(self.refreshPlayerForBasicParamAction);
    }];
    
    [subtitleItemView setRefreshPlayerBlock:^(BOOL forceRefresh)
     {
         STRONG_SELF
         SAFE_BLOCK(self.refreshPlayerAction, forceRefresh);
     }];
    
    return subtitleItemView;
}

#pragma mark - Private Methods

- (CGRect)createRandomRect:(NSInteger)targetWidth
              targetHeight:(NSInteger)targetHeight
               sourceWidth:(NSInteger)sourceWidth
              sourceHeight:(NSInteger)sourceHeight
{
    CGRect rect = CGRectZero;
    CGFloat x = [self getRandomNumber:0 to:fabs((CGRectGetWidth(self.frame) - targetWidth))];
    CGFloat y = [self getRandomNumber:0 to:fabs((CGRectGetHeight(self.frame) - targetHeight))];
    CGFloat scaleW = (CGFloat)targetWidth/sourceWidth;
    CGFloat scaleH = (CGFloat)targetHeight/sourceHeight;
    CGFloat scale = MIN(scaleW, scaleH);
    int w = sourceWidth*scale;
    int h = sourceHeight*scale;
    rect.origin.x = x;
    rect.origin.y = y;
    rect.size.width = w;
    rect.size.height = h;
    return rect;
}

- (int)getRandomNumber:(int)from to:(int)to
{
    return (int)(from + (arc4random() % (to - from + 1)));
}

@end
