
#import "QHVCEditUtilsSet.h"

@implementation QHVCEditUtils(QHVCEditTimeUtils)

+ (NSDate *)currentTime
{
    return [NSDate date];
}

+ (NSTimeInterval)currentTimestamp
{
    NSDate* currentDate = [QHVCEditUtils currentTime];
    NSTimeInterval currentTimeStamp = [currentDate timeIntervalSince1970];
    return currentTimeStamp;
}

+ (NSString *)systemTime
{
    NSDate *  senddate=[NSDate date];
    NSDateFormatter  *dateformatter=[[NSDateFormatter alloc] init];
    [dateformatter setDateFormat:@"HH:mm"];
    NSString * locationString = [dateformatter stringFromDate:senddate];
    return locationString;
}

@end
