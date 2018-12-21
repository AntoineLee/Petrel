#include "VTDecoder.h"
#include "libyuv.h"
#include "VECommon.h"
#include "VELog.h"

#define MAX_PACKET_DATA_SIZE 5*1024*1024
#define MAX_FRAME_INTERVAL 10 //MS
#define MAX_FRAME_NUMBER   7

struct my_node{
    bool operator()(const JPlayer_MediaFrame& t1,const JPlayer_MediaFrame& t2){
        return t1.m_timestamp<t2.m_timestamp;    //会产生升序排序,若改为>,则变为降序
    }
};

VTDecoder::VTDecoder()
{
    m_sps = m_vps =NULL;
    m_spsSize = m_vpsSize = 0;
    int i = 0;
    for(i;i<MAX_PPS_NUMS;i++){
        m_pps[i] = NULL;
        m_ppsSize[i] = 0;
    }
    m_ppsNums = 0;
    m_ready = 0;
    m_pTmpBuffer = (uint8_t *)malloc(MAX_PACKET_DATA_SIZE);
    m_pTmpRawBuffer = NULL;
    m_timestampEx = -1;
    m_decoderNum = 0;
    m_bHasBFrame = 1;
    m_bIsNeedRender = true;
    m_decoderFormatDescription = NULL;
    m_deocderSession = NULL;
    m_freeList.clear();
    m_useList.clear();
    m_codecType = VT_CODEC_TYPE_UNKNOWN;
    m_output = 1;
    m_lastOutput = m_output;
    m_async = 0;
    m_frameMode = 0;
    m_max_frame_num = MAX_FRAME_NUMBER;
}

VTDecoder::~VTDecoder()
{
    close();
    if(m_pTmpBuffer){
        free(m_pTmpBuffer);
        m_pTmpBuffer = NULL;
    }
    if(m_pTmpRawBuffer){
        free(m_pTmpRawBuffer);
        m_pTmpRawBuffer = NULL;
    }
    if(m_sps ){
        free(m_sps);
        m_sps = NULL;
        m_spsSize = 0;
    }
    int i = 0;
    for(i;i<MAX_PPS_NUMS;i++){
        if(m_pps[i]){
            free(m_pps[i]);
            m_pps[i] = NULL;
            m_ppsSize[i] = 0;
        }
    }
    if(m_vps){
        free(m_vps);
        m_vps = NULL;
        m_vpsSize = 0;
    }
}

void VTDecoder::close()
{
    comn::AutoCritSec lock(m_cs);
    //VE_LOG_INFO("VT Decoded total frame %lld \n",m_decoderNum);
    if(m_deocderSession) {
        VTDecompressionSessionWaitForAsynchronousFrames(m_deocderSession);
        VTDecompressionSessionInvalidate(m_deocderSession);
        CFRelease(m_deocderSession);
        m_deocderSession = NULL;
    }
    
    if(m_decoderFormatDescription) {
        CFRelease(m_decoderFormatDescription);
        m_decoderFormatDescription = NULL;
    }
    
    clearAllBuffers();
    m_timestampEx = -1;
    m_ready = 0;
}

void VTDecoder::reset()
{
    //VE_LOG_INFO("reset VT Decoder Config \n");
    if(m_deocderSession){
        VTDecompressionSessionWaitForAsynchronousFrames(m_deocderSession);
        VTDecompressionSessionInvalidate(m_deocderSession);
        CFRelease(m_deocderSession);
        m_deocderSession = NULL;
    }
    
    clearAllBuffers();
    m_timestampEx = -1;
    m_ready = 0;
}

void VTDecoder::clearAllBuffers()
{
    std::list<JPlayer_MediaFrame>::iterator it = m_freeList.begin();
    {
    	comn::AutoCritSec lock(m_freeList_cs);
		for(; it!= m_freeList.end();it++){
			if(it->m_pData[0] && it->m_flag != JPLAYER_FRAME_PIXELBUFFER){
				free(it->m_pData[0]);
			}
		}
		m_freeList.clear();
    }
    
    std::list<JPlayer_MediaFrame>::iterator it2 = m_useList.begin();
    {
        comn::AutoCritSec lock(m_useList_cs);
        for(; it2!= m_useList.end();it2++){
            if(it2->m_pData[0] && it2->m_flag == JPLAYER_FRAME_PIXELBUFFER){
                CVPixelBufferRelease(CVImageBufferRef(it2->m_pData[0]));
                it2->m_pData[0] = NULL;
            }else{
                free(it2->m_pData[0]);
            }
        }
        m_useList.clear();
    }
}

