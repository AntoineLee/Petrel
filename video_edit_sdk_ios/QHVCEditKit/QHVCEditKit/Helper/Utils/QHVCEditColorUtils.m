

#import "QHVCEditColorUtils.h"

@implementation QHVCEditUtils(QHVCEditColorUtils)

+ (UIColor *)colorForHex:(NSString *)hexColor forAlpha:(CGFloat)alpha
{
    // String should be 6 or 7 characters if it includes '#'
    if ([hexColor length]  < 6)
        return nil;
    
    // strip # if it appears
    if ([hexColor hasPrefix:@"#"])
        hexColor = [hexColor substringFromIndex:1];
    
    // if the value isn't 6 characters at this point return
    // the color black
    if ([hexColor length] != 6)
        return nil;
    
    // Separate into r, g, b substrings
    NSRange range;
    range.location = 0;
    range.length = 2;
    
    NSString *rString = [hexColor substringWithRange:range];
    
    range.location = 2;
    NSString *gString = [hexColor substringWithRange:range];
    
    range.location = 4;
    NSString *bString = [hexColor substringWithRange:range];
    
    // Scan values
    unsigned int r, g, b;
    [[NSScanner scannerWithString:rString] scanHexInt:&r];
    [[NSScanner scannerWithString:gString] scanHexInt:&g];
    [[NSScanner scannerWithString:bString] scanHexInt:&b];
    
    return [UIColor colorWithRed:((float) r / 255.0f)
                           green:((float) g / 255.0f)
                            blue:((float) b / 255.0f)
                           alpha:alpha];
}

+ (UIColor *)colorForHex:(NSString *)hexColor
{
    // String should be 8 or 9 characters if it includes '#'
    if ([hexColor length]  < 8)
        return nil;
    
    // strip # if it appears
    if ([hexColor hasPrefix:@"#"])
        hexColor = [hexColor substringFromIndex:1];
    
    // if the value isn't 6 characters at this point return
    // the color black
    if ([hexColor length] != 8)
        return nil;
    
    // Separate into a, r, g, b substrings
    NSRange range;
    range.location = 0;
    range.length = 2;
    
    NSString *aString = [hexColor substringWithRange:range];
    
    range.location = 2;
    NSString *rString = [hexColor substringWithRange:range];
    
    range.location = 4;
    NSString *gString = [hexColor substringWithRange:range];
    
    range.location = 6;
    NSString *bString = [hexColor substringWithRange:range];
    
    // Scan values
    unsigned int a, r, g, b;
    [[NSScanner scannerWithString:aString] scanHexInt:&a];
    [[NSScanner scannerWithString:rString] scanHexInt:&r];
    [[NSScanner scannerWithString:gString] scanHexInt:&g];
    [[NSScanner scannerWithString:bString] scanHexInt:&b];
    
    return [UIColor colorWithRed:((float) r / 255.0f)
                           green:((float) g / 255.0f)
                            blue:((float) b / 255.0f)
                           alpha:((float) a / 255.0f)];
}

+ (NSString *)hexStringFromColor:(UIColor *)color
{
    const CGFloat *components = CGColorGetComponents(color.CGColor);
    
    CGFloat r = components[0];
    CGFloat g = components[1];
    CGFloat b = components[2];
    CGFloat a = components[3];
    
    return [NSString stringWithFormat:@"%02lX%02lX%02lX%02lX",
            lroundf(a * 255),
            lroundf(r * 255),
            lroundf(g * 255),
            lroundf(b * 255)];
}

@end
