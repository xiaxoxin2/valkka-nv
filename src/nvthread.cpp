/*
 * nvthread.cpp : Cuda accelerated decoder thread for libValkka
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
 *  @file    nvthread.cpp
 *  @author  Xiao Xoxin
 *  @date    2021
 *  @version 1.0.0 
 *  
 *  @brief   Cuda accelerated decoder thread for libValkka
 */ 

#include "nvthread.h"
#include "nvdecoder.h"


bool NVcuInit() {
    CUresult res=cuInit(0);
    return (res==CUDA_SUCCESS);
}


PyObject* NVgetDevices() {
    // PyObject* pydic = PyDict_New();
    PyObject* pylist = PyList_New(0);

    int nGpu, iGpu;
    CUdevice dev = 0;
    char szDeviceName[80];
    
    ck(cuDeviceGetCount(&nGpu));
    //std::cout << "NVgetdevices: " << nGpu << std::endl;

    for(iGpu=0; iGpu<nGpu; iGpu++) {
        ck(cuDeviceGet(&dev, iGpu));
        ck(cuDeviceGetName(szDeviceName, sizeof(szDeviceName), dev));
        /*
        std::cout << "NVgetDevices: " << iGpu
            << " " << szDeviceName << std::endl;
        */
        PyObject* pystr = PyUnicode_FromString(szDeviceName);
        PyList_Append(pylist, pystr);
        Py_DECREF(pystr);
    }
    return pylist;
}



NVThread::NVThread(const char* name, FrameFilter& outfilter, int gpu_index, FrameFifoContext fifo_ctx) 
    : DecoderThread(name, outfilter, fifo_ctx), gpu_index(gpu_index) 
    {
    }

NVThread::~NVThread() {
}

Decoder* NVThread::chooseAudioDecoder(AVCodecID codec_id) {
    DecoderThread::chooseAudioDecoder(codec_id);
}

Decoder* NVThread::chooseVideoDecoder(AVCodecID codec_id) {
    //TODO: try to get NVDecoder, if it's not possible, default
    //to AVDecoder
    switch (codec_id) { // switch: video codecs
        case AV_CODEC_ID_H264:
            return new NVDecoder(AV_CODEC_ID_H264, gpu_index, 5); // gpu_index, n_buffer
            break;
        default:
            return NULL;
            break;        
    }
}

Decoder* NVThread::fallbackAudioDecoder(AVCodecID codec_id) {
    return DecoderThread::chooseAudioDecoder(codec_id);
}


Decoder* NVThread::fallbackVideoDecoder(AVCodecID codec_id) {
    return DecoderThread::chooseVideoDecoder(codec_id);
}
