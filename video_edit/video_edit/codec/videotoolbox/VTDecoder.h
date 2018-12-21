//
//  VTDecoder.hpp
//  JPlayer
//
//  Created by daxi on 16/2/18.
//  Copyright © 2016年 360. All rights reserved.
//

#ifndef VTDecoder_hpp
#define VTDecoder_hpp

#include <stdio.h>
#include "MediaInfo.h"
#include <VideoToolbox/VideoToolbox.h>
//#include "TResQueue.h"
#include <list>
#include "TCriticalSection.h"
#define MAX_PPS_NUMS  5

enum
{
    VT_CODEC_TYPE_UNKNOWN = -1,
    VT_CODEC_TYPE_H264 ,
    VT_CODEC_TYPE_H265
};

enum
{
    VT_NALU_TYPE_PPS = 5,
    VT_NALU_TYPE_SPS,
    VT_NALU_TYPE_VPS,
    VT_NALU_TYPE_IDR,
    VT_NALU_TYPE_NONIDR,
    VT_NALU_TYPE_UNK
};

class VTDecoder
{
public:
    VTDecoder();
    ~VTDecoder();
    
    int inputPacket(unsigned char *pInBuffer,
                                   size_t size,
                                   uint64_t timestamp,
                                   uint64_t serialNum,int isKey,uint64_t dts,uint64_t duration,JPlayer_MediaFrame &frame);
    void close();
    void flushBuffers();
    void set_renderless();

    void setCodecType(int vtCodecId);

    void set_needPixelBuffer();
    void set_asyncMode();
    void set_ve_exportMode();
    void setOutput(int output);
    void setFrameMode(int mode);

public:
    static void didDecompress( void *decompressionOutputRefCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef pixelBuffer, CMTime presentationTimeStamp, CMTime presentationDuration );
private:
    int initVTDecoder();
    CVPixelBufferRef decode(uint8_t *pBuffer,size_t size,uint64_t dts,uint64_t duration,uint64_t pts);
    size_t splitPacket(unsigned char *p,size_t size);
    int getFreeNode(JPlayer_MediaFrame &frame);
    int getUseNode(JPlayer_MediaFrame &frame,uint64_t pts);
    void addFreeNode(JPlayer_MediaFrame frame);
    void addUseNode(JPlayer_MediaFrame frame);
    int addPixelBuffer(CVImageBufferRef pixelBuffer,uint64_t timestamp,uint64_t duration);
    void clearAllBuffers();
    void reset();
    void SetVideoFormat(CMVideoFormatDescriptionRef video_format);
    void reconfigure();
    int getSegmentType(int val);
    int getNaluType(uint8_t data);
     bool m_bNeedPixelBuffer{false};
private:
    uint8_t *m_pps[MAX_PPS_NUMS];
    size_t  m_ppsSize[MAX_PPS_NUMS];
    int     m_ppsNums;
    uint8_t m_ready;
    uint8_t *m_sps;
    size_t m_spsSize;
    uint8_t *m_vps;
    size_t m_vpsSize;
    VTDecompressionSessionRef m_deocderSession;
    CMVideoFormatDescriptionRef m_decoderFormatDescription;
    uint8_t *m_pTmpBuffer;
    uint8_t *m_pTmpRawBuffer;
    std::list<JPlayer_MediaFrame> m_freeList;
    std::list<JPlayer_MediaFrame> m_useList;

    comn::CriticalSection m_freeList_cs;
    comn::CriticalSection m_useList_cs;

    int64_t m_timestampEx;
    uint64_t m_decoderNum;
    bool m_bHasBFrame;
    comn::CriticalSection m_cs;
    bool m_bIsNeedRender;

    int m_codecType;
    int m_output;
    int m_frameMode;
    int m_lastOutput;

    int m_max_frame_num{0};

    int m_async;
    int m_exportMode{0};
};

#endif /* VTDecoder_hpp */
