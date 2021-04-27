#ifndef nvthread_HEADER_GUARD
#define nvthread_HEADER_GUARD
/*
 * nvthread.h : Cuda accelerated decoder thread for libValkka
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
 *  @file    nvthread.h
 *  @author  Xiao Xoxin
 *  @date    2021
 *  @version 1.0.0 
 *  
 *  @brief   Cuda accelerated decoder thread for libValkka
 */ 

#include "valkkanv_common.h"

bool NVcuInit(); // <pyapi>

PyObject* NVgetDevices(); // <pyapi>

class NVThread : public DecoderThread { // <pyapi>
  
public: // <pyapi>
    /** Default constructor
    * 
    * @param name              Name of the thread
    * @param outfilter         Outgoing frames are written here.  Outgoing frames may be of type FrameType::avframe
    * @param fifo_ctx          Parametrization of the internal FrameFifo
    * 
    */
    NVThread(const char* name, FrameFilter& outfilter, int gpu_index = 0, FrameFifoContext fifo_ctx=FrameFifoContext());   // <pyapi>
    virtual ~NVThread(); ///< Default destructor.  Calls AVThread::stopCall                             // <pyapi>

private:
    int gpu_index;

protected:
    virtual Decoder* chooseAudioDecoder(AVCodecID codec_id);
    virtual Decoder* chooseVideoDecoder(AVCodecID codec_id);
    virtual Decoder* fallbackAudioDecoder(AVCodecID codec_id);
    virtual Decoder* fallbackVideoDecoder(AVCodecID codec_id);

}; // <pyapi>

#endif
