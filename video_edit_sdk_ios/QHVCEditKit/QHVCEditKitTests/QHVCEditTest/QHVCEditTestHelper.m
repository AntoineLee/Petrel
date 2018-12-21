//
//  QHVCEditTestHelper.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/5.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEditTestHelper.h"
#import "QHVCEditTestMacroDefs.h"
#import "QHVCEditLogger.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCLifeCycle.h"
#import "QHVCEditCommonDef.h"

#pragma mark - Logger

int QHVCEditTestHelperLogger(void)
{
    [QHVCEditLogger setLogFilePath:NSTemporaryDirectory()];
    [QHVCEditLogger writeLogToLocal:YES];
    [QHVCEditLogger setLogFileParams:1 count:1];
    
    [QHVCEditLogger setSDKLogLevel:QHVCEditLoggerLevelTrace];
    [QHVCEditLogger setSDKLogLevelForFile:QHVCEditLoggerLevelDebug];
    
    [QHVCEditLogger setUserLogLevel:QHVCEditLoggerLevelDebug];
    [QHVCEditLogger setUserLogLevelForFile:QHVCEditLoggerLevelDebug];
    
    [QHVCEditLogger printSDKLog:QHVCEditLoggerLevelDebug prefix:@"[Test]" content:@"test"];
    [QHVCEditLogger printUserLog:QHVCEditLoggerLevelDebug prefix:@"[Test]" content:@"test"];
    
    LogDebug(@"log test debug");
    LogWarn(@"log test warn");
    LogError(@"log test error");
    LogInfo(@"log test info");
    [QHVCEditLogger logFunc:__FUNCTION__ line:__LINE__ prefix:@"[Test]" level:QHVCEditLoggerLevelTrace msg:@"log test trace"];
    
    return 0;
}

#pragma mark - Utils

int QHVCEditTestUtilsDictionary()
{
    [QHVCEditUtils dictionaryIsNull:@"null"];
    [QHVCEditUtils dictionaryIsNull:@{}];
    [QHVCEditUtils dictionaryIsNull:@{@"key":@"value"}];
    
    [QHVCEditUtils createJsonDataWithDictionary:nil];
    [QHVCEditUtils createJsonDataWithDictionary:@"test"];
    NSData* data = [QHVCEditUtils createJsonDataWithDictionary:@{@"key":@"value"}];
    QHVCEDIT_TEST_OBJECT(data);
    
    [QHVCEditUtils resolveJsonDataToDictionary:nil];
    [QHVCEditUtils resolveJsonDataToDictionary:@"test"];
    [QHVCEditUtils resolveJsonDataToDictionary:[NSData data]];
    [QHVCEditUtils resolveJsonDataToDictionary:data];
    
    return 0;
}

int QHVCEditTestUtilsFile()
{
    NSString* filePath = [NSTemporaryDirectory() stringByAppendingString:@"test.jpg"];
    NSString* fileDir = [NSTemporaryDirectory() stringByAppendingString:@"testDir"];
    [QHVCEditUtils isFileExisted:filePath isDirectory:NO];
    [QHVCEditUtils removeFile:fileDir];
    [QHVCEditUtils createDirectoryAtPath:fileDir];
    
    return 0;
}

int QHVCEditTestUtilsString()
{
    [QHVCEditUtils stringIsNull:@""];
    [QHVCEditUtils stringIsNull:[NSNull null]];
    [QHVCEditUtils stringIsNull:@"test"];
    
    return 0;
}

int QHVCEditTestUtilsBuffer()
{
    CVPixelBufferRef buffer = [QHVCEditUtils createPixelbuffer:100 height:100 type:QHVCEditUtilsPixelbufferType_BGRA];
    QHVCEDIT_TEST_OBJECT(buffer);
    
    buffer = [QHVCEditUtils createPixelbuffer:100 height:100 type:QHVCEditUtilsPixelbufferType_NV12];
    QHVCEDIT_TEST_OBJECT(buffer);
    
    return 0;
}

