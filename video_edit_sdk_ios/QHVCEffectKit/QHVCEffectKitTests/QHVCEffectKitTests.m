//
//  QHVCEffectKitTests.m
//  QHVCEffectKitTests
//
//  Created by liyue-g on 2018/12/4.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "QHVCEffectTest.h"

@interface QHVCEffectKitTests : XCTestCase
@property (nonatomic, strong) CIImage* inputImage;

@end

@implementation QHVCEffectKitTests

- (void)setUp
{
    NSString* imagePath = [[NSBundle mainBundle] pathForResource:@"pic1" ofType:@"jpg"];
    self.inputImage = [CIImage imageWithContentsOfURL:[NSURL fileURLWithPath:imagePath]];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample
{
    XCTAssertEqual(QHVCEffectTestAll(self.inputImage), 0);
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
