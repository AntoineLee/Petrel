//
//  QHVCEditPrefs.h
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/6/25.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "QHVCConfig.h"
#import <QHVCEditKit/QHVCEditKit.h>

//颜色
#define UIColorFromHex(s) [UIColor colorWithRed:(((s & 0xFF0000) >> 16 )) / 255.0 green:((( s & 0xFF00 ) >> 8 )) / 255.0 blue:(( s & 0xFF )) / 255.0 alpha:1.0]
#define UIColorFromARGBHex(s) [UIColor colorWithRed:(((s & 0xFF0000) >> 16 )) / 255.0 green:((( s & 0xFF00 ) >> 8 )) / 255.0 blue:(( s & 0xFF )) / 255.0 alpha:(((s & 0xFF000000) >> 32 ) / 255.0)]

//block
#define SAFE_BLOCK(block, ...) if((block)) { block(__VA_ARGS__); }

#define SAFE_BLOCK_IN_MAIN_QUEUE(block, ...) if((block)) {\
if ([NSThread isMainThread]) {\
block(__VA_ARGS__);\
}\
else {\
dispatch_async(dispatch_get_main_queue(), ^{\
block(__VA_ARGS__);\
});\
}\
}

//self
#define WEAK_SELF    __weak __typeof(&*self) weakSelf = self;
#define STRONG_SELF  __strong __typeof(&*self) self = weakSelf;

//通知
#define QHVCEDIT_DEFINE_NOTIFY_SHOW_OVERLAY_FUNCTION  @"ShowOverlayFunction"
#define QHVCEDIT_DEFINE_NOTIFY_SHOW_STICKER_FUNCTION  @"ShowStickerFunction"
#define QHVCEDIT_DEFINE_NOTIFY_SHOW_SUBTITLE_FUNCTION @"ShowSubtitleFunction"
#define QHVCEDIT_DEFINE_NOTIFY_CLEAN_ALL_PARAMS       @"cleanALLParams"

//other
#define kColorHigh @"77C5FF"
#define kColorNormal @"8C8B91"
#define kDefaultOverlayWidth 200.0
#define kDefaultOverlayHeight 200.0
#define kDefaultStickerWidth 100.0
#define kDefaultStickerHeight 100.0
#define kDefaultSubtitleWidth 100.0
#define kDefaultSubtitleHeight 100.0

typedef NS_ENUM(NSUInteger, QHVCEditFunctionViewType)
{
    QHVCEditFunctionViewTypeFunctionBase,
    QHVCEditFunctionViewTypeOverlay,
};

@interface QHVCEditPrefs : NSObject

+ (QHVCEditPrefs *)sharedPrefs;

+ (NSString *)timeFormatMs:(NSInteger)ms;
+ (UIColor *)colorHighlight;
+ (UIColor *)colorNormal;
+ (UIColor *)colorHex:(NSString *)hex;
+ (UIColor *)colorARGBHex:(NSString *)hex;
+ (int)randomNum:(int)from to:(int)to;
+ (UIImage *)convertViewToImage:(UIView *)view;
+ (NSString *)hexStringFromColor:(UIColor *)color; //UIColor转ARGB

- (NSString *)videoTempDir;
- (NSArray *)editFunctions;

@end
