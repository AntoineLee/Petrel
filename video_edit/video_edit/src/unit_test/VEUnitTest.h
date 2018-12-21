#ifndef __VE_UNIT_TEST__
#define __VE_UNIT_TEST__

/*
 * 单测函数返回0成功，-1失败
 */

#define VE_ASSERT_EQ(a,b) \
	do{	\
		if(a != b)return -1; \
	}while(0);

#define VE_ASSERT_NEQ(a,b) \
	do{	\
		if(a == b)return -1; \
	}while(0);


const char* getVideoPath(int i);
const char* getHevcVideoPath(int i);
const char* getWebmVideoPath(int i);
const char* getMusicPath(int i);
const char* getPicPath(int i);
const char* getRgbaPicPath(int i);
const char* getOutputDir();
int getId();
int testAll();

void testEnableLog();

int testInterfaceUitls();
int testInterfaceTimeline();
int testInterfaceTrack();
int testInterfaceClip();
int testInterfaceTransition();
int testInterfaceFilter();
int testInterfaceExport();
int testInterfaceThumbnail();

int testInterfaceWebm();
int testInterfaceWebm2();
int testVEUtils();
int testVESwsscale();
int testVEBitstream();
int testVEConfigBase();

int testVEConfigTrack();
int testVEConfigInsertClip1();
int testVEConfigInsertClip2();
int testVEConfigModClip1();
int testVEConfigModClip2();
int testVEConfigMoveClip();
int testVEConfigDelClip1();
int testVEConfigDelClip2();
int testVEConfigGetClip();
int testVEConfigGetClipDuration();
int testVEConfigGetClipsCount();
int testVEConfigGetClips();
int testVEConfigGetTrackDuration();
int testVEConfigAddTransition();
int testVEConfigModTransition();
int testVEConfigGetTransition();
int testVEConfigDelTransition();
int testVEConfigAddFilter();
int testVEConfigModFilter();
int testVEConfigGetFilter();
int testVEConfigDelFilter();


int testVEThumbnail();
int testVESourceOpenSource();
int testVESourceGetAudioFilterString();
int testVESourceGetSourceInfo();
int testVESourceH264Video();
int testVESourcePic();
int testVESourceAudio();
int testVESourceAudioNoFilter();
int testVESourceAudioSlv();
int testVESourceGetDurationForPicture();

int testVEExport1();
int testVEExport2();
int testVEExport3();
int testVEExport4();
int testVEExport5();
int testVEExport6();
int testVEExport7();
int testVEExport8();
int testVEExport9();
int testVEQueue();
int testVEMp4Writer();
int testVEAudioFilter();
int testVEThumbnail1();

int testInterfaceWebm3();
int testVEThumbnail2();
int testVEMp4Writer2();

#ifdef __cplusplus
extern "C" {
#endif
	int GetAndroidVersion();
#ifdef __cplusplus
}
#endif
#endif /* __VE_UNIT_TEST__ */
