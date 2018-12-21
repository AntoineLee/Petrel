#include "VEAudioRender.h"
#include "VEPlayerLogs.h"

#ifdef ANDROID_NDK
#include "mydef.h"
#include "my_buffer.h"
#include "lock.h"
#include "mbuf.h"
#include "VEAudioManager.h"
#include <math.h>

#define TAGAUDIORENDER "[AudioRender]"

bool g_bAudioRenderInit = false;
SLObjectItf AudioRender::engineObject = NULL;
SLEngineItf AudioRender::engineEngine = NULL;
SLObjectItf AudioRender::outputMixObject = NULL;
SLEnvironmentalReverbItf AudioRender::outputMixEnvironmentalReverb = NULL;
comn::CriticalSection AudioRender::m_csDevice;

AudioRender::AudioRender()
{
    m_needExit = false;
    lck = lock_initial;
    nb = 0;
    bytes_cached = 0;
    bytesperms = 0;
    m_fVolume = AUDIO_DEFAULT_VALUE;
    
    bqPlayerVolume = NULL;
    bqPlayerPlaybackRate = NULL;
    bqPlayerObject = NULL;
    bqPlayerPlay = NULL;
    bqPlayerBufferQueue = NULL;
    bqPlayerEffectSend = NULL;
    bqPlayerMuteSolo = NULL;
    m_fLastVol = 1.0;
    
    // aux effect on the output mix, used by the buffer queue player
    reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    m_pAM = NULL;
    init_list();
}

AudioRender::~AudioRender()
{
    
}

void AudioRender::init_list()
{
    INIT_LIST_HEAD(&used_by_sl);
    INIT_LIST_HEAD(&head);
}

intptr_t AudioRender::sles_startup()
{
    struct my_buffer* mbuf1 = mbuf_alloc_2(1024);
    if (mbuf1 == NULL) {
        return -1;
    }

    struct my_buffer* mbuf2 = mbuf_alloc_2(1024);
    if (mbuf2 == NULL) {
        return -1;
    }
    
//    struct my_buffer* mbuf3 = mbuf_alloc_2(1024);
//    if (mbuf3 == NULL) {
//        return -1;
//    }
//    
//    struct my_buffer* mbuf4 = mbuf_alloc_2(1024);
//    if (mbuf4 == NULL) {
//        return -1;
//    }
    memset(mbuf1->ptr[0], 0, 1024);
    memset(mbuf2->ptr[0], 0, 1024);
//    memset(mbuf3->ptr[0], 0, 1024);
//    memset(mbuf4->ptr[0], 0, 1024);
    
    list_add_tail(&mbuf1->head, &used_by_sl);
    list_add_tail(&mbuf2->head, &used_by_sl);
//    list_add_tail(&mbuf3->head, &used_by_sl);
//    list_add_tail(&mbuf4->head, &used_by_sl);
    
    if(bqPlayerBufferQueue == 0){
        return -1;
    }
    //mark("bqPlayerBufferQueue == %p\n", bqPlayerBufferQueue);
    SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                                      (short *)mbuf1->ptr[0], mbuf1->length);
    if (SL_RESULT_SUCCESS != result) {
        free_buffer(&used_by_sl);
        return -1;
    }

    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                             (short *) mbuf2->ptr[0], mbuf2->length);
    if (SL_RESULT_SUCCESS != result) {
        list_del(&mbuf2->head);
        mbuf2->mop->free(mbuf2);
    }
    
//    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
//                                                      (short *)mbuf3->ptr[0], mbuf3->length);
//    if (SL_RESULT_SUCCESS != result) {
//        JPLAYER_LOG_ERROR("bqPlayerBufferQueue bq init failed3 !!!\n");
//        list_del(&mbuf3->head);
//        mbuf3->mop->free(mbuf3);
//        return -1;
//    }
//    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
//                                             (short *)mbuf4->ptr[0], mbuf4->length);
//    if (SL_RESULT_SUCCESS != result) {
//        JPLAYER_LOG_ERROR("bqPlayerBufferQueue bq init failed4 !!!\n");
//        list_del(&mbuf4->head);
//        mbuf4->mop->free(mbuf4);
//        return -1;
//    }
    return 0;
}

//int AudioRender::BwAudioWrite2(void* ptr)
//{
//    lock_my(&lck);
//    if (list_empty(&used_by_sl)) {
//        if (-1 == sles_startup()) {
//            unlock_my(&lck);
//            return -1;
//        }
//    }
//
//    if (ptr != NULL) {
//        struct my_buffer* mbuf = (struct my_buffer *) ptr;
//        //mark("mbuf = %p\n", mbuf);
//        list_add_tail(&mbuf->head, &head);
//        //mark();
//        bytes_cached += mbuf->length;
//    }
//
//    int n = 0;
//    if (bytes_cached < nb) {
//        n = nb - bytes_cached;
//    }
//    unlock_my(&lck);
//
//    return n;
//}

