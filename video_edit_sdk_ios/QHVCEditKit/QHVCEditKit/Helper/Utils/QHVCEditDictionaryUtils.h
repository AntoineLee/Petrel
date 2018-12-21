
#import "QHVCEditUtils.h"

@interface QHVCEditUtils(QHVCEditDictionaryUtils)

+ (BOOL)dictionaryIsNull:(NSDictionary *)dict;
+ (NSData *)createJsonDataWithDictionary:(NSDictionary *)dictionary;
+ (NSDictionary *)resolveJsonDataToDictionary:(NSData *)jsonData;

@end
