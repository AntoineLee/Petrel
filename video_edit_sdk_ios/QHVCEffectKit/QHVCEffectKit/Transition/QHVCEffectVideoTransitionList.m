//
//  QHVCEffectVideoTransitionList.m
//  QHVCEffectKit
//
//  Created by liyue-g on 2018/12/3.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import "QHVCEffectVideoTransitionList.h"

@implementation QHVCEffectVideoTransitionList

+ (NSDictionary *)transitionList
{
    NSDictionary* dict = @{
                           @"LinearBlur":@"CICustomLinearBlurTransition",
                       @"GlitchMemories":@"CICustomGlitchMemoriesTransition",
                            @"CrossZoom":@"CICustomCrossZoomTransition",
                          @"Directional":@"CICustomDirectionalTransition",
                        @"ZoominCircles":@"CICustomZoominCirclesTransition",
                          @"Windowslice":@"CICustomWindowsliceTransition",
                       @"BowTieVertical":@"CICustomBowTieVerticalTransition",
                     @"BowTieHorizontal":@"CICustomBowTieHorizontalTransition",
                            @"WipeRight":@"CICustomWipeRightTransition",
                             @"WipeLeft":@"CICustomWipeLeftTransition",
                               @"WipeUp":@"CICustomWipeUpTransition",
                             @"WipeDown":@"CICustomWipeDownTransition",
                               @"Radial":@"CICustomRadialTransition",
                                 @"Fade":@"CICustomFadeTransition",
                            @"FadeColor":@"CICustomFadeColorTransition",
                       @"FadeColorWhite":@"CICustomFadeColorWhiteTransition",
                        @"FadeGrayScale":@"CICustomFadeGrayScaleTransition",
                           @"SimpleZoom":@"CICustomSimpleZoomTransition",
                                @"Morph":@"CICustomMorphTransition",
                           @"CircleCrop":@"CICustomCircleCropTransition",
                           };
    return dict;
}

@end
