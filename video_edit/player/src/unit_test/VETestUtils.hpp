#ifndef VETestUtils_hpp
#define VETestUtils_hpp

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "VEConfig.h"

#ifdef __cplusplus
extern "C" {
#endif
    
int veTestNowMS();
VEConfig *veTestCreateConfig();
    
#ifdef __cplusplus
}
#endif

#define VE_TEST_SYNC(timeout,status) {\
int start = veTestNowMS(); \
int left = timeout; \
do{ \
    left = timeout - (veTestNowMS() - start);\
    usleep(50 * 1000);\
}while(left > 0 && status == 0); \
}

#define VE_TEST_ASSERT(expression) {\
    if(!(expression)) \
        return -1; \
}


#endif /* VETestUtils_hpp */
