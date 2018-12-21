/*
 * h264 video Decoder over videotoolbox from yanjia
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/atomic.h"
#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "libavutil/display.h"
#include "libavutil/internal.h"
#include "libavutil/md5.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/stereo3d.h"

//#include "bswapdsp.h"
#include "libavcodec/bytestream.h"
#include "libavcodec/cabac_functions.h"
#include "libavcodec/golomb.h"

#include "videotoolbox.h"
#ifndef WIN32
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <dlfcn.h>
#endif

#include "libavcodec/hevc.h"


#include "libavutil/buffer.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/bswapdsp.h"
#include "libavcodec/get_bits.h"
#include "libavcodec/internal.h"
#include "libavcodec/thread.h"
#include "libavcodec/videodsp.h"
#include "MediaInfo.h"

#include "libyuv/convert.h"
#include "VELog.h"
typedef struct{
	int m_mode;
}VEVTMode;

void* vt_decoder_create();
void vt_decoder_destroy(void* handle);
void vt_decoder_set_renderless(void* handle);
void vt_decoder_set_output(void *handle,int output);
void vt_decoder_set_pixelbuffer(void* handle);
void vt_set_asyncMode(void* handle);
void vt_set_ve_exportMode(void* handle);
void vt_set_frame_mode(void *handle,int mode);
void vt_decoder_close(void* handle);
void vt_decoder_close_and_destroy(void* handle);
void vt_decoder_set_h264_codec_type(void* handle);
void vt_decoder_set_hevc_codec_type(void* handle);
void vt_decoder_flush(void* handle);
int vt_decoder_decode(void* handle,unsigned char *pInBuffer,
		size_t size,
		uint64_t timestamp,
		uint64_t serialNum,int isKey,uint64_t dts,uint64_t duration,JPlayer_MediaFrame *frame);
#define _AVOID_FRAME_COPY_ 0

typedef struct  VTContext {
const AVClass *c;  // needed by private avoptions
AVCodecContext *avctx;
void* decoder;
AVFrame * output_frame;
int renderless;
int need_pixelbuffer;
} VTContext;

#define QYVIDEO_ENABLE_LOGO_FLAG 0x1000
int64_t av_gettime(void);
static int vt_h264_decode_frame(AVCodecContext *avctx, void *data, int *got_output,
                             AVPacket *avpkt)
{
	int ret = 0;
	JPlayer_MediaFrame frame;
	AVFrame * ptr_out_frame, * ptr_frame ;
    VTContext *s = avctx->priv_data;
    int64_t start_time = av_gettime();

    *got_output = 0;

    if(s->decoder){
        if(avpkt && avpkt->pos == -360){
            vt_decoder_set_output(s->decoder, 0);
        }else{
            vt_decoder_set_output(s->decoder, 1);
        }
        
        if(avpkt && avpkt->pos == -361 || avpkt->pos == -360){
            vt_set_frame_mode(s->decoder, 1);
        }else{
            vt_set_frame_mode(s->decoder, 0);
        }
        
            
    	ret = vt_decoder_decode(s->decoder,avpkt->data,
    			avpkt->size,
    			avpkt->pts,
    			avpkt->pos,avpkt->flags & AV_PKT_FLAG_KEY,avpkt->dts,avpkt->duration,&frame);

    	
    	//VE_LOG_INFO("vt_decoder_decode_cost1:%lld",(av_gettime() - start_time));
    	if(ret == 0){

    		ptr_out_frame = data;


    		*ptr_out_frame = *s->output_frame;


    		ptr_out_frame->width  = frame.m_width;
    		ptr_out_frame->height = frame.m_height;
    		if(s->renderless && s->need_pixelbuffer){
    			ptr_out_frame->format = AV_PIX_FMT_YUV420P;
    		}
            /*
            else if(s->renderless){
    			ptr_out_frame->format = AV_PIX_FMT_NV12;
    		}else{
    			ptr_out_frame->format = AV_PIX_FMT_YUV420P;
    		}
        	  */
    		av_frame_get_buffer(ptr_out_frame, 32 );

    		if(s->renderless && s->need_pixelbuffer){

    			ptr_out_frame->data[7] = frame.m_pData[0];
                ptr_out_frame->linesize[7] = AV_PIX_FMT_VIDEOTOOLBOX;
    		}
            /*
            else if(s->renderless){

        		memcpy(ptr_out_frame->data[0],frame.m_pData[0],frame.m_size[0] * frame.m_height);
        		memcpy(ptr_out_frame->data[1],frame.m_pData[1],frame.m_size[1] * frame.m_height / 2);
			
        	}else{


				I420Copy((uint8_t*)frame.m_pData[0],frame.m_size[0],(uint8_t*)frame.m_pData[1],frame.m_size[1],(uint8_t*)frame.m_pData[2],frame.m_size[2],
						ptr_out_frame->data[0], ptr_out_frame->linesize[0],ptr_out_frame->data[1], ptr_out_frame->linesize[1],ptr_out_frame->data[2], ptr_out_frame->linesize[2],
						frame.m_width,frame.m_height);
        	}
             */

    	    ptr_out_frame->pts = frame.m_timestamp;
    	    ptr_out_frame->pkt_dts = frame.m_dts;
    	    ptr_out_frame->pkt_pts = frame.m_timestamp;

    	    //VE_LOG_INFO("vt_decoder_decode_cost2:%lld",(av_gettime() - start_time));
    		*got_output = 1;
    		return avpkt->size;
    	}
    }
    return 0;
}

