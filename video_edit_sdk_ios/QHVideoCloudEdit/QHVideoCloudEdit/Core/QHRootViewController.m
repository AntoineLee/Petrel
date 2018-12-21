//
//  QHRootViewController.m
//  QHVideoCloudToolSet
//
//  Created by yangkui on 2017/6/14.
//  Copyright © 2017年 yangkui. All rights reserved.
//

#import "QHRootViewController.h"
#import <UIKit/UIKit.h>
#import "QHVCTabTableViewCell.h"
#import "QHVCConfig.h"
#import "QHVCEditVC.h"

static NSString *const itemCellIdentifier = @"QHRootCollectionCell";

@interface QHRootViewController ()<UITableViewDelegate, UITableViewDataSource>
{
    UIView *_headerView;
}

@property (nonatomic, strong) UITableView *tableView;
@property (nonatomic, strong) NSArray *dataSource;

@end

@implementation QHRootViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"app_bg"]];
    
    [self initHeaderView];
    [self initTableView];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [self.navigationController setNavigationBarHidden:YES animated:animated];
}

- (UILabel *)labelWithTitle:(NSString *)title fontName:(NSString *)fontName fontSize:(NSInteger)fontSize
{
    UILabel *label = [UILabel new];
    label.text = title;
    label.textColor = [UIColor whiteColor];
    label.textAlignment = NSTextAlignmentCenter;
    label.font = [UIFont fontWithName:fontName size:fontSize];
    
    return label;
}

- (void)initTableView
{
    _tableView = [UITableView new];
    [self.view addSubview:self.tableView];
    [_tableView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.right.bottom.equalTo(self.view);
        make.top.equalTo(_headerView.mas_bottom);
    }];
}

- (UITableView *)tableView
{
    _tableView = [[UITableView alloc] initWithFrame:CGRectZero];
    _tableView.tableFooterView = [UIView new];
    _tableView.backgroundColor = [UIColor whiteColor];
    _tableView.delegate = self;
    _tableView.dataSource = self;
    [_tableView registerClass: [QHVCTabTableViewCell class]
       forCellReuseIdentifier:itemCellIdentifier];
    
    return _tableView;
}

#pragma mark TableViewDelegate
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 50;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return self.dataSource.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    QHVCTabTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:itemCellIdentifier];
    [cell updateCellDetail:_dataSource[indexPath.row]];
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UIViewController *viewController = [QHVCEditVC new];
    [self.navigationController pushViewController:viewController animated:YES];
}

-(CGFloat)tableView:(UITableView*)tableView heightForFooterInSection:(NSInteger)section
{
    return 2.0;
}

- (NSArray *)dataSource
{
    if (!_dataSource)
    {
        _dataSource = @[
                        @{
                            @"leftImage":@"tab_videoEdit",
                            @"title":@"剪辑",
                            @"rightImage":@"jiantou"
                            }
                        ];
    }
    
    return _dataSource;
}

- (void)initHeaderView
{
    _headerView = [UIView new];
    [self.view addSubview:_headerView];
    [_headerView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.right.equalTo(self.view);
        make.top.equalTo(@20);
        make.height.equalTo(@270);
    }];
    
    UIImageView *imgView = [UIImageView new];
    imgView.image = [UIImage imageNamed:@"logo"];
    [_headerView addSubview:imgView];
    [imgView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(_headerView);
        make.top.equalTo(@53);
        make.height.equalTo(@100);
        make.width.equalTo(@100);
    }];
    
    UILabel *titleLabel = [self labelWithTitle:@"奇影 DEMO" fontName:kBoldFontName fontSize:25];
    [_headerView addSubview:titleLabel];
    [titleLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(_headerView);
        make.top.equalTo(imgView.mas_bottom).offset(18);
        make.width.equalTo(@210);
        make.height.equalTo(@25);
    }];
    
    UILabel *descriptionOne = [self labelWithTitle:@"本demo以最简单的代码展示360短视频sdk的" fontName:kDefaultFontName fontSize:14];
    [_headerView addSubview:descriptionOne];
    [descriptionOne mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(_headerView);
        make.top.equalTo(titleLabel.mas_bottom).offset(18);
        make.width.equalTo(@300);
        make.height.equalTo(@20);
    }];
    
    UILabel *descriptionTwo = [self labelWithTitle:@"使用方法，各功能之间相互独立，可单独使用。" fontName:kDefaultFontName fontSize:14];
    [_headerView addSubview:descriptionTwo];
    [descriptionTwo mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(_headerView);
        make.top.equalTo(descriptionOne.mas_bottom);
        make.width.equalTo(@300);
        make.height.equalTo(@20);
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
