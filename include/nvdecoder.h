#ifndef nvdecoder_HEADER_GUARD
#define nvdecoder_HEADER_GUARD
/*
 * nvdecoder.h : Cuda accelerated decoder class for libValkka
 * 
 * Authors: Xiao Xoxin <xiaoxoxin@gmail.com>
 * 
 * This file is part of the Valkka Nvidia cuda bridge.
 *
 * (c) Copyright 2021 Xiao Xoxin
 * 
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 
 * 
 *  0. You just DO WHAT THE FUCK YOU WANT TO. 
 * 
 */

/** 
 *  @file    nvdecoder.h
 *  @author  Xiao Xoxin
 *  @date    2021
 *  @version 1.0.0 
 *  
 *  @brief   Cuda accelerated decoder class for libValkka
 */ 


/*
* Copyright 2017-2018 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

// #pragma once

#include "valkkanv_common.h"
#include "semaring.h"
#include <cuda.h>
#include "NvDecoder.h"
#include "NvCodecUtils.h"
#include "FFmpegDemuxer.h"

// https://docs.nvidia.com/cuda/cuda-driver-api/index.html

bool CUDA_CALL(CUresult res);


int CUDAAPI NVDecoder__sequenceCallback(void *obj, CUVIDEOFORMAT* pVideoFormat);
int CUDAAPI NVDecoder__decodePicture(void* obj, CUVIDPICPARAMS* pPicParams);
int CUDAAPI NVDecoder__displayPicture(void* obj, CUVIDPARSERDISPINFO* pDispInfo);


class NVDecoder : public Decoder {

public:
    NVDecoder(AVCodecID av_codec_id, int gpu_index=0, int n_buf=5);
    virtual ~NVDecoder();

public:
    AVCodecID av_codec_id;  ///< FFmpeg AVCodecId, identifying the codec

protected:
    bool        active;
    int         nGpu, iGpu;
    CUdevice    cuDevice;
    char        szDeviceName[80];
    // NvDecoder   *nv_dec;
    // CUVIDPARSERPARAMS videoParserParameters;
    
public: // so that static callback methods can access them..
    CUvideoparser   parser; // alias to void*
    CUvideodecoder  decoder; // alias to void*
    CUcontext       cuContext;
    SemaRingBuffer  semaring;
    std::mutex      mutex;
    std::vector<AVBitmapFrame*>  
                    out_frame_rb;
    uint8_t*        aux_plane;
    unsigned int    current_pitch, current_height;
    unsigned long   first_timestamp;

protected:
    CUcontext m_cuContext = NULL;
    CUvideoctxlock m_ctxLock;
    std::mutex *m_pMutex;
    CUvideoparser m_hParser = NULL;
    CUvideodecoder m_hDecoder = NULL;
    bool m_bUseDeviceFrame;
    // dimension of the output
    unsigned int m_nWidth = 0, m_nHeight = 0;
    // height of the mapped surface 
    int m_nSurfaceHeight = 0;
    int m_nSurfaceWidth = 0;
    cudaVideoCodec m_eCodec = cudaVideoCodec_NumCodecs;
    cudaVideoChromaFormat m_eChromaFormat;
    int m_nBitDepthMinus8 = 0;
    CUVIDEOFORMAT m_videoFormat = {};
    Rect m_displayRect = {};
    // stock of frames
    std::vector<uint8_t *> m_vpFrame; 
    // decoded frames for return
    std::vector<uint8_t *> m_vpFrameRet;
    // timestamps of decoded frames
    std::vector<int64_t> m_vTimestamp;
    int m_nDecodedFrame = 0, m_nDecodedFrameReturned = 0;
    int m_nDecodePicCnt = 0, m_nPicNumInDecodeOrder[32];
    bool m_bEndDecodeDone = false;
    std::mutex m_mtxVPFrame;
    int m_nFrameAlloc = 0;
    CUstream m_cuvidStream = 0;
    bool m_bDeviceFramePitched = false;
    size_t m_nDeviceFramePitch = 0;
    Rect m_cropRect = {};
    Dim m_resizeDim = {};

    std::ostringstream m_videoInfo;
    unsigned int m_nMaxWidth = 0, m_nMaxHeight = 0;
    bool m_bReconfigExternal = false;
    bool m_bReconfigExtPPChange = false;

    // aux variable
    void* dstFrame = NULL;

public:
    int sequenceCallback(CUVIDEOFORMAT* pVideoFormat);
    int decodePicture(CUVIDPICPARAMS* pPicParams);
    int displayPicture(CUVIDPARSERDISPINFO* pDispInfo);

protected:
    int ReconfigureDecoder(CUVIDEOFORMAT *pVideoFormat);

private:
    // parameters that have to be passed somehow
    // through the async API
    SlotNumber  n_slot_aux;
    int         subsession_index_aux;


public:
    virtual Frame *output();
    virtual void flush();
    virtual bool pull();
    virtual void releaseOutput();
    virtual bool isOk();
    void deactivate(const char* err);
    bool CudaCall(CUresult res);
};

#endif
