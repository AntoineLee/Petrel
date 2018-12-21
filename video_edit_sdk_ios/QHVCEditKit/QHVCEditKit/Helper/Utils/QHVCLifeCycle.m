
#import "QHVCLifeCycle.h"
#import <UIKit/UIApplication.h>
#import <AVFoundation/AVFoundation.h>

@implementation QHVCLifeCycle

+ (void)registerLifeCycleListener:(id<QHVCLifeCycleDelegate>)obj;
{
    #define REGISTER_SELECTOR(sel, notif_name)                     \
    if([obj respondsToSelector:sel])                               \
        [[NSNotificationCenter defaultCenter] 	addObserver:obj    \
                                                selector:sel       \
                                                name:notif_name    \
                                                object:nil         \
        ];                                                         \

    REGISTER_SELECTOR(@selector(didBecomeActive:), UIApplicationDidBecomeActiveNotification);
    REGISTER_SELECTOR(@selector(willResignActive:), UIApplicationWillResignActiveNotification);
    REGISTER_SELECTOR(@selector(didEnterBackground:), UIApplicationDidEnterBackgroundNotification);
    REGISTER_SELECTOR(@selector(willEnterForeground:), UIApplicationWillEnterForegroundNotification);
    REGISTER_SELECTOR(@selector(willTerminate:), UIApplicationWillTerminateNotification);
    REGISTER_SELECTOR(@selector(didReceiveMemoryWarning:), UIApplicationDidReceiveMemoryWarningNotification);
    REGISTER_SELECTOR(@selector(applicationOpenURL:), UIApplicationLaunchOptionsURLKey);
    REGISTER_SELECTOR(@selector(audioSessionInterrupt:), AVAudioSessionInterruptionNotification);
    REGISTER_SELECTOR(@selector(screenLocked:), UIApplicationProtectedDataWillBecomeUnavailable);
    #undef REGISTER_SELECTOR
}

+ (void)unregisterLifeCycleListener:(id<QHVCLifeCycleDelegate>)obj;
{
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationDidBecomeActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationWillEnterForegroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationWillTerminateNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationDidReceiveMemoryWarningNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationLaunchOptionsURLKey object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:AVAudioSessionInterruptionNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:obj name:UIApplicationProtectedDataWillBecomeUnavailable object:nil];
}

@end