int QHVCEditTestUtilsColor()
{
    [QHVCEditUtils colorForHex:@"FFFF" forAlpha:1.0];
    [QHVCEditUtils colorForHex:@"#FFFFFFFF" forAlpha:1.0];
    [QHVCEditUtils colorForHex:@"FFFFFF" forAlpha:1.0];
    
    [QHVCEditUtils colorForHex:@"FFFF"];
    [QHVCEditUtils colorForHex:@"#FFFFFFFFFF"];
    [QHVCEditUtils colorForHex:@"FFFFFFFF"];
    
    [QHVCEditUtils hexStringFromColor:[UIColor blackColor]];
    
    return 0;
}

int QHVCEditTestUtilsEasingFunction()
{
    [QHVCEditUtils cubicEaseIn:0.0 endValue:1.0 duration:1.0 curTime:0];
    [QHVCEditUtils cubicEaseOut:0.0 endValue:1.0 duration:1.0 curTime:0];
    [QHVCEditUtils cubicEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0];
    [QHVCEditUtils quintEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0];
    [QHVCEditUtils quartEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0];
    [QHVCEditUtils quadEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0];
    [QHVCEditUtils quadEaseOut:0.0 endValue:1.0 duration:1.0 curTime:0];
    
    [QHVCEditUtils cubicEaseIn:0.0 endValue:1.0 duration:1.0 curTime:1.0];
    [QHVCEditUtils cubicEaseOut:0.0 endValue:1.0 duration:1.0 curTime:1.0];
    [QHVCEditUtils cubicEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:1.0];
    [QHVCEditUtils quintEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:1.0];
    [QHVCEditUtils quartEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:1.0];
    [QHVCEditUtils quadEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:1.0];
    [QHVCEditUtils quadEaseOut:0.0 endValue:1.0 duration:1.0 curTime:1.0];
    
    [QHVCEditUtils cubicEaseIn:0.0 endValue:1.0 duration:1.0 curTime:0.5];
    [QHVCEditUtils cubicEaseOut:0.0 endValue:1.0 duration:1.0 curTime:0.5];
    [QHVCEditUtils cubicEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.5];
    [QHVCEditUtils quintEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.5];
    [QHVCEditUtils quartEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.5];
    [QHVCEditUtils quadEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.5];
    [QHVCEditUtils quadEaseOut:0.0 endValue:1.0 duration:1.0 curTime:0.5];
    
    [QHVCEditUtils cubicEaseIn:0.0 endValue:1.0 duration:1.0 curTime:0.3];
    [QHVCEditUtils cubicEaseOut:0.0 endValue:1.0 duration:1.0 curTime:0.3];
    [QHVCEditUtils cubicEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.3];
    [QHVCEditUtils quintEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.3];
    [QHVCEditUtils quartEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.3];
    [QHVCEditUtils quadEaseInOut:0.0 endValue:1.0 duration:1.0 curTime:0.3];
    [QHVCEditUtils quadEaseOut:0.0 endValue:1.0 duration:1.0 curTime:0.3];
    
    return 0;
}

int QHVCEditTestUtilsImage()
{
    NSString* jpgPath = [[NSBundle mainBundle] pathForResource:@"pic1" ofType:@"jpg"];
    NSString* heicPath = [[NSBundle mainBundle] pathForResource:@"test_heic" ofType:@"HEIC"];
    [QHVCEditUtils isHEICFile:@""];
    [QHVCEditUtils isHEICFile:jpgPath];
    [QHVCEditUtils isHEICFile:heicPath];
    
    NSString* mp4Path = [[NSBundle mainBundle] pathForResource:@"video2" ofType:@"mp4"];
    NSString* errorPath = @"errorPath.mp3";
    [QHVCEditUtils heicToJPEG:nil];
    [QHVCEditUtils heicToJPEG:errorPath];
    [QHVCEditUtils heicToJPEG:mp4Path];
    [QHVCEditUtils heicToJPEG:heicPath];
    
    return 0;
}

