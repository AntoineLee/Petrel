//
//  QHVCEditCIFilterStack.m
//  QHVCEditKit
//
//  Created by liyue-g on 2017/11/21.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import "QHVCEditCIFilterStack.h"
#import "QHVCEditConfig.h"
#import "QHVCEditLogger.h"
#import <QHVCEffectKit/QHVCEffectBase+Process.h>

@interface QHVCEditCIFilterStack ()
@property (nonatomic, weak) id<QHVCEditCIOutputProtocol> output;
@property (atomic,  retain) NSMutableArray* filterArray;

@end

@implementation QHVCEditCIFilterStack

- (void)dealloc
{
}

- (instancetype)initWithOutput:(id<QHVCEditCIOutputProtocol>)output
{
    self = [super init];
    if (self)
    {
        self.output = output;
        self.filterArray = [[NSMutableArray alloc] initWithCapacity:0];
    }
    
    return self;
}

- (void)addFilter:(QHVCEffectBase *)filter
{
    if (!filter)
    {
        return;
    }
    
    [self.filterArray addObject:filter];
}

- (void)addFilter:(QHVCEffectBase *)filter atIndex:(NSUInteger)index
{
    if (!filter)
    {
        return;
    }
    
    if ([self.filterArray count] >= index)
    {
        [self.filterArray insertObject:filter atIndex:index];
    }
    else
    {
        [self addFilter:filter];
    }
}

- (void)removeFilter:(QHVCEffectBase *)filter
{
    if (!filter)
    {
        return;
    }
    
    [self.filterArray removeObject:filter];
}

- (void)removeFilterAtIndex:(NSUInteger)index
{
    if ([self.filterArray count] > index)
    {
        [self.filterArray removeObjectAtIndex:index];
    }
}

- (void)removeAllFilters
{
    [self.filterArray removeAllObjects];
}

- (void)processImage:(CIImage *)image timestamp:(NSInteger)timestamp
{
    if ([self.filterArray count] <= 0)
    {
        [self.output processImage:nil timestampMs:timestamp userData:nil];
        return;
    }
    
    if (!image)
    {
        [self.output processImage:nil timestampMs:timestamp userData:nil];
        return;
    }
    
    __block CIImage* outImage = image;
    [self.filterArray enumerateObjectsUsingBlock:^(QHVCEffectBase* filter, NSUInteger idx, BOOL * _Nonnull stop)
     {
         outImage = [filter processImage:outImage timestamp:timestamp];
     }];
    
    if (outImage)
    {
        [self.output processImage:outImage timestampMs:timestamp userData:nil];
    }
}

@end
