
#import <Foundation/Foundation.h>

@protocol QHVCLifeCycleDelegate <NSObject>

@optional

- (void)didBecomeActive:(NSNotification*)notification;
- (void)willResignActive:(NSNotification*)notification;
- (void)didEnterBackground:(NSNotification*)notification;
- (void)willEnterForeground:(NSNotification*)notification;
- (void)willTerminate:(NSNotification*)notification;
- (void)didReceiveMemoryWarning:(NSNotification*)notification;
- (BOOL)applicationOpenURL:(NSNotification*)notification;
- (void)audioSessionInterrupt:(NSNotification *)notification;
- (void)screenLocked:(NSNotification *)notification;

@end


@interface QHVCLifeCycle : NSObject<QHVCLifeCycleDelegate>

+ (void)registerLifeCycleListener:(id<QHVCLifeCycleDelegate>)obj;
+ (void)unregisterLifeCycleListener:(id<QHVCLifeCycleDelegate>)obj;

@end