int QHVCEditTestUtilsTime()
{
    [QHVCEditUtils currentTime];
    [QHVCEditUtils currentTimestamp];
    [QHVCEditUtils systemTime];
    
    return 0;
}

int QHVCEditTestUtilsTransform()
{
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    CIImage* ciImage = [CIImage imageWithCGImage:image.CGImage];
    QHVCEDIT_TEST_OBJECT(ciImage);
    
    [QHVCEditUtils imageTranslateToZeroPoint:ciImage];
    
    return 0;
}

@interface QHVCEditTestVC : UIViewController <QHVCLifeCycleDelegate>
@end

@implementation QHVCEditTestVC
- (void)didBecomeActive:(NSNotification*)notification {}
- (void)willResignActive:(NSNotification*)notification {}
- (void)didEnterBackground:(NSNotification*)notification {}
- (void)willEnterForeground:(NSNotification*)notification {}
- (void)willTerminate:(NSNotification*)notification {}
- (void)didReceiveMemoryWarning:(NSNotification*)notification {}
- (BOOL)applicationOpenURL:(NSNotification*)notification {return NO;}
- (void)audioSessionInterrupt:(NSNotification *)notification {}
- (void)screenLocked:(NSNotification *)notification {}
@end

int QHVCEditTestUtilsLifeCircle()
{
    QHVCEditTestVC* testVC = [[QHVCEditTestVC alloc] init];
    [QHVCLifeCycle registerLifeCycleListener:testVC];
    [QHVCLifeCycle unregisterLifeCycleListener:testVC];
    return 0;
}

int QHVCEditTestHelperUtils(void)
{
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsDictionary());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsFile());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsString());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsBuffer());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsColor());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsEasingFunction());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsImage());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsTime());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsTransform());
    QHVCEDIT_TEST_RETURN_VALUE(QHVCEditTestUtilsLifeCircle());
    
    return 0;
}

#pragma mark - Tools

int QHVCEditTestHelperTools(void)
{
    NSString* movPath = [[NSBundle mainBundle] pathForResource:@"video1" ofType:@"MOV"];
    NSString* mp4Path = [[NSBundle mainBundle] pathForResource:@"test_video3" ofType:@"mp4"];
    NSString* heicPath = [[NSBundle mainBundle] pathForResource:@"test_heic" ofType:@"HEIC"];
    NSString* jpgPath1 = [[NSBundle mainBundle] pathForResource:@"pic1" ofType:@"jpg"];
    NSString* jpgPath2 = [[NSBundle mainBundle] pathForResource:@"pic1" ofType:@"jpg"];
    
    [QHVCEditTools getFileInfo:movPath];
    [QHVCEditTools getFileInfo:mp4Path];
    [QHVCEditTools getFileInfo:heicPath];
    [QHVCEditTools getFileInfo:jpgPath1];
    [QHVCEditTools getFileInfo:jpgPath2];
    [QHVCEditTools getFileInfo:nil];
    [QHVCEditTools getFileInfo:@"errorPath"];
    
    [QHVCEditTools getVersion];
    [QHVCEditTools setSDKLogLevel:QHVCEditLogLevelDebug];
    [QHVCEditTools setSDKLogLevelForFile:QHVCEditLogLevelDebug];
    [QHVCEditTools setUserLogLevel:QHVCEditLogLevelDebug];
    [QHVCEditTools setUserLogLevelForFile:QHVCEditLogLevelDebug];
    [QHVCEditTools setLogFilePath:NSTemporaryDirectory()];
    [QHVCEditTools writeLogToLocal:NO];
    [QHVCEditTools setLogFileParams:1 count:1];
    [QHVCEditTools printUserLog:QHVCEditLogLevelDebug prefix:@"[User]" content:@"test"];
    
    return 0;
}