//static intptr_t nr_hungry = 0;

void AudioRender::playcb(SLAndroidSimpleBufferQueueItf bq, void *soundMix)
{
    lock_my(&lck);

    bool b = list_empty(used_by_sl.next);
    struct my_buffer* oldmbuf = list_entry(used_by_sl.next, struct my_buffer, head);
    list_del(&oldmbuf->head);
    
    if(bq != bqPlayerBufferQueue) {
    	unlock_my(&lck);
        return;
    }

    if (bqPlayerBufferQueue == NULL) {
    	unlock_my(&lck);
        return;
    }

    if (b) {
        unlock_my(&lck);
        return;
    }
    while(!m_needExit){

        struct my_buffer* mbuf = NULL;
        if (!list_empty(&head)) {
            struct list_head* ent = head.next;
            list_del(ent);
            mbuf = list_entry(ent, struct my_buffer, head);
            oldmbuf->mop->free(oldmbuf);
            bytes_cached -= mbuf->length;
        }else{
        	unlock_my(&lck);
        	usleep(10000);
        	lock_my(&lck);
        	continue;
        }
        list_add_tail(&mbuf->head, &used_by_sl);

        SLresult result = (*bq)->Enqueue(bq, (short *) mbuf->ptr[0], mbuf->length);
        if(SL_RESULT_SUCCESS != result) {
            my_assert(0);
            list_del(&mbuf->head);
            mbuf->mop->free(mbuf);
            bytes_cached -= mbuf->length;
        }
        break;
#if 0
        else {
            mbuf = oldmbuf;
            memset(mbuf->ptr[0], 0, mbuf->length);
            //nr_hungry += mbuf->length;
        }
#endif
    }

     unlock_my(&lck);
}

//#ifdef __cplusplus
//extern "C"
//{
//#endif


void AudioRender::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *soundMix)
{
   // lock_my(&lck);
    ((AudioRender *) soundMix)->playcb(bq,NULL);
    //playcb(bq, soundMix);
    //unlock_my(&lck);
}  


// shut down the native audio system
void AudioRender::SlesShutdown()
{
	// destroy buffer queue audio player object, and invalidate all associated interfaces
	if (bqPlayerObject != NULL) {
		(*bqPlayerObject)->Destroy(bqPlayerObject);
		bqPlayerObject = NULL;
		bqPlayerPlay = NULL;
		bqPlayerBufferQueue = NULL;
		bqPlayerEffectSend = NULL;
		bqPlayerMuteSolo = NULL;
		bqPlayerVolume = NULL;
	}

	// destroy output mix object, and invalidate all associated interfaces
    /*
	if (outputMixObject != NULL) {
		(*outputMixObject)->Destroy(outputMixObject);
		outputMixObject = NULL;
		outputMixEnvironmentalReverb = NULL;
	}

	// destroy engine object, and invalidate all associated interfaces
	if (engineObject != NULL) {
		(*engineObject)->Destroy(engineObject);
		engineObject = NULL;
		engineEngine = NULL;
	}*/
}


// create the engine and output mix objects
int AudioRender::SlesCreateEngine( )
{
    comn::AutoCritSec lock(m_csDevice);
    if(g_bAudioRenderInit)
        return 0;
	SLresult result;

	// create engine
	result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
	if (SL_RESULT_SUCCESS != result) {
		SlesShutdown();
		return -1;
	}

	// realize the engine
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
	if (SL_RESULT_SUCCESS != result) {
		SlesShutdown();
		return -1;
	}

	// get the engine interface, which is needed in order to create other objects
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
	if (SL_RESULT_SUCCESS != result) {
		SlesShutdown();
		return -1;
	}

	// create output mix, with environmental reverb specified as a non-required interface
	const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
	if (SL_RESULT_SUCCESS != result) {
		SlesShutdown();
		return -1;
	}

	// realize the output mix
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
	if (SL_RESULT_SUCCESS != result) {
		SlesShutdown();
		return -1;
	}

	

	// get the environmental reverb interface
	// this could fail if the environmental reverb effect is not available,
	// either because the feature is not present, excessive CPU load, or
	// the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
	result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
		&outputMixEnvironmentalReverb);
	if (SL_RESULT_SUCCESS == result) {
		result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
			outputMixEnvironmentalReverb, &reverbSettings);
	}
	// ignore unsuccessful result codes for environmental reverb, as it is optional for this example

    g_bAudioRenderInit = true;
	return 0;
}