static av_cold int vt_h264_decode_free(AVCodecContext *avctx)
{

    VTContext *s = avctx->priv_data;
    if(s->decoder){
    	vt_decoder_close_and_destroy(s->decoder);

    	av_frame_free(&s->output_frame);
    	s->decoder = NULL;
    }
    return 0;
}





static av_cold int vt_hevc_decode_init(AVCodecContext *avctx) {
    VTContext *s = avctx->priv_data;
    VEVTMode vt_mode;
    s->avctx = avctx;

    s->output_frame = av_frame_alloc();
    /*
    if ( !s->output_frame) {
         return AVERROR(ENOMEM);
    }
     */
    s->decoder = vt_decoder_create();
    vt_decoder_set_hevc_codec_type(s->decoder);


    if(avctx->pix_fmt == AV_PIX_FMT_VIDEOTOOLBOX){
    	vt_decoder_set_renderless(s->decoder);
    	vt_decoder_set_pixelbuffer(s->decoder);
    	s->renderless = 1;
    	s->need_pixelbuffer = 1;
    }/*
    else if(avctx->pix_fmt == AV_PIX_FMT_NV12){
    	vt_decoder_set_renderless(s->decoder);
    	s->renderless = 1;
    	s->need_pixelbuffer = 0;
    }else{
    	s->renderless = 0;
    	s->need_pixelbuffer = 0;
    }
      */
    vt_mode = *((VEVTMode*)s->avctx->opaque);
    if(vt_mode.m_mode == 1){
    	vt_set_asyncMode(s->decoder);
    	vt_set_ve_exportMode(s->decoder);
    }

    return 0;
}

static av_cold int vt_h264_decode_init(AVCodecContext *avctx) {
    VTContext *s = avctx->priv_data;
    VEVTMode vt_mode;
    s->avctx = avctx;
    
    s->output_frame = av_frame_alloc();
    if ( !s->output_frame) {
        return AVERROR(ENOMEM);
    }
    s->decoder = vt_decoder_create();
    vt_decoder_set_h264_codec_type(s->decoder);
    
    
    
    if(avctx->pix_fmt == AV_PIX_FMT_VIDEOTOOLBOX){
        vt_decoder_set_renderless(s->decoder);
        vt_decoder_set_pixelbuffer(s->decoder);
        s->renderless = 1;
        s->need_pixelbuffer = 1;
    }
    /*
    else if(avctx->pix_fmt == AV_PIX_FMT_NV12){
        vt_decoder_set_renderless(s->decoder);
        s->renderless = 1;
        s->need_pixelbuffer = 0;
    }else{
        s->renderless = 0;
        s->need_pixelbuffer = 0;
    }
     */
    vt_mode = *((VEVTMode*)s->avctx->opaque);
    if(vt_mode.m_mode == 1){
    	vt_set_asyncMode(s->decoder);
    	vt_set_ve_exportMode(s->decoder);
    }
    
    return 0;
}
static void vt_h264_decode_flush(AVCodecContext *avctx)
{
    VTContext *s = avctx->priv_data;
    if(s->decoder){
    	vt_decoder_flush(s->decoder);
    }

}