void VTDecoder::flushBuffers()
{
    comn::AutoCritSec lock(m_cs);
    VTDecompressionSessionWaitForAsynchronousFrames(m_deocderSession);
    //VE_LOG_INFO("vt decoder flush buffers \n");
    m_timestampEx = -1;
    JPlayer_MediaFrame frame;
    while (1){
        if(getUseNode(frame, -1) == 0){
            //VE_LOG_INFO("vt flush buffers get use node \n");
            if(m_bNeedPixelBuffer && frame.m_flag == JPLAYER_FRAME_PIXELBUFFER){
                CVPixelBufferRelease(CVImageBufferRef(frame.m_pData[0]));
                frame.m_pData[0] = NULL;
                //
            }
            addFreeNode(frame);
        }else{
            //VE_LOG_INFO("vt flush buffers complete \n");
            break;
        }
    }
}
void VTDecoder::set_needPixelBuffer(){
	m_bNeedPixelBuffer = true;
}
void VTDecoder::set_asyncMode(){
	m_async = 1;
}
void VTDecoder::set_ve_exportMode(){
	m_exportMode = 1;
}
void VTDecoder::set_renderless()
{
    m_bIsNeedRender = false;
}

int VTDecoder::initVTDecoder()
{
    CMVideoFormatDescriptionRef input_format = nullptr;

    OSStatus status = -1;
    if(m_codecType == VT_CODEC_TYPE_H264){
        int count = 1 + m_ppsNums;
        
        uint8_t **parameterSetPointers =(uint8_t**) malloc(count * sizeof(uint8_t *))/*{ _vps,_sps, _pps[0],_pps[1] }*/;
        size_t *parameterSetSizes = (size_t *)malloc(count * sizeof(size_t))/*{ _vpsSize,_spsSize, _ppsSize[0],_ppsSize[1] }*/;
        int i = 1;
        parameterSetPointers[0] = m_sps;
        parameterSetSizes[0] = m_spsSize;
        for(i ; i < count ; i++){
            parameterSetSizes[i] = m_ppsSize[i-1];
            parameterSetPointers[i] = m_pps[i-1];
        }
        status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                              count, //param count
                                                                              parameterSetPointers,
                                                                              parameterSetSizes,
                                                                              4,
                                                                              &input_format);
        free(parameterSetPointers);
        free(parameterSetSizes);
    }else if(m_codecType == VT_CODEC_TYPE_H265){
        int count = 2 + m_ppsNums;
        
         uint8_t **parameterSetPointers =(uint8_t**) malloc(count * sizeof(uint8_t *))/*{ _vps,_sps, _pps[0],_pps[1] }*/;
         size_t *parameterSetSizes = (size_t *)malloc(count * sizeof(size_t))/*{ _vpsSize,_spsSize, _ppsSize[0],_ppsSize[1] }*/;
        int i = 2;
        parameterSetPointers[0] = m_vps;
        parameterSetPointers[1] = m_sps;
        parameterSetSizes[0] = m_vpsSize;
        parameterSetSizes[1] = m_spsSize;
        for(i ; i < count ; i++){
            parameterSetSizes[i] = m_ppsSize[i-2];
            parameterSetPointers[i] = m_pps[i-2];
        }
        
        status = CMVideoFormatDescriptionCreateFromHEVCParameterSets(kCFAllocatorDefault,
                                                                     count, //param count
                                                                     parameterSetPointers,
                                                                     parameterSetSizes,
                                                                     4,
                                                                     NULL,
                                                                     &input_format);
        free(parameterSetPointers);
        free(parameterSetSizes);
    }
    
    if(status == noErr)
    {
        if (input_format)
        {
            if (!CMFormatDescriptionEqual(input_format, m_decoderFormatDescription) || m_deocderSession == NULL)
            {
                SetVideoFormat(input_format);
                reconfigure();
            }
            CFRelease(input_format);
        }
    } else
    {
        //VE_LOG_INFO("VT decoder CMVideoFormatDescriptionCreateFromH264ParameterSets failed \n");
        return -1;
    }
    
    return 0;
}


