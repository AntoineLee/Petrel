#ifndef AUDIO_PLAY_H
#define AUDIO_PLAY_H

#include <stdlib.h>
#define AUDIO_DEFAULT_VALUE 10.0
#define AUDIO_CACHED_MIN 50 //ms

#ifdef ANDROID_NDK

#include <assert.h>
#include <jni.h>
#include <string.h>

// for __android_log_print(ANDROID_LOG_INFO, "YourApp", "formatted message");
// #include <android/log.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include"list_head.h"
#include"lock.h"
#include"TCriticalSection.h"

#define ms_cached 100
class VEAudioManager;

class AudioRender
{
public:
    AudioRender();
    ~AudioRender();
public:
    int BwAudioCreate( int rate,
                      int nChannel,
                      int bitsPerSample);
    void BwAudioDestroy();
    int  BwAudioStart();
    void BwAudioStop();
    void BwAudioPause();
    
    //int BwAudioWrite2(void* ptr);
    int BwAudioWrite( const void* buffer, int size);
    
    //int BwAudioGetVolume( float* left);
    int BwAudioSetVolume( float left);
    
    //int BwAudioTimeCached();
    
    //void BwSetAM(VEAudioManager *pAM);
    //void BwAudioSpeed(float rate);
    
private:
    void init_list();
    intptr_t sles_startup();
    void playcb(SLAndroidSimpleBufferQueueItf bq, void *soundMix);
    static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *soundMix);
    void SlesShutdown();
    int SlesCreateEngine();
    int SlesCreateBQPlayer(int rate,int nChannel,int bitsPerSample);
    
    
private:
    // engine interfaces
    static SLObjectItf engineObject;
    static SLEngineItf engineEngine;
    
    // output mix interfaces
    static SLObjectItf outputMixObject;
    static SLEnvironmentalReverbItf outputMixEnvironmentalReverb;

    static comn::CriticalSection m_csDevice;
    
    // buffer queue player interfaces
    SLObjectItf bqPlayerObject;
    SLPlayItf bqPlayerPlay;
    SLPlaybackRateItf bqPlayerPlaybackRate;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    SLEffectSendItf bqPlayerEffectSend;
    SLMuteSoloItf bqPlayerMuteSolo;
    SLVolumeItf bqPlayerVolume;
    
    // aux effect on the output mix, used by the buffer queue player
    SLEnvironmentalReverbSettings reverbSettings;
    
    list_head head, used_by_sl;
    lock_t lck;
    intptr_t nb;
    intptr_t bytes_cached;
    intptr_t bytesperms;
    
    VEAudioManager *m_pAM;
    float m_fVolume;
    float m_fLastVol;
    comn::CriticalSection m_cs;
    bool m_needExit;
};
#endif

#ifdef __APPLE__
class AudioProcess;

class AudioRender
{
public:
    AudioRender();
    ~AudioRender();
public:
    int BwAudioCreate(int rate,int nChannel,int bitsPerSample);
    void BwAudioDestroy();
    int  BwAudioStart();
    int  BwAudioPause();
    void BwAudioStop();
    
    //int BwAudioWrite2(void* ptr);
    int BwAudioWrite( const void* buffer, int size);
    
    //int BwAudioGetVolume( float* left);
    int BwAudioSetVolume( float left);
    
    //int BwAudioTimeCached();
    //void BwClean();
    //void BwCleanAudio();
    //void BwAudioSpeed(float rate);
    //void BwResetAudioRender();
private:
    AudioProcess *m_pAudioProcess;
};
#endif

#endif