// create buffer queue audio player
int AudioRender::SlesCreateBQPlayer(
						int rate, 
						int nChannel, 
						int bitsPerSample )
{
    comn::AutoCritSec autolock(m_cs);
    SLresult result;
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm;
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = nChannel;
    format_pcm.samplesPerSec = rate * 1000; //SL_SAMPLINGRATE_22_05;
    format_pcm.bitsPerSample = bitsPerSample;
    format_pcm.containerSize = bitsPerSample;

    if( nChannel == 2 ){
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	}else{
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
	}
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	// create audio player
	const SLInterfaceID ids[3] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_EFFECTSEND,
		/*SL_IID_MUTESOLO,*/ SL_IID_VOLUME/*,SL_IID_PLAYBACKRATE,SL_IID_ANDROIDCONFIGURATION*/};
	const SLboolean req[3] = {/*SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,*/
		SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE/*,SL_BOOLEAN_TRUE*/};
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
		3, ids, req);
	if (SL_RESULT_SUCCESS != result) goto fail;
    
//    
//    SLAndroidConfigurationItf player_config;
//    result = (*bqPlayerObject)->GetInterface(bqPlayerObject,SL_IID_ANDROIDCONFIGURATION,&player_config);
//
//    // Set audio player configuration to SL_ANDROID_STREAM_VOICE which
//    // corresponds to android.media.AudioManager.STREAM_VOICE_CALL.
//    SLint32 stream_type = SL_ANDROID_STREAM_SYSTEM;
//    (*player_config)->SetConfiguration(player_config, SL_ANDROID_KEY_STREAM_TYPE,
//                                       &stream_type, sizeof(SLint32)),

    
    
	// realize the player
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
	if (SL_RESULT_SUCCESS != result) goto fail;

	// get the play interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
	if (SL_RESULT_SUCCESS != result) goto fail;
    
//    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAYBACKRATE, &bqPlayerPlaybackRate);
//    if (SL_RESULT_SUCCESS != result)
//    {
//        JPLAYER_LOG_ERROR("%s bqPlayerObject GetInterface SL_IID_PLAYBACKRATE fail\n",TAGAUDIORENDER);
//    }

	// get the buffer queue interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
		&bqPlayerBufferQueue);
	if (SL_RESULT_SUCCESS != result) goto fail;

	// register callback on the buffer queue
	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, 
		bqPlayerCallback, this);
	if (SL_RESULT_SUCCESS != result) goto fail;
	// get the effect send interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
		&bqPlayerEffectSend);
	if (SL_RESULT_SUCCESS != result) {
		//SlesShutdown();
		//return -1;
	}

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
	// get the mute/solo interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
	assert(SL_RESULT_SUCCESS == result);
#endif

	// get the volume interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    if (SL_RESULT_SUCCESS != result) goto fail;

	result = (*bqPlayerVolume)->SetMute( bqPlayerVolume, SL_BOOLEAN_FALSE );
//    if(bqPlayerPlaybackRate)
//    {
//        result = (*bqPlayerPlaybackRate)->SetPropertyConstraints(bqPlayerPlaybackRate,SL_RATEPROP_NOPITCHCORAUDIO);
//        if(result == SL_RESULT_PARAMETER_INVALID)
//            JPLAYER_LOG_DEBUG("%s opensl SetPropertyConstraints SL_RESULT_PARAMETER_INVALID \n",TAGAUDIORENDER);
//        if(result == SL_RESULT_FEATURE_UNSUPPORTED)
//            JPLAYER_LOG_DEBUG("%s opensl SetPropertyConstraints SL_RESULT_FEATURE_UNSUPPORTED \n",TAGAUDIORENDER);
//        if(result == SL_RESULT_SUCCESS)
//            JPLAYER_LOG_DEBUG("%s opensl SetPropertyConstraints sucess \n",TAGAUDIORENDER);
//    }
    
	//    assert(SL_RESULT_SUCCESS == result);

//	SLmillibel level;
//	result = (*bqPlayerVolume)->GetVolumeLevel( bqPlayerVolume, &level );


	/*// set the player's state to playing
	result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	if (SL_RESULT_SUCCESS != result) 
	{
		SlesShutdown();
		return -1;
	}*/

	return 0;
fail:
    SlesShutdown();
    return -1;
}


int AudioRender::BwAudioCreate(
					 int rate, 
					 int nChannel, 
					 int bitsPerSample)
{
    int ret = 0;
    m_needExit = false;
    if(!g_bAudioRenderInit){
        ret = SlesCreateEngine();
        if(ret == 0){
            g_bAudioRenderInit = true;
        }
    }
	if (ret == 0){
        if(nChannel > 2){
            nChannel = 2;
        }
        nb = ms_cached * rate * nChannel * bitsPerSample / 8000;
        bytesperms = rate * nChannel * bitsPerSample / 8000;
		ret = SlesCreateBQPlayer(rate, nChannel, bitsPerSample);
	}

    return ret;
}
    
