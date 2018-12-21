//
//  QHVCEditAudioItem.h
//  QHVideoCloudToolSet
//
//  Created by deng on 2018/1/24.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface QHVCEditAudioItem : NSObject

@property (nonatomic, strong) NSString *audiofile;
@property (nonatomic, assign) NSTimeInterval audioDuration;
@property (nonatomic, assign) NSTimeInterval startTime;
@property (nonatomic, assign) NSTimeInterval endTime;
@property (nonatomic, assign) NSTimeInterval insertstartTime;
@property (nonatomic, assign) NSTimeInterval insertendTime;
@property (nonatomic, assign) float volume;

@end