void VTDecoder::reconfigure()
{
    reset();
    if(m_sps && m_sps[1] == 66){
        m_bHasBFrame = 0;
    }
    
    CFDictionaryRef attrs = NULL;
    const void *keys[] = { kCVPixelBufferPixelFormatTypeKey };
    //      kCVPixelFormatType_420YpCbCr8Planar is YUV420
    //      kCVPixelFormatType_420YpCbCr8BiPlanarFullRange is NV12
    uint32_t v = /*kCVPixelFormatType_420YpCbCr8BiPlanarFullRange*/kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
    //uint32_t v = kCVPixelFormatType_420YpCbCr8Planar;
    const void *values[] = { CFNumberCreate(NULL, kCFNumberSInt32Type, &v) };
    attrs = CFDictionaryCreate(NULL, keys, values, 1, NULL, NULL);
    
    VTDecompressionOutputCallbackRecord callBackRecord;
    callBackRecord.decompressionOutputCallback = didDecompress;
    callBackRecord.decompressionOutputRefCon = this;
    
    OSStatus status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                          m_decoderFormatDescription,
                                          NULL, attrs,
                                          &callBackRecord,
                                          &m_deocderSession);
    CFRelease(attrs);

}

void VTDecoder::SetVideoFormat(CMVideoFormatDescriptionRef video_format) {
    if (m_decoderFormatDescription == video_format) {
        return;
    }
    if (m_decoderFormatDescription) {
        CFRelease(m_decoderFormatDescription);
    }
    m_decoderFormatDescription = video_format;
    if (m_decoderFormatDescription) {
        CFRetain(m_decoderFormatDescription);
    }
}

void VTDecoder::didDecompress(void *decompressionOutputRefCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef pixelBuffer, CMTime presentationTimeStamp, CMTime presentationDuration )
{
    if(!pixelBuffer){
        //VE_LOG_INFO("VT pixelbuffer is nil skip \n");
        return ;
    }
    
    CVBufferRemoveAllAttachments(pixelBuffer); //lyue:取消pixelbuffer附加属性
    //CVPixelBufferRef *outputPixelBuffer = (CVPixelBufferRef *)sourceFrameRefCon;
    CVPixelBufferRetain(pixelBuffer);
    uint64_t timestamp = (uint64_t)(CMTimeGetSeconds(presentationTimeStamp)*1000);
    uint64_t duration = (uint64_t)(CMTimeGetSeconds(presentationDuration)*1000);
    

   // CMTimeGetSeconds(CMTime time)
    ////VE_LOG_INFO("vt output pts = %lld duration = %lld \n",timestamp,duration);
    
    if(pixelBuffer){
        VTDecoder *pVT = (VTDecoder *)decompressionOutputRefCon;
        pVT->addPixelBuffer(pixelBuffer, timestamp,duration);
        
        if(!pVT->m_bNeedPixelBuffer){
        	CVPixelBufferRelease(pixelBuffer);
        }
        ////VE_LOG_INFO("leave inputpacket time = %llu \n",timestamp);
        return;
    }
}

