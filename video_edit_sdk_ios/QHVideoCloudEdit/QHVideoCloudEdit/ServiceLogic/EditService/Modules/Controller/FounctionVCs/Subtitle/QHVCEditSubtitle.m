//
//  QHVCEditSubtitle.m
//  QHVideoCloudToolSet
//
//  Created by yinchaoyu on 2018/7/4.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditSubtitle.h"
#import "QHVCEditSubtitleView.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditFrameView.h"
#import "QHVCEditSubtitleItem.h"
#import "QHVCEditMediaEditor.h"
#import "UIViewAdditions.h"

@interface QHVCEditSubtitle ()
{
    QHVCEditSubtitleView *_subtitleView;
    QHVCEditFrameView *_frameView;
    QHVCEditSubtitleItem *_currentSubtitleItem;
    NSArray *_colorsArray;
    BOOL _hasChange;
    NSTimeInterval _duration;
    QHVCEditPlayerBaseVC *_vc;
}
@property (nonatomic, strong) NSMutableArray<NSMutableArray *> *subtitleInfos;

@end

@implementation QHVCEditSubtitle

- (instancetype)init
{
    if (self = [super init]) {
        [self initParams];
    }
    
    return self;
}

- (void)initParams
{
    _hasChange = NO;
//    _subtitleInfos = [NSMutableArray arrayWithArray:[QHVCEditPrefs sharedPrefs].subtitleTimestamp];

    _colorsArray = @[@[@"edit_color_black",@"000000"],@[@"edit_color_blue",@"125FDF"],
                     @[@"edit_color_gray",@"888888"],@[@"edit_color_green",@"25B727"],@[@"edit_color_pink",@"FE8AB1"],
                     @[@"edit_color_red",@"F54343"],@[@"edit_color_white",@"FFFFFF"],@[@"edit_color_yellow",@"FFDB4F"]];
    _duration = [[QHVCEditMediaEditor sharedInstance] getTimelineDuration];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onKeyboardNotification:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onKeyboardNotification:) name:UIKeyboardWillHideNotification object:nil];
}

- (void)subtitleAction:(QHVCEditFrameView *)view withVC:(QHVCEditPlayerBaseVC *)vc
{
    _vc = vc;
    _frameView = view;
//    _frameView.duration = _duration;
    
//    _frameView.timeStamp = [QHVCEditPrefs sharedPrefs].subtitleTimestamp;

//    __weak typeof(self) weakSelf = self;
//    _frameView.addCompletion = ^(NSInteger insertStartMs) {
//        [weakSelf handleAddAction:insertStartMs];
//    };
//    _frameView.doneCompletion = ^(NSInteger insertEndMs) {
//        [weakSelf handleDoneAction:insertEndMs];
//    };
//    _frameView.editCompletion = ^{
//        [weakSelf handleEditAction];
//    };
}

- (void)handleAddAction:(NSTimeInterval)insertStartMs
{
    _currentSubtitleItem = [[QHVCEditSubtitleItem alloc] init];
    _currentSubtitleItem.insertstartTime = insertStartMs;

    _subtitleView = [[NSBundle mainBundle] loadNibNamed:[[QHVCEditSubtitleView class] description] owner:self options:nil][0];
    _subtitleView.frame = CGRectMake(0, kScreenHeight - 170, kScreenWidth, 170);
    _subtitleView.subtitleItem = _currentSubtitleItem;
//    _subtitleView.colorsArray = _colorsArray;
    WEAK_SELF
    _subtitleView.refreshCompletion = ^(QHVCEditSubtitleItem *item) {
        STRONG_SELF
        [self handleRefreshSticker:item];
    };
    [_vc.view addSubview:_subtitleView];
}

- (void)handleRefreshSticker:(QHVCEditSubtitleItem *)item
{
//    if (_sticker) {
//        [self updateSticker:item];
//    }
//    else
//    {
//        [self addSticker:item];
//    }
}

- (void)updateSticker:(QHVCEditSubtitleItem *)item
{
//    _sticker.sticker.image = [UIImage imageNamed:[NSString stringWithFormat:@"%@_%@",kStylesName,@(item.styleIndex)]];
//    [self updateSubtitleText:item];
}

- (void)updateSubtitleText:(QHVCEditSubtitleItem *)item
{
//    _sticker.subtitle.text = item.subtitleText;
//    _sticker.subtitle.font = [UIFont systemFontOfSize:item.fontValue];
//    _sticker.subtitle.textColor = [QHVCEditPrefs colorHex:_colorsArray[item.colorIndex][1]];
}

- (void)addSticker:(QHVCEditSubtitleItem *)item
{
//    CGFloat x = [QHVCEditPrefs randomNum:0 to:_preview.width - 100];
//    CGFloat y = [QHVCEditPrefs randomNum:0 to:_preview.height - 100];
//
//    _sticker = [[QHVCEditStickerIconView alloc] initWithFrame:CGRectMake(x, y, 100, 100)];
//    [self updateSticker:item];
//
//    __weak typeof(self) weakSelf = self;
//    _sticker.deleteCompletion = ^(QHVCEditStickerIconView *sticker) {
//        [weakSelf handleDeleteAction];
//    };
//    [_preview addSubview:_sticker];
}

- (void)handleDeleteAction
{
    [_subtitleView resetView];
}

- (void)handleDoneAction:(NSTimeInterval)insertEndMs
{
//    [self nextAction:nil];
}

- (void)handleEditAction
{
//    [self backAction:nil];
}

- (void)handleDiscardAction
{
    [_subtitleView removeFromSuperview];
    _subtitleView = nil;
}

- (void)onKeyboardNotification:(NSNotification *)notif
{
    NSDictionary *userInfo = notif.userInfo;
    //
    // Get keyboard animation.

    NSNumber *durationValue = userInfo[UIKeyboardAnimationDurationUserInfoKey];
    NSTimeInterval animationDuration = durationValue.doubleValue;

    NSNumber *curveValue = userInfo[UIKeyboardAnimationCurveUserInfoKey];
    UIViewAnimationCurve animationCurve = curveValue.intValue;

    if ([notif.name isEqualToString:UIKeyboardWillShowNotification])
    {
        NSValue *endFrameValue = userInfo[UIKeyboardFrameEndUserInfoKey];
        CGRect keyboardEndFrame = [_vc.view convertRect:endFrameValue.CGRectValue fromView:nil];

        CGFloat offset = kScreenHeight - keyboardEndFrame.origin.y + 100;
        _subtitleView.y = offset;
    }

    else if ([notif.name isEqualToString:UIKeyboardWillHideNotification])
    {
        _subtitleView.y = kScreenHeight - 170;
    }

    // Create animation.
    void (^animations)() = ^() {
        [_vc.view layoutIfNeeded];
    };

    [UIView animateWithDuration:animationDuration
                          delay:0.0
                        options:(animationCurve << 16)
                     animations:animations
                     completion:^(BOOL f){
                     }];
}


- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
