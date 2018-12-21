#include "VETestAudioProducer.hpp"
#include "ve_audio_producer.h"
#include "VETestUtils.hpp"
#include "VEConfig.h"
#include "VEUnitTestConfig.h"

int ve_test_ap_got_data = 0;
int ve_test_ap_got_status = 0;
int ve_test_ap_got_status_noaudio = 0;

void testDataCallback(HANDLE client_handle,unsigned char *pPCM,int size, void* userExt){
    if(pPCM && size)
        ve_test_ap_got_data = 1;
    return;
}

void testStatusCallback(HANDLE client_handle,int status,void* userExt){
    if(status == AP_STATUS_FILE_COMPLETE)
        ve_test_ap_got_status = 1;
    if(status == AP_STATUS_FILE_NO_AUDIO)
        ve_test_ap_got_status_noaudio = 1;
    return;
}


int testInterfaceAudioProducer(){
    {
        VEConfig config = getVEConfig1();
        HANDLE h = ve_audio_producer_create(&config, testDataCallback, testStatusCallback, NULL);
        VE_TEST_ASSERT(h != NULL)
        
        int track_id = 0;
        int clip_id  = 0;
        int start    = 0;
        int end      = 0;
        std::map<int,VETrackData>::iterator it = config.m_tracks.begin();
        for(it;it != config.m_tracks.end();it++){
            track_id = it->second.m_track.track_id;
            clip_id  = it->second.m_clips[0].m_clip.clip_id;
            start    = it->second.m_clips[0].m_clip.start_time;
            end      = it->second.m_clips[0].m_clip.end_time;
            break;
        }
        ve_audio_producer_start(h, track_id, clip_id, start, end);
        VE_TEST_SYNC(2000, ve_test_ap_got_status_noaudio)
        ve_audio_producer_free(h);
        if(ve_test_ap_got_status_noaudio == 0)
            return -1;
    }
    {
        VEConfig config = getVEConfig5();
        HANDLE h = ve_audio_producer_create(&config, testDataCallback, testStatusCallback, NULL);
        VE_TEST_ASSERT(h != NULL)
        
        int track_id = 0;
        int clip_id  = 0;
        int start    = 0;
        int end      = 0;
        std::map<int,VETrackData>::iterator it = config.m_tracks.begin();
        for(it;it != config.m_tracks.end();it++){
            if(it->second.m_track.type == VE_TRACK_AUDIO)
            {
                track_id = it->second.m_track.track_id;
                clip_id  = it->second.m_clips[0].m_clip.clip_id;
                start    = it->second.m_clips[0].m_clip.start_time;
                end      = it->second.m_clips[0].m_clip.end_time;
                break;
            }
        }
        
        ve_audio_producer_start(h, -100, clip_id, start, end);
        ve_audio_producer_start(h, track_id, clip_id, start, end);
        
        VE_TEST_SYNC(5000, ve_test_ap_got_status)
        
        ve_audio_producer_free(h);
        
        if(ve_test_ap_got_status && ve_test_ap_got_data)
            return 0;
        
        return -1;
    }
}