int VTDecoder::addPixelBuffer(CVImageBufferRef pixelBuffer,uint64_t timestamp,uint64_t duration)
{
    size_t width = 0;
    size_t height = 0;
    size_t count = 0;
    size_t bytesperrow = 0;
    void *base_y;
    void *base_uv;
    
    int64_t start_time = av_gettime();


    CVPixelBufferLockBaseAddress(pixelBuffer, 1 );

    count = CVPixelBufferGetPlaneCount(pixelBuffer);
    base_y = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
    base_uv = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);

    width = CVPixelBufferGetWidthOfPlane(pixelBuffer, 0);
    height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 0);
    bytesperrow = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0);

    
    m_decoderNum ++;
    
    if(m_freeList.empty()){
        for(int i=0;i<m_max_frame_num;i++){
            uint8_t *pTmpBuffer =  NULL;
            if(!m_bNeedPixelBuffer){
				pTmpBuffer = (uint8_t *)malloc(bytesperrow * height *3 /2);
				memset(pTmpBuffer, 0, bytesperrow *height * 3 / 2);
            }
            JPlayer_MediaFrame f;
            f.m_pData[0] = pTmpBuffer;
            addFreeNode(f);
        }

    }

    JPlayer_MediaFrame frame;
    if(getFreeNode(frame) == 0){
//        if(m_bIsNeedRender)
//        {
////            memcpy(frame.m_pData[0], (uint8_t *)base_y, bytesperrow*height);
////            splitUV((unsigned char *)base_uv, frame.m_pData[0]+bytesperrow*height, bytesperrow/2, height/2);
//
//            int ret = libyuv::NV21ToI420((uint8_t *)base_y, bytesperrow,
//                                         (uint8_t *)base_uv, bytesperrow,
//                                         frame.m_pData[0], bytesperrow,
//                                         frame.m_pData[0] + bytesperrow * height * 5 / 4, bytesperrow / 2,
//                                         frame.m_pData[0] + bytesperrow * height, bytesperrow / 2,
//                                         width, height);
//            if (0 != ret)
//            {
//                CVPixelBufferUnlockBaseAddress(pixelBuffer, 1);
//                VE_LOG_WARN("VT Decoder NV12ToI420 Failed \n");
//                return -1;
//            }
//            frame.m_flag = JPLAYER_FRAME_YUV420P;
//        }
//        else
        {
        	if(m_bNeedPixelBuffer){
        		frame.m_pData[0] = (uint8_t*)pixelBuffer;
        		frame.m_flag = JPLAYER_FRAME_PIXELBUFFER;
        	}/*else{
				memcpy(frame.m_pData[0], (uint8_t *)base_y, bytesperrow*height);
				memcpy(frame.m_pData[0] + bytesperrow * height, (uint8_t *)base_uv, bytesperrow * height /2);
				frame.m_size[0] = frame.m_size[1] = bytesperrow;
				frame.m_flag = JPLAYER_FRAME_NV12;
        	}*/
        }
        
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 1);


        //frame.m_flag      = JPLAYER_FRAME_YUV420P;
        frame.m_height    = height;
        frame.m_width     = width;
        //frame.m_pData[0]  = _pTmpBuffer;
//        if(frame.m_flag == JPLAYER_FRAME_YUV420P){
//            frame.m_pData[1]  = frame.m_pData[0] + bytesperrow * height;
//            frame.m_pData[2]  = frame.m_pData[0] + bytesperrow * height + bytesperrow/2 * height/2;
//            frame.m_size[0]   = bytesperrow;
//            frame.m_size[1]   = bytesperrow/2;
//            frame.m_size[2]   = bytesperrow/2;
//        }else if(frame.m_flag == JPLAYER_FRAME_NV12){
//            frame.m_pData[1]  = frame.m_pData[0] + bytesperrow * height;
//            frame.m_size[0]   = bytesperrow;
//            frame.m_size[1]   = bytesperrow;
//        }
        frame.m_timestamp = timestamp;
        frame.m_duration = duration;
        frame.m_dts = 0;
        frame.m_sar_den = 0;
        frame.m_sar_num = 0;
        addUseNode(frame);

        //VE_LOG_INFO("NV21ToI420:%lld",(av_gettime() - start_time));
        return 0;
        //frame.m_serialNum = serialNum;
    }else{
    	CVPixelBufferUnlockBaseAddress(pixelBuffer, 1);
    }
    return -1;

}

