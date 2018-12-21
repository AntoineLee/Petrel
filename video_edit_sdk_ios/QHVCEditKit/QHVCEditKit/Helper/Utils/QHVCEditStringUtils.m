
#import "QHVCEditUtils.h"

@implementation QHVCEditUtils(QHVCEditStringUtils)

+ (BOOL)stringIsNull:(NSString *)parameter
{
    if ((NSNull *)parameter == [NSNull null])
    {
        return YES;
    }
    if (parameter == nil || [parameter length] == 0)
    {
        return YES;
    }
    return NO;
}

@end
