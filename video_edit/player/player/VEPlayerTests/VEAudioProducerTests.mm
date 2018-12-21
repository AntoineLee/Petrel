#import <XCTest/XCTest.h>
#import "VETestAudioProducer.hpp"
@interface VEAudioProducerTests : XCTestCase

@end

const char* getVideoPath(int i){
    
    if(0 == i){
        NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"video1.MOV" ofType:@""];
        const char* path = [pathStr UTF8String];
        return path;
    }else if(1 == i){
        NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"video2.mp4" ofType:@""];
        const char* path = [pathStr UTF8String];
        return path;
    }else{
        NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"video1.MOV" ofType:@""];
        const char* path = [pathStr UTF8String];
        return path;
    }
    return 0;
}

const char* getHevcVideoPath(int i){
    NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"hevc.mp4" ofType:@""];
    const char* path = [pathStr UTF8String];
    return path;
    
}
const char* getWebmVideoPath(int i){
    NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"a.webm" ofType:@""];
    const char* path = [pathStr UTF8String];
    return path;
}
const char* getPicPath(int i){
    NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"pic1.jpg" ofType:@""];
    const char* path = [pathStr UTF8String];
    return path;
}
const char* getMusicPath(int i){
    NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"music.mp3" ofType:@""];
    const char* path = [pathStr UTF8String];
    return path;
}

const char* getRgbaPicPath(int i){
    NSString* pathStr = [[NSBundle mainBundle] pathForResource:@"pic_rgba.PNG" ofType:@""];
    const char* path = [pathStr UTF8String];
    return path;
}

const char* getOutputDir(){
    NSString* outPath = NSTemporaryDirectory();
    const char* path = [outPath UTF8String];
    return path;
}

@implementation VEAudioProducerTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    XCTAssertTrue(testInterfaceAudioProducer() == 0,"获取pcm成功");
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