//int AudioRender::BwAudioTimeCached()
//{
//    int cached = 0;
//    if(bytesperms){
//        cached += bytes_cached / bytesperms;
//    }
//    return cached;
//
//}

int AudioRender::BwAudioStart()
{
	if (bqPlayerPlay != NULL) {
        SLresult result;
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        if (SL_RESULT_SUCCESS == result) {
            return 0;
        }
	}
	return -1;
}

void AudioRender::BwAudioPause()
{
     comn::AutoCritSec autolock(m_cs);
    if (bqPlayerPlay != NULL) {
        SLresult result;
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
        if (SL_RESULT_SUCCESS == result) {
            return;
        }
    }
}

void AudioRender::BwAudioStop()
{
	if (bqPlayerPlay != NULL){
		SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
		if (SL_RESULT_SUCCESS == result) {
            if (bqPlayerBufferQueue != NULL)
                (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
            lock_my(&lck);
            m_needExit = true;
            free_buffer(&used_by_sl);
            free_buffer(&head);
            bytes_cached = 0;
            //m_fvolume = AUDIO_DEFAULT_VALUE;
            unlock_my(&lck);
			return ;
		}
	}
}


int AudioRender::BwAudioWrite( const void* buffer, int size)
{
    lock_my(&lck);
    if (list_empty(&used_by_sl)) {
        if (-1 == sles_startup()) {
            unlock_my(&lck);
            return -1;
        }
    }
    
    if (buffer != NULL) {
        struct my_buffer* mbuf = mbuf_alloc_2(size);
        memcpy(mbuf->ptr[0], buffer, size);
        list_add_tail(&mbuf->head, &head);
        bytes_cached += mbuf->length;
    }
    
    int n = 0;
    if (bytes_cached < nb) {
        n = nb - bytes_cached;
    }
    
    unlock_my(&lck);
    return n;
}

void AudioRender::BwAudioDestroy()
{
    comn::AutoCritSec autolock(m_cs);
    SlesShutdown();
    lock_my(&lck);
    free_buffer(&used_by_sl);
    unlock_my(&lck);
}

//int AudioRender::BwAudioGetVolume( float* left)
//{
//    *left = m_fLastVol;
//    return 0;
//}

int AudioRender::BwAudioSetVolume( float left)
{
    //according to the doc, the volume ranges from [[SL_MILLIBEL_MIN, GetMaxVolumeLevel()].
    //so we can use a normalized scalar [0,1] to set the volume.
    comn::AutoCritSec autolock(m_cs);
    if ( bqPlayerVolume != NULL){
        //get min & max
        SLmillibel MinVolume = SL_MILLIBEL_MIN;
        SLmillibel MaxVolume = SL_MILLIBEL_MIN;
        //(*bqPlayerVolume)->GetMaxVolumeLevel(bqPlayerVolume, &MaxVolume);
        
        //calc SLES volume
        //SLmillibel Volume = MinVolume + (SLmillibel)( ((float)(MaxVolume - MinVolume))*left );
        SLmillibel Volume;
        if(left < 0.00000001f)
            Volume = SL_MILLIBEL_MIN;
        else{
            Volume = lroundf(2000.f * log10f(left));
            if(Volume < SL_MILLIBEL_MIN )
                Volume = SL_MILLIBEL_MIN;
            else if(Volume > 0)
                Volume = 0;
        }

        SLresult result = (*bqPlayerVolume)->SetVolumeLevel( bqPlayerVolume, Volume );
        if(SL_RESULT_SUCCESS == result){
            m_fLastVol = left;
            return 0;
        }
    }else{
        m_fVolume = left;
    }
    return -1;
}

//void AudioRender::BwSetAM(VEAudioManager *pAM)
//{
//    m_pAM = pAM;
//}

//void AudioRender::BwAudioSpeed(float rate)
//{
//    SLresult result;
//    rate = rate * 1000;
//    if(bqPlayerPlaybackRate){
//        result = (*bqPlayerPlaybackRate)->SetRate(bqPlayerPlaybackRate,rate);
//        if(result != SL_RESULT_SUCCESS){
//            JPLAYER_LOG_INFO("%s opensl SetRate failed \n",TAGAUDIORENDER);
//        }
//    }else{
//        JPLAYER_LOG_INFO("%s opensl bqPlayerPlaybackRate nil \n",TAGAUDIORENDER);
//    }
//}

#endif


