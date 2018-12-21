//
//  QHVCEffectTest.h
//  QHVCEffectKitTests
//
//  Created by liyue-g on 2018/12/4.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreImage/CoreImage.h>

int QHVCEffectTestAll(CIImage* inputImage);
int QHVCEffectTestUtils(void);
int QHVCEffectTestFilter(CIImage* inputImage);
int QHVCEffectTestSticker(CIImage* inputeImage);
int QHVCEffectTestVideoTransfer(CIImage* inputImage);
int QHVCEffectTestMix(CIImage* inputImage);
int QHVCEffectTestVideoTransition(CIImage* image);
int QHVCEffectTestBase(CIImage* image);
