
#import "QHVCEditEasingFunctionUtils.h"

@implementation QHVCEditUtils(QHVCEditEasingFunctionUtils)

+ (float)cubicEaseIn:(float)beginValue endValue:(float)endValue duration:(float)duration curTime:(float)time
{
    if (time == 0.0)
        return beginValue;
    else if (time == duration)
        return beginValue + endValue;
    
    time = time / (duration);
    
    return endValue * pow(time, 3.0) + beginValue;
}

+ (float)cubicEaseOut:(float)beginValue endValue:(float)endValue duration:(float)duration curTime:(float)time
{
    if (time == 0.0)
        return beginValue;
    else if (time == duration)
        return beginValue + endValue;
    
    time = time / duration - 1.0;
    
    return endValue * (pow(time, 3.0) + 1.0) + beginValue;
}

+ (float)cubicEaseInOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time
{
    if (time < 0.0001) {
        return begainValue;
    }
    else if (time == duration) {
        return begainValue + endValue;
    }
    
    if ((time /= duration / 2.0) < 1.0) {
        return endValue / 2.0 * time*time*time + begainValue;
    }
    
    time -= 2.0;
    
    return endValue / 2.0 * (time*time*time + 2.0) + begainValue;
}

+ (float)quintEaseInOut:(float)beginValue endValue:(float)endValue duration:(float)duration curTime:(float)time
{
    if (time == 0.0)
    {
        return beginValue;
    }
    else if (time == duration)
    {
        return beginValue + endValue;
    }
    
    time = time / (duration / 2.0);
    if (time < 1.0)
    {
        return endValue / 2.0 * pow(time, 5.0) + beginValue;
    }
    time = time - 2.0;
    return endValue / 2.0 * (pow(time, 5.0) + 2.0) + beginValue;
}

+ (float)quartEaseInOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time
{
    if (time < 0.0001) {
        return begainValue;
    }
    else if (time == duration) {
        return begainValue + endValue;
    }
    
    if ((time /= duration / 2.0) < 1.0) {
        return endValue / 2 * time*time*time*time + begainValue;
    }
    
    time -= 2.0;
    
    return -endValue / 2.0 * (time*time*time*time - 2.0) + begainValue;
}

+ (float)quadEaseInOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time
{
    if (time < 0.0001) {
        return begainValue;
    }
    else if (time == duration) {
        return begainValue + endValue;
    }
    
    return time < 0.5 ? 2.0 * time * time : time * (4.0 - 2.0 * time) - 1;
}

+ (float)quadEaseOut:(float)begainValue endValue:(float)endValue duration:(float)duration curTime:(float)time
{
    if (time < 0.0001) {
        return begainValue;
    }
    else if (time == duration) {
        return begainValue + endValue;
    }
    
    time /= duration;
    
    return -endValue *(time)*(time - 2.0) + begainValue;
}

@end
