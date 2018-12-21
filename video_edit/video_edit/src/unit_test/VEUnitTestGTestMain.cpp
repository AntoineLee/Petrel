#include <stdio.h>
#include "gtest/gtest.h"
#include "VEUnitTest.h"


int test1(){
	printf("test1");
	return 0;
}

void test2(){
	printf("test2");
}
const char* getVideoPath(int i){
	return "/sdcard/video/video2.mp4";
}
const char* getPicPath(int i){
	return "/sdcard/video/pic1.jpg";
}
const char* getOutputDir(){
	return "/sdcard/video/";
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
const char* getRgbaPicPath(int i){
	return "/sdcard/video/pic_rgba.PNG";
}
int GetAndroidVersion(){
	return 18;
}

TEST(VELIBTEST, 1)

{

	ASSERT_EQ(0,testInterfaceUitls());
	ASSERT_EQ(0,testInterfaceTimeline());
	ASSERT_EQ(0,testInterfaceTrack());
	ASSERT_EQ(0,testInterfaceClip());
	ASSERT_EQ(0,testInterfaceTransition());
	ASSERT_EQ(0,testInterfaceFilter());
	ASSERT_EQ(0,testInterfaceExport());
	ASSERT_EQ(0,testInterfaceThumbnail());

	SUCCEED();
}

TEST(VELIBTEST, 2)

{

    ASSERT_EQ(0,testInterfaceWebm());
    ASSERT_EQ(0,testInterfaceWebm2());
    ASSERT_EQ(0,testVEUtils());
    ASSERT_EQ(0,testVESwsscale());
    ASSERT_EQ(0,testVEBitstream());


	SUCCEED();
}

TEST(VELIBTEST, 3)

{
	ASSERT_EQ(0,testVEConfigBase());
    ASSERT_EQ(0,testVEConfigTrack());
    ASSERT_EQ(0,testVEConfigInsertClip1());
    ASSERT_EQ(0,testVEConfigInsertClip2());
    ASSERT_EQ(0,testVEConfigModClip1());
    ASSERT_EQ(0,testVEConfigModClip2());
    ASSERT_EQ(0,testVEConfigMoveClip());
    ASSERT_EQ(0,testVEConfigDelClip1());
    ASSERT_EQ(0,testVEConfigDelClip2());
    ASSERT_EQ(0,testVEConfigGetClip());
    ASSERT_EQ(0,testVEConfigGetClipDuration());
    ASSERT_EQ(0,testVEConfigGetClipsCount());
    ASSERT_EQ(0,testVEConfigGetClips());
    ASSERT_EQ(0,testVEConfigGetTrackDuration());
    ASSERT_EQ(0,testVEConfigAddTransition());
    ASSERT_EQ(0,testVEConfigModTransition());
    ASSERT_EQ(0,testVEConfigGetTransition());
    ASSERT_EQ(0,testVEConfigDelTransition());
    ASSERT_EQ(0,testVEConfigAddFilter());
    ASSERT_EQ(0,testVEConfigModFilter());
    ASSERT_EQ(0,testVEConfigGetFilter());
    ASSERT_EQ(0,testVEConfigDelFilter());

	SUCCEED();
}

TEST(VELIBTEST, 4)

{

    ASSERT_EQ(0,testVEThumbnail());
    ASSERT_EQ(0,testVESourceOpenSource());
    ASSERT_EQ(0,testVESourceGetAudioFilterString());
    ASSERT_EQ(0,testVESourceGetSourceInfo());
    ASSERT_EQ(0,testVESourceH264Video());
    ASSERT_EQ(0,testVESourcePic());
    ASSERT_EQ(0,testVESourceAudio());
    ASSERT_EQ(0,testVESourceAudioNoFilter());
    ASSERT_EQ(0,testVESourceAudioSlv());
    ASSERT_EQ(0,testVESourceGetDurationForPicture());

	SUCCEED();
}
TEST(VELIBTEST, 5)

{
    ASSERT_EQ(0,testVEQueue());
    ASSERT_EQ(0,testVEAudioFilter());
    ASSERT_EQ(0,testVEThumbnail1());

	SUCCEED();
}


TEST(VELIBTEST, 6)

{

	ASSERT_EQ(0,testVEExport1());
	ASSERT_EQ(0,testVEExport2());
	ASSERT_EQ(0,testVEExport3());
	ASSERT_EQ(0,testVEExport4());
	ASSERT_EQ(0,testVEExport5());
	ASSERT_EQ(0,testVEExport6());
	ASSERT_EQ(0,testVEExport7());
	ASSERT_EQ(0,testVEExport9());
	SUCCEED();
}


TEST(VELIBTEST, 7)

{

	ASSERT_EQ(0,testInterfaceWebm3());
	ASSERT_EQ(0,testVEThumbnail2());
}

TEST(VELIBTEST, 8)
{

	ASSERT_EQ(0,testVEMp4Writer2());
}
GTEST_API_ int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