#define OFFSET(x) offsetof(VTContext, x)
#define PAR (AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_VIDEO_PARAM)
#define VD AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_DECODING_PARAM

static const AVProfile profiles[] = {
	    { FF_PROFILE_H264_BASELINE,             "Baseline"              },
	    { FF_PROFILE_H264_CONSTRAINED_BASELINE, "Constrained Baseline"  },
	    { FF_PROFILE_H264_MAIN,                 "Main"                  },
	    { FF_PROFILE_H264_EXTENDED,             "Extended"              },
	    { FF_PROFILE_H264_HIGH,                 "High"                  },
	    { FF_PROFILE_H264_HIGH_10,              "High 10"               },
	    { FF_PROFILE_H264_HIGH_10_INTRA,        "High 10 Intra"         },
	    { FF_PROFILE_H264_HIGH_422,             "High 4:2:2"            },
	    { FF_PROFILE_H264_HIGH_422_INTRA,       "High 4:2:2 Intra"      },
	    { FF_PROFILE_H264_HIGH_444,             "High 4:4:4"            },
	    { FF_PROFILE_H264_HIGH_444_PREDICTIVE,  "High 4:4:4 Predictive" },
	    { FF_PROFILE_H264_HIGH_444_INTRA,       "High 4:4:4 Intra"      },
	    { FF_PROFILE_H264_CAVLC_444,            "CAVLC 4:4:4"           },
	    { FF_PROFILE_UNKNOWN },
};
static const AVProfile hevc_profiles[] = {
    { FF_PROFILE_HEVC_MAIN,                 "Main"                },
    { FF_PROFILE_HEVC_MAIN_10,              "Main 10"             },
    { FF_PROFILE_HEVC_MAIN_STILL_PICTURE,   "Main Still Picture"  },
    { FF_PROFILE_HEVC_REXT,                 "Rext"                },
    { FF_PROFILE_UNKNOWN },
};

static const AVOption options[] = {
    { NULL },
};

static const AVClass h264_videotoolbox_class = {
    .class_name = "h264_videotoolbox decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static const AVClass hevc_videotoolbox_class = {
    .class_name = "hevc_videotoolbox decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec h264_videotoolbox_decoder = {
    .name                  = "h264_videotoolbox_dec",
    .long_name             = NULL_IF_CONFIG_SMALL("h264 videotoolbox decoder"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_H264,
    .priv_data_size        = sizeof(VTContext),
    .priv_class            = &h264_videotoolbox_class,
    .init                  = vt_h264_decode_init,
    .close                 = vt_h264_decode_free,
    .decode                = vt_h264_decode_frame,
    .flush                 = vt_h264_decode_flush,
    .capabilities          = CODEC_CAP_DR1 | CODEC_CAP_DELAY,
    .profiles              = NULL_IF_CONFIG_SMALL(profiles),
};
AVCodec hevc_videotoolbox_decoder = {
    .name                  = "hevc_videotoolbox_dec",
    .long_name             = NULL_IF_CONFIG_SMALL("hevc videotoolbox decoder"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_HEVC,
    .priv_data_size        = sizeof(VTContext),
    .priv_class            = &hevc_videotoolbox_class,
    .init                  = vt_hevc_decode_init,
    .close                 = vt_h264_decode_free,
    .decode                = vt_h264_decode_frame,
    .flush                 = vt_h264_decode_flush,
    .capabilities          = CODEC_CAP_DR1 | CODEC_CAP_DELAY,
    .profiles              = NULL_IF_CONFIG_SMALL(hevc_profiles),
};
