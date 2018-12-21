//
//  QHVCEditFilterView.m
//  QHVideoCloudToolSet
//
//  Created by yinchaoyu on 2018/6/29.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditFilterView.h"
#import "QHVCEditPrefs.h"
#import "QHVCEditFilterCell.h"
#import "QHVCEditMediaEditor.h"
#import "QHVCEditMediaEditorConfig.h"

static NSString * const filterCellIdentifier = @"QHVCEditFilterCell";

@interface  QHVCEditFilterView()
{
    __weak IBOutlet UICollectionView *_filterCollectionView;
    NSInteger _selectedIndex;
}

@property (nonatomic, strong) NSArray<NSDictionary *> *filters;

@end

@implementation QHVCEditFilterView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    [_filterCollectionView registerNib:[UINib nibWithNibName:filterCellIdentifier bundle:nil] forCellWithReuseIdentifier:filterCellIdentifier];
    _selectedIndex = [QHVCEditMediaEditorConfig sharedInstance].filterIndex;
    
    _filters = @[@{@"title":@"原图",@"name":@""},
                 @{@"title":@"滤镜1",@"name":@"lut_1.png"},
                 @{@"title":@"滤镜2",@"name":@"lut_2.png"},
                 @{@"title":@"滤镜3",@"name":@"lut_3.png"},
                 @{@"title":@"滤镜4",@"name":@"lut_4.png"},
                 @{@"title":@"滤镜5",@"name":@"lut_5.png"},
                 @{@"title":@"滤镜6",@"name":@"lut_6.png"},
                 @{@"title":@"滤镜7",@"name":@"lut_7.png"},
                 @{@"title":@"滤镜8",@"name":@"lut_8.png"},
                 @{@"title":@"滤镜9",@"name":@"lut_9.png"},
                 @{@"title":@"滤镜10",@"name":@"lut_10.png"},
                 @{@"title":@"滤镜11",@"name":@"lut_11.png"},
                 @{@"title":@"滤镜12",@"name":@"lut_12.png"},
                 @{@"title":@"滤镜13",@"name":@"lut_13.png"},
                 @{@"title":@"滤镜14",@"name":@"lut_14.png"},
                 @{@"title":@"滤镜15",@"name":@"lut_15.png"},
                 @{@"title":@"滤镜16",@"name":@"lut_16.png"},
                 @{@"title":@"滤镜17",@"name":@"lut_17.png"},
                 ];
}

- (void)confirmAction
{
    SAFE_BLOCK(self.confirmBlock, self);
}

#pragma mark UICollectionViewDataSource

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return _filters.count;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    QHVCEditFilterCell * cell = [collectionView dequeueReusableCellWithReuseIdentifier:filterCellIdentifier forIndexPath:indexPath];
    [cell updateCell:_filters[indexPath.row] filterIndex:indexPath.row];
    return cell;
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath
{
    return CGSizeMake(70, 90);
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout minimumLineSpacingForSectionAtIndex:(NSInteger)section
{
    return 5;
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout minimumInteritemSpacingForSectionAtIndex:(NSInteger)section
{
    return 0;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
    if (_selectedIndex == indexPath.row)
    {
        return;
    }

    [self updateFilter:indexPath.row];
    [_filterCollectionView reloadData];
}

- (void)updateFilter:(NSInteger)index
{
    //更新当前index
    _selectedIndex = index;
    [QHVCEditMediaEditorConfig sharedInstance].filterIndex = index;
    QHVCEditFilterEffect* filter = [[QHVCEditMediaEditorConfig sharedInstance] effectFilter];
    
    //原图，删除特效
    if (index == 0)
    {
        if (!filter)
        {
            return;
        }
        
        [[QHVCEditMediaEditor sharedInstance] deleteEffectFilter:filter];
        [[QHVCEditMediaEditorConfig sharedInstance] setEffectFilter:nil];
        SAFE_BLOCK(self.refreshForEffectBasicParamsBlock);
        return;
    }

    //更新特效
    NSDictionary *item = _filters[index];
    NSString* name = [item objectForKey:@"name"];
    NSString* path = [NSString stringWithFormat:@"%@/%@/%@", [[NSBundle mainBundle] bundlePath], @"Filters", name];
    if (filter)
    {
        //更新
        [filter setFilePath:path];
        [[QHVCEditMediaEditor sharedInstance] updateEffectFilter:filter];
        SAFE_BLOCK(self.refreshPlayerBlock, YES);
    }
    else
    {
        //新增
        QHVCEditFilterEffect* filter = [[QHVCEditMediaEditor sharedInstance] createEffectFilter];
        [filter setFilePath:path];
        [filter setStartTime:0];
        [filter setEndTime:[[QHVCEditMediaEditor sharedInstance] getTimelineDuration]];
        [[QHVCEditMediaEditorConfig sharedInstance] setEffectFilter:filter];
        [[QHVCEditMediaEditor sharedInstance] addEffectFilterToTimeline:filter];
        SAFE_BLOCK(self.refreshForEffectBasicParamsBlock);
    }
}

@end
