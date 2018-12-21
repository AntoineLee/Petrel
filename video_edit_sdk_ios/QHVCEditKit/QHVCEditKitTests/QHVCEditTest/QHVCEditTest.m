//
//  QHVCEditTest.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/5.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEditTest.h"
#import "QHVCEditTestMacroDefs.h"
#import "QHVCEditConfig.h"
#import "QHVCEditTestHelper.h"
#import "QHVCEditTestEditor.h"
#import "QHVCEditTestPlayer.h"
#import "QHVCEditTestRender.h"
#import "QHVCEditTestProducer.h"
#import "QHVCEditTestThumbnail.h"

int QHVCEditTestAll(void)
{
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestConfig());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestHelper());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestEditor());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestPlayer());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestRender());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestProducer());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestThumbnailAll());
    
    return 0;
}

#pragma mark - Config

int QHVCEditTestConfig(void)
{
    
    QHVCEditConfig* config = [QHVCEditConfig sharedInstance];
    
    NSString* bundlePath = [config getBundlePath];
    QHVCEDIT_TEST_OBJECT(bundlePath);
    
    NSString* cacheDirectory = [config cacheDirectory];
    QHVCEDIT_TEST_OBJECT(cacheDirectory);
    
    [config getTimelineIndex];
    
    for (int i = -999; i <= -785; i++)
    {
        NSString* errInfo = [config getErrorCodeInfo:i];
        QHVCEDIT_TEST_OBJECT(errInfo);
    }
    
    NSString* effectBasicInfo = [config getEffectBasicInfo:nil];
    QHVCEDIT_TEST_OBJECT(effectBasicInfo);
    
    NSString* effectDetail = [config printEffectDetail:nil];
    QHVCEDIT_TEST_OBJECT(effectDetail);
    
    EAGLContext* context = [config sharedImageProcessingContext];
    QHVCEDIT_TEST_OBJECT(context);
    
    return 0;
}

#pragma mark - Helper

int QHVCEditTestHelper(void)
{
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestHelperLogger());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestHelperUtils());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestHelperTools());
    return 0;
}

#pragma mark - Editor

int QHVCEditTestEditor(void)
{
    return QHVCEditTestEditorAll();
}

#pragma mark - Player

int QHVCEditTestPlayer(void)
{
    return QHVCEditTestPlayerAll();
}

#pragma mark - Render

int QHVCEditTestRender(void)
{
    return QHVCEditTestRenderAll();
}

#pragma mark - Producer

int QHVCEditTestProducer(void)
{
    return QHVCEditTestProducerAll();
}

#pragma mark - Thumbnail

int QHVCEditTestThumbnail(void)
{
    return QHVCEditTestThumbnailAll();
}
