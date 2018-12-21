//
//  QHVCEditTestThumbnail.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/9.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEditTestThumbnail.h"
#import "QHVCEditTestMacroDefs.h"
#import "QHVCEditThumbnail.h"
#import "QHVCEditThumbnailInternal.h"

int QHVCEditTestRequestThumbnailNative()
{
    NSString* path = [[NSBundle mainBundle] pathForResource:@"video1" ofType:@"MOV"];
    QHVCEditThumbnail* thumbnail = [[QHVCEditThumbnail alloc] init];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              timestamp:0
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error){}];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              startTime:0
                                endTime:2000
                                  count:2
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    
    NSString* imagePath = [[NSBundle mainBundle] pathForResource:@"pic1" ofType:@"jpg"];
    [thumbnail requestThumbnailFromFile:imagePath
                                  width:100
                                 height:100
                              timestamp:0
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:imagePath
                                  width:100
                                 height:100
                              startTime:0
                                endTime:1000
                                  count:1
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    
    NSString* gifPath = [[NSBundle mainBundle] pathForResource:@"test_gif" ofType:@"gif"];
    [thumbnail requestThumbnailFromFile:gifPath
                                  width:100
                                 height:100
                              startTime:0
                                endTime:1000
                                  count:1
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              timestamp:0
                           dataCallback:nil];
    [thumbnail requestThumbnailFromFile:nil
                                  width:100
                                 height:100
                              timestamp:0
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:path
                                  width:0
                                 height:100
                              timestamp:0
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:0
                              timestamp:0
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              startTime:0
                                endTime:2000
                                  count:2
                           dataCallback:nil];
    [thumbnail requestThumbnailFromFile:nil
                                  width:100
                                 height:100
                              startTime:0
                                endTime:2000
                                  count:2
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:path
                                  width:0
                                 height:100
                              startTime:0
                                endTime:2000
                                  count:2
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:0
                              startTime:0
                                endTime:2000
                                  count:2
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              startTime:2000
                                endTime:0
                                  count:2
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              startTime:0
                                endTime:2000
                                  count:0
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    
    [thumbnail cancelAllThumbnailRequest];
    [thumbnail free];
    
    return 0;
}

int QHVCEditTestRequestVEThumbnail()
{
    NSString* path = [[NSBundle mainBundle] pathForResource:@"a" ofType:@"webm"];
    QHVCEditThumbnail* thumbnail = [[QHVCEditThumbnail alloc] init];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              startTime:0
                                endTime:1000
                                  count:2
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    [thumbnail requestThumbnailFromFile:path
                                  width:100
                                 height:100
                              timestamp:0
                           dataCallback:^(QHVCEditThumbnailItem *thumbnail, QHVCEditError error) {}];
    
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    dispatch_time_t t = dispatch_time(DISPATCH_TIME_NOW, 2*1000*1000*1000);
    dispatch_semaphore_wait(semaphore, t);

    return 0;
}

int QHVCEditTestThumbnailInternal(void)
{
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    QHVCEditThumbnailInternal* internal = [[QHVCEditThumbnailInternal alloc] init];
    [internal rotateImage:image veRotate:0];
    [internal rotateImage:image veRotate:1];
    [internal rotateImage:image veRotate:2];
    [internal rotateImage:image veRotate:3];
    
    [internal imageRotationtoVERotate:image];
    
    return 0;
}

int QHVCEditTestThumbnailAll(void)
{
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestRequestThumbnailNative());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestRequestVEThumbnail());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestThumbnailInternal());
    
    return 0;
}
