#ifndef MEDIACODEC_DEC_ENTRY_H_
#define MEDIACODEC_DEC_ENTRY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>

bool enableHwDecodeGPMediaCodecDecoderEx();
void startCallBackGPMediaCodecDecoderEx(void *handle, int param0);
void *createGPMediaCodecDecoderEx(void* handle, const char* path);
bool initGPMediaCodecDecoderEx(void *handle, int render, int width, int height);
void updateTexImageFromEGLGPMediaCodecDecoderEx(void *handle, int render);
void releaseOutputBufferGPMediaCodecDecoderEx(void *handle);
void initSurfaceGPMediaCodecDecoderEx(void *handle, int textureid);
void releaseOutputBufferAllGPMediaCodecDecoderEx();
void stopGPMediaCodecDecoderEx(void *handle);
void releaseGPMediaCodecDecoderEx(void *handle);

void updateTexImageGPMediaCodecDecoderEx(void *handle);
void requestExitGPMediaCodecDecoderEx(void *handle);
void requestStopGPMediaCodecDecoderEx(void *handle);
void receiveStopMsgGPMediaCodecDecoderEx(void *handle);
void playerFrameDropGPMediaCodecDecoderEx();
/**
 * return 0:成功 1队列满
 */
int cacheInputDataGPMediaCodecDecoderEx(void *handle, uint8_t* data, int dataLen, int64_t time);
void getInputDataGPMediaCodecDecoderEx(void * handle, uint8_t* buffer, int64_t buffer_capacity, int* wrote_size, int64_t* pts);
#ifdef __cplusplus
}
#endif

#endif