const uint8_t KStartCode_04[4] = { 0, 0, 0, 1};
const uint8_t KStartCode_03[3] = { 0, 0, 1};
size_t VTDecoder::splitPacket(unsigned char *p, size_t size)
{
    if(size >=5){
//        int start_index = 0;
//        if(memcmp(p,KStartCode_03,3) == 0)
//            start_index = 3;
//        if (memcmp(p, KStartCode_04, 4) == 0)
//            start_index = 4;
//        
//        if((p[start_index] & 0x1F) == 0x05 || (p[start_index] & 0x1F) == 0x01)
//            return size;
        uint8_t *pStart = p +4;
        uint8_t *pEnd = p + size;
        while (pStart != pEnd){
            
            if(*pStart == 0x01) {
                if(memcmp(pStart - 3, KStartCode_04, 4) == 0) {
                    size_t packetSize = pStart - p - 3;
                    return packetSize;
                }
                else if(memcmp(pStart - 2, KStartCode_03, 3) == 0) {
                    size_t packetSize = pStart -p - 2;
                    return packetSize;
                }
            }
            pStart ++;
        }
    }
    else
        return size;
        
    return size;
}

int VTDecoder::inputPacket(unsigned char *pInBuffer,
                size_t size,
                uint64_t timestamp,
                uint64_t serialNum,int isKey,uint64_t dts,uint64_t duration,JPlayer_MediaFrame &frame)
{
//    if(!isAppInActive())
//    {
//        VE_LOG_WARN("ios hw app is not active \n");
//        if(_deocderSession)
//            close();
//        return -1;
//    }
    comn::AutoCritSec lock(m_cs);
    uint8_t *pInBuffer2 = pInBuffer;
    uint8_t *pInbufferEnd = pInBuffer + size;
    size_t size2 = size;
    CVPixelBufferRef pixelBuffer = NULL;
    size_t packet_size = 0;
    uint8_t *pTmp = NULL;
    int mslices = 0;
    int tmp_size = 0;
    int pps_index = 0;
    int pps_nums = 0;
    
    ////VE_LOG_INFO("enter inputpacket pts = %lld dts = %lld \n",timestamp,dts);
    while(pInBuffer2 != pInbufferEnd){
        packet_size = splitPacket(pInBuffer2, size2);
        size2 -= packet_size;
        pTmp = pInBuffer2;
        pInBuffer2 += packet_size;
        
        if(pTmp[2] == 1 && packet_size < MAX_PACKET_DATA_SIZE){
           // //VE_LOG_INFO("sb inputpacket \n");
            if(!m_pTmpRawBuffer){
                m_pTmpRawBuffer = (uint8_t *)malloc(MAX_PACKET_DATA_SIZE);
                m_pTmpRawBuffer[0] = 0;
            }
            
            memmove(m_pTmpRawBuffer + 1, pTmp, packet_size);
            //pInBuffer2[0] = 0;
            packet_size = packet_size + 1;
            pTmp = m_pTmpRawBuffer;
        }

        uint32_t nalSize = (uint32_t)(packet_size - 4);
        uint8_t *pNalSize = (uint8_t*)(&nalSize);
        pTmp[0] = *(pNalSize + 3);
        pTmp[1] = *(pNalSize + 2);
        pTmp[2] = *(pNalSize + 1);
        pTmp[3] = *(pNalSize);
        
        int nalType = getNaluType(pTmp[4]);
        int segType = getSegmentType(nalType);
        int nal_idc = 1;
        if(m_frameMode && segType == VT_NALU_TYPE_NONIDR){
            if(m_codecType == VT_CODEC_TYPE_H264)
                nal_idc = pTmp[4] & 0x60;
            if(m_codecType == VT_CODEC_TYPE_H265)
                nal_idc = nalType % 2;
        }
        
        if(pInBuffer2 == pInbufferEnd && packet_size<= 4){
        	segType = VT_NALU_TYPE_NONIDR;
            packet_size = 0;
        }
        switch (segType) {
            case VT_NALU_TYPE_IDR:
                if(initVTDecoder() == 0) {
                    m_ready = 1;
                    
                    if(pInBuffer2 != pInbufferEnd || tmp_size){
                        mslices = 1;
                        memcpy(m_pTmpBuffer + tmp_size, pTmp, packet_size);
                        tmp_size += packet_size;
                        if(pInBuffer2 != pInbufferEnd)
                            break;
                    }
                    
                    int finalsize = tmp_size ? tmp_size : packet_size;
                    uint8_t *pData = tmp_size ? m_pTmpBuffer : pTmp;
                    pixelBuffer = decode(pData, finalsize,dts,duration,timestamp);
                }
                break;
            case VT_NALU_TYPE_NONIDR:
            {
                if(m_ready && nal_idc){
                    if(pInBuffer2 != pInbufferEnd || tmp_size){
                        mslices = 1;
                        memcpy(m_pTmpBuffer + tmp_size, pTmp, packet_size);
                        tmp_size += packet_size;
                        if(pInBuffer2 != pInbufferEnd)
                            break;
                    }
                    int finalsize = tmp_size ? tmp_size : packet_size;
                    uint8_t *pData = tmp_size ? m_pTmpBuffer : pTmp;
                    pixelBuffer = decode(pData,finalsize ,dts,duration,timestamp);
                }

                break;
            }
            case VT_NALU_TYPE_SPS:
                //VE_LOG_INFO("ios hw coming sps \n");
                //if(!_sps)
                {
                    free(m_sps);
                    m_spsSize = packet_size - 4;
                    m_sps = (uint8_t *)malloc(m_spsSize);
                    //VE_LOG_INFO("ios hw coming spsSize=%d \n",m_spsSize);
                    memcpy(m_sps, pTmp + 4, m_spsSize);
                }
                break;
            case VT_NALU_TYPE_PPS:
                //VE_LOG_INFO("ios hw coming pps\n");
                if(m_pps[pps_index])
                {
                    free(m_pps[pps_index]);
                    m_pps[pps_index] = NULL;
                }
                m_ppsSize[pps_index] = packet_size - 4;
                //VE_LOG_INFO("ios hw coming ppsSize=%d \n",m_ppsSize[pps_index] );
                m_pps[pps_index] = (uint8_t *)malloc(m_ppsSize[pps_index]);
                memcpy(m_pps[pps_index], pTmp + 4, m_ppsSize[pps_index]);
                pps_index++;
                pps_nums ++;
                m_ppsNums = pps_nums;
                break;
            case VT_NALU_TYPE_VPS:
                //VE_LOG_INFO("ios hw coming vps \n");
                //if(!_vps)
                {
					free(m_vps);
					m_vpsSize = packet_size - 4;
					//VE_LOG_INFO("ios hw coming vpsSize=%d \n",m_vpsSize);
					m_vps = (uint8_t *)malloc(m_vpsSize);
					memcpy(m_vps, pTmp + 4, m_vpsSize);
                }
                break;
            default:
                //NSLog(@"Nal type is B/P frame");
                if(m_ready) {
                    if(tmp_size)
                        pixelBuffer = decode(m_pTmpBuffer, tmp_size,dts,duration,timestamp);
                }else{
                    //VE_LOG_INFO("ios hw coming unkown video frame \n");
                }
                break;
        }
    }
    
    if(pInBuffer == NULL){
        m_timestampEx = -1;
        m_exportMode = 0;
        VTDecompressionSessionWaitForAsynchronousFrames(m_deocderSession);
    }

    if(getUseNode(frame, m_timestampEx) == 0){
        JPlayer_MediaFrame freeFrame = frame;
        ////VE_LOG_INFO("getUseNode frame.m_timestamp=%lld \n",frame.m_timestamp);
        if(m_bHasBFrame)
            m_timestampEx = frame.m_timestamp + frame.m_duration;
        if(JPLAYER_FRAME_PIXELBUFFER == freeFrame.m_flag){
            freeFrame.m_pData[0] = NULL;
            freeFrame.m_flag = JPLAYER_FRAME_YUV420P;
        }
        addFreeNode(freeFrame);
        return 0;
    }
    
    
    return -1;
}

