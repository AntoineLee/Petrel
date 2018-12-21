#include "VEAudioRender.h"
#include "VEPlayerLogs.h"
#include "TCriticalSection.h"
#include "mydef.h"
#include "my_buffer.h"
#include "list_head.h"
#include "lock.h"
#include "mbuf.h"

#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioUnit/AudioUnit.h>
#import <UIKit/UIKit.h>
//#include "VEPlayerUtils.h"

#define TAGAUDIORENDER "[AudioRender]"

class AudioProcess
{
public:
    AudioProcess()
    {
        feeded = 0;
        needed = 0;
        m_fVolume = AUDIO_DEFAULT_VALUE;
        m_needExit = 0;
        lck = lock_initial;
        m_auIO = NULL;
        INIT_LIST_HEAD(&head);
    }
    ~AudioProcess()
    {
        lock_my(&lck);
        free_buffer(&head);
        feeded = 0;
        m_needExit = 1;
        unlock_my(&lck);
    }
    
    void setPara(int rate,int nChannel,int bitsPerSample);
    void initAudioSession();
    bool initAudioUnit();
    void close();
    //void clean();
    //void cleanBuffer();
    
    bool openAudioFromQueue(unsigned char* data, int dataSize);
    int  getAudioBuffer(uint8_t *buf,int len);
    
    bool playSound();
    bool pauseSound();
    bool stopSound();
    
    void setVolume(float v);
    //void getVolume(float *v);
    //void setRate(float v);
    //static int createDevice();
    //void reset();
    
    intptr_t audio_feed(void *ptr);
    uintptr_t needed, feeded;
    uintptr_t ios_bytesperms;
    
    //int updataQueueBuffer();
    //void cleanQueueBuffer();
    
private:
    AudioUnit m_auIO;
    int m_rate;
    int m_nChannel;
    int m_bitsPerSample;
    comn::CriticalSection m_csPlaySound;
    float m_fVolume;
    int m_needExit;
    
    list_head head;
    lock_t lck;
};

static OSStatus OnGetPlayoutData(void* in_ref_con,
                                 AudioUnitRenderActionFlags* flags,
                                 const AudioTimeStamp* time_stamp,
                                 UInt32 bus_number,
                                 UInt32 num_frames,
                                 AudioBufferList* io_data)
{
    AudioProcess *ap = (AudioProcess *) in_ref_con;
    if(ap == NULL || io_data == NULL)
        return noErr;
    int i = 0;
    for(i;i<(int)io_data->mNumberBuffers;i++){
        AudioBuffer *ioBuffer = &io_data->mBuffers[i];
        ap->getAudioBuffer((uint8_t *)ioBuffer->mData, ioBuffer->mDataByteSize);
    }
    
    return noErr;
}

void AudioProcess::setPara(int rate,
             int nChannel,
             int bitsPerSample)
{
    m_rate = rate;
    m_nChannel = nChannel;
    m_bitsPerSample = bitsPerSample;
    
    intptr_t ms = 160;
    ios_bytesperms = rate * nChannel * bitsPerSample / 8000;
    needed = ms * rate * nChannel * bitsPerSample / 8000;
    return;
}

void AudioProcess::initAudioSession()
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    if (audioSession != nil){
        if([audioSession setCategory:AVAudioSessionCategoryPlayback withOptions:AVAudioSessionCategoryOptionMixWithOthers error:nil]== NO){
            PREVIEW_LOG_WARN("render session setCategory failed \n");
        }
        
        if([audioSession setActive:YES error:nil] == NO){
            PREVIEW_LOG_WARN("render session setActive failed \n");
        }
        [audioSession setPreferredIOBufferDuration:0.03 error:nil];
    }
}

//int AudioProcess::createDevice()
//{
//    return 0;
//}

