
#import "QHVCEditDictionaryUtils.h"
#import "QHVCEditUtilsSet.h"

@implementation QHVCEditUtils(QHVCEditDictionaryUtils)

+ (BOOL)dictionaryIsNull:(NSDictionary *)dict
{
    if (![dict isKindOfClass:[NSDictionary class]])
    {
        return YES;
    }
    
    if(dict.count == 0)
    {
        return YES;
    }
    
    return NO;
}

+ (NSData *)createJsonDataWithDictionary:(NSDictionary *)dictionary
{
    if (dictionary == nil)
    {
        return nil;
    }
    
    BOOL isValid = [NSJSONSerialization isValidJSONObject:dictionary];
    if (!isValid)
    {
        return nil;
    }
    
    NSError* error = nil;
    NSData* jsonData = [NSJSONSerialization dataWithJSONObject:dictionary
                                                       options:NSJSONWritingPrettyPrinted
                                                         error:&error];
    return jsonData;
}

+ (NSDictionary *)resolveJsonDataToDictionary:(NSData *)jsonData
{
    if (!jsonData)
    {
        return nil;
    }
    
    if (![jsonData isKindOfClass:[NSData class]])
    {
        return nil;
    }
    
    NSError* error = nil;
    NSDictionary* resolvedData = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableLeaves error:&error];
    if (error)
    {
        return nil;
    }
    
    return resolvedData;
}

@end
