#ifndef __VE_UNIT_TEST_CONFIG__
#define __VE_UNIT_TEST_CONFIG__

#include "VEConfig.h"

//单个视频,只有视频track
VEConfig getVEConfig1();
//单个图片
VEConfig getVEConfig2();
//单个视频
VEConfig getVEConfig3();
//单个视频(有音频）+ 音频track，背景音乐
VEConfig getVEConfig4();
//单个视频(无音频）+ 音频track，背景音乐,变速，调节音量
VEConfig getVEConfig5();
//单个文件config
VEConfig getVESFConfig(const char *file,int start,int end,int pic);
//多个视频+转场
VEConfig getTransitionConfig();
//模拟慢视频
VEConfig getSLVConfig();

VEConfig getVEConfig11();
#endif