bool AudioProcess::initAudioUnit()
{
    comn::AutoCritSec lock(m_csPlaySound);
    
    initAudioSession();
    
    // Create an audio component description to identify the Voice Processing
    // I/O audio unit.
    AudioComponentDescription io_unit_description;
    io_unit_description.componentType = kAudioUnitType_Output;
    io_unit_description.componentSubType = kAudioUnitSubType_RemoteIO;
    io_unit_description.componentManufacturer = kAudioUnitManufacturer_Apple;
    io_unit_description.componentFlags = 0;
    io_unit_description.componentFlagsMask = 0;
    
    // Obtain an audio unit instance given the description.
    AudioComponent found_vpio_unit_ref = AudioComponentFindNext(nullptr, &io_unit_description);
    
    // Create a Voice Processing IO audio unit.
    OSStatus result = noErr;
    result = AudioComponentInstanceNew(found_vpio_unit_ref, &m_auIO);
    if (result != noErr){
        m_auIO = nullptr;
        return false;
    }

    // Enable output on the output scope of the output element.
    UInt32 enable_output = 1;
    result = AudioUnitSetProperty(m_auIO, kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output, 0,
                                  &enable_output, sizeof(enable_output));
    if (result != noErr){
        return false;
    }
    
    AudioStreamBasicDescription inputformat;
    memset(&inputformat, 0, sizeof(inputformat));
    inputformat.mSampleRate = m_rate;
    inputformat.mFormatID = kAudioFormatLinearPCM;
    inputformat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;//kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    inputformat.mFramesPerPacket = 1;
    inputformat.mChannelsPerFrame = m_nChannel;
    inputformat.mBytesPerFrame   =  2 * m_nChannel;
    inputformat.mBytesPerPacket = 2 * m_nChannel;
    inputformat.mBitsPerChannel = 16;
    
    result = AudioUnitSetProperty(m_auIO, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &inputformat, sizeof(inputformat));
    if(result != noErr){
        return false;
    }
    
    // Specify the callback function that provides audio samples to the audio
    // unit.
    AURenderCallbackStruct render_callback;
    render_callback.inputProc = OnGetPlayoutData;
    render_callback.inputProcRefCon = this;
    result = AudioUnitSetProperty(
                                  m_auIO, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
                                  0, &render_callback, sizeof(render_callback));
    if (result != noErr){
        return false;
    }
    
    result = AudioUnitInitialize(m_auIO);
    if(result != noErr){
        PREVIEW_LOG_INFO("AduioUnitInitialize failed \n");
    }
    
    return true;
}

//void AudioProcess::cleanBuffer()
//{
//    lock_my(&lck);
//    free_buffer(&head);
//    feeded = 0;
//    unlock_my(&lck);
//}

int AudioProcess::getAudioBuffer(uint8_t *buf, int len)
{
    lock_my(&lck);
    int buf_pos = 0;
    int total = len;
    int buf_enough = 1;
    while (len > 0 && !m_needExit){
        struct my_buffer* mbuf = NULL;
        if (!list_empty(&head)){
            struct list_head* ent = head.next;
            mbuf = list_entry(ent, struct my_buffer, head);
            int mbuf_len = mbuf->length - mbuf->pos;
            uint8_t *mbufPtr = (uint8_t *)(mbuf->ptr[0] + mbuf->pos);
            uint8_t *bufPtr = buf + buf_pos;
            if(len >= mbuf_len){
                memcpy(bufPtr, mbufPtr, mbuf_len);
                feeded -= mbuf_len;
                len -= mbuf_len;
                buf_pos += mbuf_len;
                list_del(ent);
                mbuf->mop->free(mbuf);
            }else{
                memcpy(bufPtr, mbufPtr, len);
                feeded -= len;
                buf_pos += len;
                mbuf->pos += len;
                len = 0;
            }
        }else{
            buf_enough = 0;
            break;
        }
    }
    if(m_needExit || m_fVolume == 0.0 || !buf_enough)
    {
        memset(buf, 0, total);
    }
    unlock_my(&lck);
    
    return 0;
}

void AudioProcess::close()
{
    comn::AutoCritSec lock(m_csPlaySound);
    if(m_auIO){
        AURenderCallbackStruct render_callback;
        memset(&render_callback, 0, sizeof(render_callback));
        AudioUnitSetProperty(m_auIO, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
                             0, &render_callback, sizeof(render_callback));
        AudioComponentInstanceDispose(m_auIO);
        m_auIO = NULL;
    }
}

//void AudioProcess::clean()
//{
//
//}

//void AudioProcess::setRate(float v)
//{
//    // comn::AutoCritSec lock(m_csPlaySound);
//}

void AudioProcess::setVolume(float v)
{
    comn::AutoCritSec lock(m_csPlaySound);
    m_fVolume = v;
}

//void AudioProcess::getVolume(float *v)
//{
//    comn::AutoCritSec lock(m_csPlaySound);
//    *v = m_fVolume;
//}

//int AudioProcess::updataQueueBuffer()
//{
//    return 0;
//}

//void AudioProcess::cleanQueueBuffer()
//{
//}

