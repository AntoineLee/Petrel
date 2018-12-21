#include "VETestPlayer.hpp"
#include "VETestUtils.hpp"
#include "ve_player.h"
#include "VEUnitTestConfig.h"
#include "VEUnitTest.h"
#include "VEPlayerUtils.h"

int ve_test_player_got_data = 0;
int ve_test_player_got_status_seek = 0;
int ve_test_player_got_status_playback = 0;
int ve_test_player_got_frames = 0;

void clear_global_flags(){
    ve_test_player_got_data = 0;
    ve_test_player_got_status_seek = 0;
    ve_test_player_got_status_playback = 0;
    ve_test_player_got_frames = 0;
}

void ve_test_player_data_callback(HANDLE player_handle,ve_filter_callback_param* param,void* userExt){
    ve_test_player_got_data = 1;
    if(param){
        ve_v_frame_callback_param frame1 = param->multitracks[0].tracks[0].frame_data[0];
        ve_test_player_got_frames = 1;
        ve_v_frame_callback_param frame2 = param->multitracks[0].tracks[0].frame_data[1];
        if(frame2.transition_frame)
            ve_test_player_got_frames ++;
    }
    //todo 检查数据
}

void ve_test_player_status_callback(HANDLE player_handle,int status,void* userExt){
    if(status == VE_PLAYER_SEEK_COMPLETE){
        ve_test_player_got_status_seek = 1;
    }else if (status == VE_PLAYER_PLAYBACK_COMPLETE){
        ve_test_player_got_status_playback = 1;
    }
}

