#include <stdio.h>
#include "gtest/gtest.h"
#include "VETestPlayer.hpp"
#include "VETestAudioProducer.hpp"

const char* getVideoPath(int i){
    if(i == 0)
        return "/sdcard/video/video1.mp4";
    else
        return "/sdcard/video/video2.mp4";
}
const char* getPicPath(int i){
	return "/sdcard/video/pic1.jpg";
}
const char* getOutputDir(){
	return "/sdcard/video/output.mp4";
}
const char* getMusicPath(int i){
	return "/sdcard/video/music.mp3";
}
const char* getHevcVideoPath(int i){
	return "/sdcard/video/hevc.mp4";
}
const char* getWebmVideoPath(int i){
	return "/sdcard/video/a.webm";
}
TEST(VEPLAYERTEST, 1)

{
    ASSERT_EQ(0,testInterfaceAudioProducer());
	ASSERT_EQ(0,testInterfacePlayer());
	SUCCEED();
}

GTEST_API_ int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
