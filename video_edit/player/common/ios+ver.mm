#import "ios+ver.h"

#ifdef __APPLE__
#import <UIKit/UIDevice.h>
#import <uikit/UIApplication.h>

bool isIOSVerAbove(float ver)
{
    bool aboveVer = false;
    if([[[UIDevice currentDevice] systemVersion] floatValue] >= ver)
        aboveVer = true;
    return aboveVer;
}
//
//bool isIOSAbove80()
//{
//    if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0)
//    {
//        return true;
//    }
//    return  false;
//}
//
//bool isIOSAbove110()
//{
//    if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 11.0)
//    {
//        return true;
//    }
//    return  false;
//}
//
//bool isIOSAbove10()
//{
//    if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 10.0)
//    {
//        return true;
//    }
//    return  false;
//}
//
//
//bool isAppInActive()
//{
//    if([UIApplication sharedApplication].applicationState == UIApplicationStateActive)
//        return true;
//    return false;
//}
#endif