CVPixelBufferRef VTDecoder::decode(uint8_t *pBuffer, size_t size,uint64_t dts,uint64_t duration,uint64_t pts)
{
    
    CVPixelBufferRef outputPixelBuffer = NULL;
    CMBlockBufferRef blockBuffer = NULL;
    
    
    // add code by zhengang
    CMSampleTimingInfo timingInfo;
    timingInfo.presentationTimeStamp = CMTimeMake(pts, 1000);
    timingInfo.duration = CMTimeMake(duration, 1000);
    timingInfo.decodeTimeStamp = CMTimeMake(dts, 1000);
    // end code by zhengang
    
    OSStatus status  = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                                          (void*)pBuffer, size,
                                                          kCFAllocatorNull,
                                                          NULL, 0, size,
                                                          0, &blockBuffer);
    if(status == kCMBlockBufferNoErr){
        CMSampleBufferRef sampleBuffer = NULL;
        const size_t sampleSizeArray[] = {size};
  
        status = CMSampleBufferCreateReady(kCFAllocatorDefault,
                                           blockBuffer,
                                           m_decoderFormatDescription ,
                                           1, 1, &timingInfo, 1, sampleSizeArray,
                                           &sampleBuffer);
        if (status == kCMBlockBufferNoErr && sampleBuffer){
            VTDecodeFrameFlags flags = 0;
            VTDecodeInfoFlags flagOut = 0;
            flags |= kVTDecodeFrame_EnableAsynchronousDecompression;
            
            if(m_async){
                flags |= kVTDecodeFrame_EnableAsynchronousDecompression;
                if(m_output == 0)
                    flags |= kVTDecodeFrame_DoNotOutputFrame;
                //VTDecompressionSessionWaitForAsynchronousFrames(_deocderSession);
            }else if(m_output == 0)
            {
                flags |= kVTDecodeFrame_DoNotOutputFrame;
                flags |= kVTDecodeFrame_EnableAsynchronousDecompression;
            }
            else if(m_output != m_lastOutput)
                VTDecompressionSessionWaitForAsynchronousFrames(m_deocderSession);
            
            m_lastOutput = m_output;

            OSStatus decodeStatus = VTDecompressionSessionDecodeFrame(m_deocderSession,
                                                                      sampleBuffer,
                                                                      flags,
                                                                      &outputPixelBuffer,
                                                                      &flagOut);
            
            
            if(decodeStatus == kVTInvalidSessionErr){
                VE_LOG_WARN("VT decoder failed kVTInvalidSessionErr \n");
                reset();
            }else if(decodeStatus == kVTVideoDecoderBadDataErr){
                VE_LOG_WARN("VT decoder failed kVTVideoDecoderBadDataErr \n");
            }else if(decodeStatus != noErr){
                VE_LOG_WARN("VT decoder failed err = %d \n",decodeStatus);
            }
            
            CFRelease(sampleBuffer);
        }
        CFRelease(blockBuffer);
    }
    
    return outputPixelBuffer;
}


