#ifndef __VE_SOUNDTOUCH_H_
#define __VE_SOUNDTOUCH_H_

#include "SoundTouch/SoundTouch.h"
#include "SoundTouch/BPMDetect.h"

using namespace soundtouch;
class VESoundTouch
{
public:
    VESoundTouch() {}
    ~VESoundTouch() {}
    
    void configSoundTouchPitch(int value,int channles,int samplerate,float speed = 1.0);
    int  processData(uint8_t *pIn,int sizeIn,uint8_t *pOut,int *sizeOut);
    
private:
    SoundTouch m_soundTouch;
    int m_valuePitch;
    int m_channels;
    int m_sampleRate;
};

#endif

