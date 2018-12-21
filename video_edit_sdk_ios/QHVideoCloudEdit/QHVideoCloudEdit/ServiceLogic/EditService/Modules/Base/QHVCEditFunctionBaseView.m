//
//  QHVCEditFuctionBaseView.m
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/6/27.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditFunctionBaseView.h"
#import "QHVCEditMainContentView.h"
#import "QHVCEditPrefs.h"

@interface QHVCEditFunctionBaseView ()
{
    BOOL _alreadyLayoutSubviews;
}

@property (nonatomic, strong) UIButton* confirmBtn;

@end

@implementation QHVCEditFunctionBaseView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    _confirmBtn = [[UIButton alloc] initWithFrame:CGRectMake(kScreenWidth - 50, 0, 50, 30)];
    [_confirmBtn.titleLabel setFont:[UIFont systemFontOfSize:13]];
    [_confirmBtn setTitle:@"确定" forState:UIControlStateNormal];
    [_confirmBtn setTintColor:[UIColor whiteColor]];
    [_confirmBtn addTarget:self action:@selector(confirmAction) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:_confirmBtn];
}

- (void)layoutSubviews
{
    if (!_alreadyLayoutSubviews)
    {
        [self prepareSubviews];
        _alreadyLayoutSubviews = YES;
    }
}

- (void)prepareSubviews
{
    //override
}

- (IBAction)clickedConfirmBtn:(id)sender
{
    [self confirmAction];
}

- (void)confirmAction
{
    //override
}

- (void)setConfirmButtionState:(BOOL)hidden
{
    [_confirmBtn setHidden:hidden];
}

@end