//bool AudioProcess::openAudioFromQueue(unsigned char* data, int dataSize)
//{
//    comn::AutoCritSec lock(m_csPlaySound);
//
//    return true;
//}

intptr_t AudioProcess::audio_feed(void *ptr)
{
    lock_my(&lck);
    
    if (ptr != NULL){
        struct my_buffer* mbuf = (struct my_buffer *) ptr;
        mbuf->pos = 0;
        list_add_tail(&mbuf->head, &head);
        feeded += mbuf->length;
    }
    
    int n = 0;
    if (feeded < needed){
        n = needed - feeded;
    }
    
    unlock_my(&lck);
    
    return n;
}

bool AudioProcess::playSound()
{
    comn::AutoCritSec lock(m_csPlaySound);
    if(m_auIO == NULL) return false;
    
    m_needExit = 1;
    OSStatus ret = AudioOutputUnitStart(m_auIO);
    m_needExit = 0;
    if(ret != noErr) return false;
    
    return true;
}

bool AudioProcess::pauseSound()
{
    comn::AutoCritSec lock(m_csPlaySound);
    if(m_auIO == NULL) return false;
    m_needExit = 1;
    OSStatus ret = AudioOutputUnitStop(m_auIO);
    m_needExit = 0;
    if(ret != noErr) return false;

    return true;
}

bool AudioProcess::stopSound()
{
    comn::AutoCritSec lock(m_csPlaySound);
    if(m_auIO == NULL) return false;
    m_needExit = 1;
    OSStatus ret = AudioOutputUnitStop(m_auIO);
    if(ret != noErr) return false;
    
    return true;
}

//void AudioProcess::reset()
//{
//    close();
//    initAudioUnit();
//}


AudioRender::AudioRender()
{
    m_pAudioProcess = new AudioProcess();
}

AudioRender::~AudioRender()
{
    if(m_pAudioProcess){
        delete m_pAudioProcess;
        m_pAudioProcess = NULL;
    }
}


int AudioRender::BwAudioCreate(int rate,
                               int nChannel,
                               int bitsPerSample)
{
    m_pAudioProcess->setPara(rate, nChannel, bitsPerSample);
    bool ok = m_pAudioProcess->initAudioUnit();
    return ok ? 0 : -1;
}

void AudioRender::BwAudioDestroy()
{
    m_pAudioProcess->close();
}

int  AudioRender::BwAudioStart()
{
    bool ok = m_pAudioProcess->playSound();
    return ok ? 0 : -1;
}

int  AudioRender::BwAudioPause()
{
    bool ok = m_pAudioProcess->pauseSound();
    return ok ? 0 : -1;
}

void AudioRender::BwAudioStop()
{
    m_pAudioProcess->stopSound();
    return ;
}

int AudioRender::BwAudioWrite(const void* buffer, int size)
{
    int bytes = 0;
    if(buffer && size){
        struct my_buffer* mbuf = mbuf_alloc_2(size);
        if(mbuf){
            memcpy(mbuf->ptr[0], buffer, size);
            bytes = (int) m_pAudioProcess->audio_feed(mbuf);
        }
    }else{
        bytes = (int) m_pAudioProcess->audio_feed(NULL);
    }
    
    return bytes;
}

//int AudioRender::BwAudioTimeCached()
//{
//    int cached = 0;
//
//    if(m_pAudioProcess->ios_bytesperms){
//        m_pAudioProcess->updataQueueBuffer();
//        cached += m_pAudioProcess->feeded / m_pAudioProcess->ios_bytesperms;
//    }
//
//    return cached;
//}


//int AudioRender::BwAudioWrite2(void* ptr)
//{
//    int bytes = 0;
//    bytes = (int) m_pAudioProcess->audio_feed(ptr);
//    return bytes;
//}

//int AudioRender::BwAudioGetVolume(float* left)
//{
//    m_pAudioProcess->getVolume(left);
//    return 0;
//}

int AudioRender::BwAudioSetVolume(float left)
{
    if(m_pAudioProcess)
        m_pAudioProcess->setVolume(left);
    return 0;
}

//void AudioRender::BwClean()
//{
//    m_pAudioProcess->clean();
//}

//void AudioRender::BwAudioSpeed(float rate)
//{
//    m_pAudioProcess->setRate(rate);
//}

//void AudioRender::BwCleanAudio()
//{
//    m_pAudioProcess->cleanBuffer();
//}

//void AudioRender::BwResetAudioRender()
//{
//    if(m_pAudioProcess)
//        m_pAudioProcess->reset();
//}

#endif



