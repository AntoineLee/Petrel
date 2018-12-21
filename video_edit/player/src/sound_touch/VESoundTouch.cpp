#include "VESoundTouch.h"



void VESoundTouch::configSoundTouchPitch(int value,int channles,int samplerate,float speed)
{
    //m_soundTouch.clear();
    m_soundTouch.setSampleRate(samplerate);
    m_soundTouch.setChannels(channles);
    m_soundTouch.setTempoChange(0);
    //m_soundTouch.setPitch(1.0);
    m_soundTouch.setPitchSemiTones(value);
    m_soundTouch.setRateChange(0);
    
//    m_soundTouch.setSetting(SETTING_USE_AA_FILTER, 1);
//    m_soundTouch.setSetting(SETTING_SEQUENCE_MS, 40);
//    m_soundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
//    m_soundTouch.setSetting(SETTING_OVERLAP_MS, 8);
    m_soundTouch.setRate(speed);
    m_valuePitch = value;
    m_channels = channles;
    m_sampleRate = samplerate;
}

int VESoundTouch::processData(uint8_t *pIn, int sizeIn, uint8_t *pOut, int *sizeOut)
{
    int samplesize = m_channels * 2;
    int nsamples = sizeIn / samplesize;
    m_soundTouch.putSamples((const SAMPLETYPE *)pIn, nsamples);
        
    int outlen = 0;
    do{
        nsamples = m_soundTouch.receiveSamples((SAMPLETYPE *)pOut, 1024 * 1024);
        outlen += nsamples * samplesize;
        pOut += nsamples * samplesize;
    }while(nsamples!=0);
    
    *sizeOut = outlen;
    if(outlen){
        return 1;
    }
    
    return 0;
}


