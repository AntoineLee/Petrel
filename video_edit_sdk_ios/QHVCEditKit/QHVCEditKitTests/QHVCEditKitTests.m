//
//  QHVCEditKitTests.m
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/5.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "QHVCEffectTest.h"
#import "QHVCEditTest.h"

@interface QHVCEditKitTests : XCTestCase
@property (nonatomic, strong) CIImage* inputImage;

@end

@implementation QHVCEditKitTests

- (void)setUp
{
    UIImage* image = [UIImage imageNamed:@"pic1.jpg"];
    self.inputImage = [CIImage imageWithCGImage:image.CGImage];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample
{
    XCTAssertNotNil(self.inputImage);
    
    QHVCEffectTestAll(self.inputImage);
    QHVCEditTestAll();
}

@end