void VTDecoder::addFreeNode(JPlayer_MediaFrame frame)
{
	comn::AutoCritSec lock(m_freeList_cs);
    m_freeList.push_back(frame);
}

void VTDecoder::addUseNode(JPlayer_MediaFrame frame)
{
	comn::AutoCritSec lock(m_useList_cs);
    m_useList.push_back(frame);
    m_useList.sort(my_node());
}
int VTDecoder::getFreeNode(JPlayer_MediaFrame &frame)
{
	comn::AutoCritSec lock(m_freeList_cs);

    if(m_freeList.empty())
        return -1;
 
    frame = m_freeList.front();
    m_freeList.pop_front();
    return 0;
}
int VTDecoder::getUseNode(JPlayer_MediaFrame &frame,uint64_t pts)
{
   // //VE_LOG_INFO("getUseNode pts = %lld \n",pts);

	comn::AutoCritSec lock(m_useList_cs);

    if(m_useList.empty())
        return -1;

    if(pts == -1 || m_useList.size() >= m_max_frame_num){
        if(m_useList.size() >= m_max_frame_num)
            //VE_LOG_INFO("the first frame timetamp = %lld \n",pts);

        if(m_exportMode && m_useList.size() < m_max_frame_num)return -1;
        frame = m_useList.front();
        m_useList.pop_front();
        return 0;
    }
    
    return -1;
}
void VTDecoder::setCodecType(int vtCodecId)
{
    m_codecType = vtCodecId;
}

