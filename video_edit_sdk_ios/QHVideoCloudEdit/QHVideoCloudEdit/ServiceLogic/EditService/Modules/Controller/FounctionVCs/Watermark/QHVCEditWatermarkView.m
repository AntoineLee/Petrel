//
//  QHVCEditWatermarkView.m
//  QHVideoCloudToolSet
//
//  Created by yinchaoyu on 2018/7/12.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditWatermarkView.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditMediaEditor.h"
#import "QHVCEditMediaEditorConfig.h"

@implementation QHVCEditWatermarkView
{
    BOOL _isWatermarkOpen;
    IBOutlet UIButton *_watermarkBtn;
}

- (void)prepareSubviews
{
    _isWatermarkOpen = [[QHVCEditMediaEditorConfig sharedInstance] effectWaterMark] ? NO:YES;
    [self onWatermarkClick:nil];
}

- (void)confirmAction
{
    SAFE_BLOCK(self.confirmBlock, self);
}

- (IBAction)onWatermarkClick:(id)sender
{
    _isWatermarkOpen = !_isWatermarkOpen;
    if (_isWatermarkOpen)
    {
        [_watermarkBtn setTitle:@"关闭水印" forState:UIControlStateNormal];
    }
    else
    {
        [_watermarkBtn setTitle:@"开启水印" forState:UIControlStateNormal];
    }
    
    [self updateWatermarkEffect];
}

- (void)updateWatermarkEffect
{
    if (_isWatermarkOpen)
    {
        QHVCEditStickerEffect* watermarkEffect = [[QHVCEditMediaEditorConfig sharedInstance] effectWaterMark];
        if (!watermarkEffect)
        {
            UIImage* image = [UIImage imageNamed:@"edit_watermask"];
            watermarkEffect = [[QHVCEditMediaEditor sharedInstance] createWatermarkEffect];
            watermarkEffect.startTime = 0;
            watermarkEffect.endTime = [[QHVCEditMediaEditor sharedInstance] getTimelineDuration];
            watermarkEffect.sticker = image;
            watermarkEffect.renderWidth = 200;
            watermarkEffect.renderHeight = 30;
            
            [[QHVCEditMediaEditorConfig sharedInstance] setEffectWaterMark:watermarkEffect];
            [[QHVCEditMediaEditor sharedInstance] addWatermarkEffectToTimeline:watermarkEffect];
            SAFE_BLOCK(self.refreshForEffectBasicParamsBlock);
        }
    }
    else
    {
        QHVCEditStickerEffect* watermarkEffect = [[QHVCEditMediaEditorConfig sharedInstance] effectWaterMark];
        if (watermarkEffect)
        {
            [[QHVCEditMediaEditor sharedInstance] deleteWatermarkEffect:watermarkEffect];
            [[QHVCEditMediaEditorConfig sharedInstance] setEffectWaterMark:nil];
            SAFE_BLOCK(self.refreshForEffectBasicParamsBlock);
        }
    }
}


@end
