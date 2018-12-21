#include "VETestUtils.hpp"

VEConfig * veTestCreateConfig(){
    return NULL;
}

int veTestNowMS(){
    struct timeval t_start;
    gettimeofday(&t_start, NULL);
    int64_t now = ((long long)t_start.tv_sec)*1000+(long long)t_start.tv_usec/1000;
    return now;
}