int testPlayerConfig1(){
    VEConfig config = getVEConfig11();
    HANDLE h;
#ifdef  __ANDROID__
    h = ve_player_create(&config, 0,ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#else
    h = ve_player_create(&config, ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#endif
    VE_TEST_ASSERT(h != NULL)
    
    ve_player_usehw(h, 0);
    ve_player_start(h);
    ve_player_pause(h);
    ve_player_play(h);
    
    VE_TEST_SYNC(2000, ve_test_player_got_data)
    ve_player_seek(h, 2000);
    
    VE_TEST_SYNC(5000, ve_test_player_got_status_seek)
    VE_TEST_ASSERT(ve_test_player_got_status_seek != 0)
    VE_TEST_ASSERT(ve_test_player_got_data != 0)
    
    ve_player_refresh_config(&config,h);
    int cur = ve_player_get_current_time(h);
    VE_TEST_ASSERT(cur >= 2000);
    VE_TEST_SYNC(5000, ve_test_player_got_status_playback)
    VE_TEST_ASSERT(ve_test_player_got_status_playback != 0)
    
    ve_player_free(h);
    return 0;
}

int testPlayerConfig5(){
    VEConfig config = getVEConfig5();
    
    HANDLE h;
#ifdef  __ANDROID__
    h = ve_player_create(&config, 0,ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#else
    h = ve_player_create(&config, ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#endif
    VE_TEST_ASSERT(h != NULL)
    
    ve_player_start(h);
    VE_TEST_SYNC(2000, ve_test_player_got_data)
    ve_player_seek(h, 2000);
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    clear_global_flags();
    ve_player_seek(h, 100);
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    clear_global_flags();
    ve_player_seek(h, 200);
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    clear_global_flags();
    ve_player_seek(h, 500);
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    
    ve_player_free(h);
    
    if(ve_test_player_got_status_seek && ve_test_player_got_data)
        return 0;
    
    return -1;
}

int testPlayerSingleFile(VEConfig *config){
    //VEConfig config = getVESFConfig(getHevcVideoPath(0), 0, 2000);
    
    HANDLE h;
#ifdef  __ANDROID__
    h = ve_player_create(config, 0,ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#else
    h = ve_player_create(config, ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#endif
    VE_TEST_ASSERT(h != NULL)
    
    ve_player_start(h);
    VE_TEST_SYNC(2000, ve_test_player_got_data)
    ve_player_seek(h, 1000);
    
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    VE_TEST_ASSERT(ve_test_player_got_status_seek != 0)
    VE_TEST_ASSERT(ve_test_player_got_data != 0)
    
    ve_player_play(h);
    VE_TEST_SYNC(5000, ve_test_player_got_status_playback)
    
    ve_player_free(h);
    VE_TEST_ASSERT(ve_test_player_got_status_playback != 0)
    return 0;
}

int testPlayerTransition(){
    VEConfig config = getTransitionConfig();
    
    HANDLE h;
#ifdef  __ANDROID__
    h = ve_player_create(&config, 0,ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#else
    h = ve_player_create(&config, ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#endif
    VE_TEST_ASSERT(h != NULL)
    
    ve_player_start(h);
    VE_TEST_SYNC(2000, ve_test_player_got_data)
    ve_player_seek(h, 2000);
    
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    VE_TEST_ASSERT(ve_test_player_got_status_seek != 0)
    VE_TEST_ASSERT(ve_test_player_got_data != 0)
    VE_TEST_ASSERT(ve_test_player_got_frames == 2)
    
    clear_global_flags();
    ve_player_seek(h, 2050);
    
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    VE_TEST_ASSERT(ve_test_player_got_status_seek != 0)
    VE_TEST_ASSERT(ve_test_player_got_data != 0)
    VE_TEST_ASSERT(ve_test_player_got_frames == 2)
    
    ve_player_play(h);
    VE_TEST_SYNC(5000, ve_test_player_got_status_playback)
    ve_player_play(h); //完后后play
    
    ve_player_free(h);
    VE_TEST_ASSERT(ve_test_player_got_status_playback != 0)
    return 0;
}

int testPlayerSLVFile(){
    VEConfig config = getSLVConfig();
    
    HANDLE h;
#ifdef  __ANDROID__
    h = ve_player_create(&config, 0,ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#else
    h = ve_player_create(&config, ve_test_player_data_callback, ve_test_player_status_callback, NULL);
#endif
    VE_TEST_ASSERT(h != NULL)
    
    ve_player_start(h);
    VE_TEST_SYNC(2000, ve_test_player_got_data)
    ve_player_seek(h, 2000);
    
    VE_TEST_SYNC(1000, ve_test_player_got_status_seek)
    VE_TEST_ASSERT(ve_test_player_got_status_seek != 0)
    VE_TEST_ASSERT(ve_test_player_got_data != 0)
    
    ve_player_play(h);
    VE_TEST_SYNC(10000, ve_test_player_got_status_playback)
    
    ve_player_free(h);
    VE_TEST_ASSERT(ve_test_player_got_status_playback != 0)
    return 0;
}

int testSLVFunc(){
    slv_info slv;
    slv.active = true;
    slv.len = 3;
    slv.start_time[0] = 0;
    slv.end_time[0] = 1000;
    slv.speed[0] = 2.0;
    slv.start_time[1] = 1000;
    slv.end_time[1] = 2000;
    slv.speed[1] = 2.0;
    slv.start_time[2] = 2000;
    slv.end_time[2] = 3000;
    slv.speed[2] = 2.0;
    
    int ret = VEPlayerUtils::GetSlvDuration(&slv, 0, 1000);
    VE_TEST_ASSERT(ret == 500)
    ret = VEPlayerUtils::GetSlvDuration(&slv, 800, 1500);
    VE_TEST_ASSERT(ret == 350)
    ret = VEPlayerUtils::GetSlvDuration(&slv, 0, 3000);
    VE_TEST_ASSERT(ret == 1500)
    ret = VEPlayerUtils::GetSlvDuration(&slv, 6000, 3000);
    VE_TEST_ASSERT(ret == 0)
    
    ret = VEPlayerUtils::GetSlvStartTimeInOriginalTime(&slv, 1000);
    VE_TEST_ASSERT(ret == 2000)
    ret = VEPlayerUtils::GetSlvStartTimeInOriginalTime(&slv, -1);
    VE_TEST_ASSERT(ret == 0)
    
    ret = VEPlayerUtils::GetSlvOriginalToStartTime(&slv, 2400);
    VE_TEST_ASSERT(ret == 1200)
    ret = VEPlayerUtils::GetSlvOriginalToStartTime(&slv, -1);
    VE_TEST_ASSERT(ret == 0)
    
    slv.active = false;
    ret = VEPlayerUtils::GetSlvStartTimeInOriginalTime(&slv, 1000);
    VE_TEST_ASSERT(ret == 1000)
    ret = VEPlayerUtils::GetSlvOriginalToStartTime(&slv, 1000);
    VE_TEST_ASSERT(ret == 1000)
    ret = VEPlayerUtils::GetSlvDuration(&slv, 0, 1000);
    VE_TEST_ASSERT(ret == 0)
    
    return 0;
}

int testInterfacePlayer(){
    VE_TEST_ASSERT(testPlayerConfig1() == 0)
    clear_global_flags();
    VE_TEST_ASSERT(testPlayerConfig5() == 0)
    clear_global_flags();
    VEConfig configh265 = getVESFConfig(getHevcVideoPath(0), 0, 2000,0);
    VE_TEST_ASSERT(testPlayerSingleFile(&configh265) == 0)
    clear_global_flags();
    VEConfig configwebm = getVESFConfig(getWebmVideoPath(0), 0, 2000,0);
    VE_TEST_ASSERT(testPlayerSingleFile(&configwebm) == 0)
    clear_global_flags();
    VEConfig configpic = getVESFConfig(getPicPath(0), 0, 3000,1);
    VE_TEST_ASSERT(testPlayerSingleFile(&configpic) == 0)
    clear_global_flags();
    VE_TEST_ASSERT(testPlayerTransition() == 0)
    clear_global_flags();
    VE_TEST_ASSERT(testPlayerSLVFile() == 0)
    clear_global_flags();
    VE_TEST_ASSERT(testSLVFunc() == 0)
    return 0;
}