int VTDecoder::getSegmentType(int val)
{
    int type = VT_NALU_TYPE_UNK;
    if(m_codecType == VT_CODEC_TYPE_H264){
        switch (val){
            case 7:
                type = VT_NALU_TYPE_SPS;
                break;
            case 8:
                type = VT_NALU_TYPE_PPS;
                break;
            case 5:
                type = VT_NALU_TYPE_IDR;
                break;
            case 1:
                type = VT_NALU_TYPE_NONIDR;
            default:
                break;
        }
    }else if(m_codecType == VT_CODEC_TYPE_H265){
        if(val <= 9)
            type = VT_NALU_TYPE_NONIDR;
        else if(val >= 16 && val <= 23)
            type = VT_NALU_TYPE_IDR;
        else if(val == 33)
            type = VT_NALU_TYPE_SPS;
        else if(val == 34)
            type = VT_NALU_TYPE_PPS;
        else if(val == 32)
            type = VT_NALU_TYPE_VPS;
        //VPS?
    }
    return type;
}

int VTDecoder::getNaluType(uint8_t data)
{
    int type = -1;
    if(m_codecType == VT_CODEC_TYPE_H264){
        type = data & 0x1F;
    }

    if(m_codecType == VT_CODEC_TYPE_H265){
        type = (data & 0x7E) >> 1;
    }
    return type;
}

void VTDecoder::setOutput(int output)
{
    m_output = output;
}

void VTDecoder::setFrameMode(int mode)
{
    m_frameMode = mode;
}

void * vt_decoder_close_and_destroy_internal(void* handle);
extern "C"{
	void* vt_decoder_create(){
		VTDecoder* handle = new VTDecoder();
        return handle;
	}
	void vt_decoder_set_renderless(void* handle){
		VTDecoder* decoder = (VTDecoder*)handle;
		decoder->set_renderless();
	}
    void vt_decoder_set_output(void *handle,int output){
        VTDecoder* decoder = (VTDecoder*)handle;
        decoder->setOutput(output);
    }
    void vt_set_frame_mode(void *handle,int mode){
        VTDecoder* decoder = (VTDecoder*)handle;
        decoder->setFrameMode(mode);
    }
    void vt_set_asyncMode(void *handle){
        VTDecoder* decoder = (VTDecoder*)handle;
        decoder->set_asyncMode();
    }
    void vt_set_ve_exportMode(void *handle){
        VTDecoder* decoder = (VTDecoder*)handle;
        decoder->set_ve_exportMode();
    }
	void vt_decoder_set_pixelbuffer(void* handle){
		VTDecoder* decoder = (VTDecoder*)handle;
		decoder->set_needPixelBuffer();
	}
	void vt_decoder_destroy(void* handle){
		VTDecoder* decoder = (VTDecoder*)handle;
		delete decoder;
	}
	void vt_decoder_close(void* handle){
		VTDecoder* decoder = (VTDecoder*)handle;
		if(decoder){
			decoder->close();
		}
	}
	void vt_decoder_close_and_destroy(void* handle){
		pthread_t tid = 0;
		if(0 != pthread_create(&tid, NULL, vt_decoder_close_and_destroy_internal, (void *)handle)){
	        return;
		}

		if(tid){
			pthread_detach(tid);
		}
	}
	void vt_decoder_set_h264_codec_type(void* handle){
		VTDecoder* decoder = (VTDecoder*)handle;
		if(decoder){
			decoder->setCodecType(VT_CODEC_TYPE_H264);
		}
	}
	void vt_decoder_set_hevc_codec_type(void* handle){
		VTDecoder* decoder = (VTDecoder*)handle;
		if(decoder){
			decoder->setCodecType(VT_CODEC_TYPE_H265);
		}
	}
	void vt_decoder_flush(void* handle){
		VTDecoder* decoder = (VTDecoder*)handle;
		if(decoder){
			decoder->flushBuffers();
		}
	}
	int vt_decoder_decode(void* handle,unsigned char *pInBuffer,
            size_t size,
            uint64_t timestamp,
            uint64_t serialNum,int isKey,uint64_t dts,uint64_t duration,JPlayer_MediaFrame *frame){
		VTDecoder* decoder = (VTDecoder*)handle;
        int ret = -1;
		if(decoder){
			ret = decoder->inputPacket(pInBuffer,size,timestamp,serialNum,isKey,dts,duration,*frame);
		}
        return ret;
	}
}

void* vt_decoder_close_and_destroy_internal(void* handle){
    vt_decoder_close(handle);
    vt_decoder_destroy(handle);
    return 0;
}
