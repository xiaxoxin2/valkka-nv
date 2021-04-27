/*
 * dectest.cpp :
 * 
* Copyright 2018 Valkka Security Ltd. and Sampsa Riikonen.
 * 
 * Authors: Sampsa Riikonen <sampsa.riikonen@iki.fi>
 * 
 * This file is part of Valkka cpp examples
 * 
 * Valkka cpp examples is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/** 
 *  @file    dectest.cpp
 *  @author  Sampsa Riikonen
 *  @date    2018
 *  @version 1.0.0 
 *  
 *  @brief 
 *
 */

#include "framefifo.h"
#include "framefilter.h"
#include "livethread.h"
#include "logging.h"
#include "avdep.h"

#include "openglthread.h"
#include "nvthread.h"
#include "nvdecoder.h"
#include "test_import.h"

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

const char *stream_1 = std::getenv("VALKKA_TEST_RTSP_1");
const char *stream_2 = std::getenv("VALKKA_TEST_RTSP_2");
const char *stream_sdp = std::getenv("VALKKA_TEST_SDP");


void test_1()
{
    const char *name = "@TEST: dectest: test 1: ";
    std::cout << name << "** @@DESCRIPTION **" << std::endl;

    // (LiveThread:livethread) --> {FrameFilter:info} --> {FifoFrameFilter:in_filter} -->> (nvthread:nvthread) --> {InfoFrameFilter:decoded_info}
    InfoFrameFilter decoded_info("decoded");
    NVThread nvthread("nvthread", decoded_info);
    FifoFrameFilter &in_filter = nvthread.getFrameFilter(); // request framefilter from nvthread
    InfoFrameFilter out_filter("encoded", &in_filter);
    LiveThread livethread("live");

    if (!stream_1)
    {
        std::cout << name << "ERROR: missing test stream 1: set environment variable VALKKA_TEST_RTSP_1" << std::endl;
        exit(2);
    }
    std::cout << name << "** test rtsp stream 1: " << stream_1 << std::endl;

    std::cout << name << "starting threads" << std::endl;
    livethread.startCall();
    nvthread.startCall();
    sleep_for(2s);
    std::cout << name << "threads started" << std::endl;

    nvthread.decodingOnCall();

    std::cout << name << "registering stream" << std::endl;
    LiveConnectionContext ctx = LiveConnectionContext(
        LiveConnectionType::rtsp,
        std::string(stream_1),
        2,
        &out_filter); // Request livethread to write into filter info
    livethread.registerStreamCall(ctx);

    // sleep_for(1s);

    std::cout << name << "playing stream !" << std::endl;
    livethread.playStreamCall(ctx);

    sleep_for(5s);
    // sleep_for(604800s); //one week

    std::cout << name << "stopping threads" << std::endl;
    livethread.stopCall();
    // nvthread.  stopCall();
    // nvthread destructor => Thread destructor => stopCall (Thread::stopCall or nvthread::stopCall ..?) .. in destructor, virtual methods are not called
    //
}

void test_2()
{
    const char *name = "@TEST: dectest: test 2: ";
    std::cout << name << "** @@DESCRIPTION **" << std::endl;

    bool ok = NVcuInit();

    PyObject* p = NVgetDevices();

    NVDecoder nvdecoder(AV_CODEC_ID_H264, 0, 10);
    NVDecoder nvdecoder2(AV_CODEC_ID_H264, 0, 10);
}

void test_3()
{
    const char *name = "@TEST: dectest: test 3: ";
    std::cout << name << "** @@DESCRIPTION **" << std::endl;

    bool ok = NVcuInit();

    OpenGLThread glthread("gl_thread", OpenGLFrameFifoContext(), 
    DEFAULT_OPENGLTHREAD_BUFFERING_TIME, ":0.0");
    FifoFrameFilter &gl_in_filter = glthread.getFrameFilter();
    // (LiveThread:livethread) --> {FrameFilter:info} --> {FifoFrameFilter:in_filter} -->> (nvthread:nvthread) --> {InfoFrameFilter:decoded_info}
    NVThread nvthread("nvthread", gl_in_filter);
    FifoFrameFilter &in_filter = nvthread.getFrameFilter(); // request framefilter from nvthread
    InfoFrameFilter out_filter("encoded", &in_filter);
    LiveThread livethread("live");

    if (!stream_1)
    {
        std::cout << name << "ERROR: missing test stream 1: set environment variable VALKKA_TEST_RTSP_1" << std::endl;
        exit(2);
    }
    std::cout << name << "** test rtsp stream 1: " << stream_1 << std::endl;

    std::cout << name << "starting threads" << std::endl;
    glthread.startCall();
    Window window_id1 = glthread.createWindow();
    glthread.makeCurrent(window_id1);
    std::cout << "new x window 1" << window_id1 << std::endl;
    livethread.startCall();
    nvthread.startCall();
    // sleep_for(2s);
    std::cout << name << "threads started" << std::endl;

    // render context
    // create render group & context
    glthread.newRenderGroupCall(window_id1);
    int i1 = glthread.newRenderContextCall(2, window_id1, 0);
    std::cout << "got render context id 1 " << i1 << std::endl;

    nvthread.decodingOnCall();

    std::cout << name << "registering stream" << std::endl;
    LiveConnectionContext ctx = LiveConnectionContext(
        LiveConnectionType::rtsp,
        std::string(stream_1),
        2,
        &out_filter); // Request livethread to write into filter info
    livethread.registerStreamCall(ctx);

    // sleep_for(1s);

    std::cout << name << "playing stream !" << std::endl;
    livethread.playStreamCall(ctx);

    // sleep_for(3s);
    sleep_for(2*15s);
    // sleep_for(604800s); //one week

    std::cout << name << "stopping threads" << std::endl;
    livethread.stopCall();
    nvthread.stopCall();
    glthread.stopCall();
    // nvthread.  stopCall();
    // nvthread destructor => Thread destructor => stopCall (Thread::stopCall or nvthread::stopCall ..?) .. in destructor, virtual methods are not called
    //
}

void test_4()
{
    const char *name = "@TEST: dectest: test 4: ";
    std::cout << name << "** @@DESCRIPTION **" << std::endl;

    bool ok = NVcuInit();

    OpenGLThread glthread("gl_thread", OpenGLFrameFifoContext(), 
    DEFAULT_OPENGLTHREAD_BUFFERING_TIME, ":0.0");
    FifoFrameFilter &gl_in_filter = glthread.getFrameFilter();
    // (LiveThread:livethread) --> {FrameFilter:info} --> {FifoFrameFilter:in_filter} -->> (nvthread:nvthread) --> {InfoFrameFilter:decoded_info}
    
    NVThread nvthread1("nvthread1", gl_in_filter);
    NVThread nvthread2("nvthread2", gl_in_filter);

    FifoFrameFilter &in_filter1 = nvthread1.getFrameFilter(); // request framefilter from nvthread
    FifoFrameFilter &in_filter2 = nvthread2.getFrameFilter(); // request framefilter from nvthread

    LiveThread livethread("live");

    if (!stream_1)
    {
        std::cout << name << "ERROR: missing test stream 1: set environment variable VALKKA_TEST_RTSP_1" << std::endl;
        exit(2);
    }
    std::cout << name << "** test rtsp stream 1: " << stream_1 << std::endl;
    if (!stream_2)
    {
        std::cout << name << "ERROR: missing test stream 2: set environment variable VALKKA_TEST_RTSP_2" << std::endl;
        exit(2);
    }
    std::cout << name << "** test rtsp stream 2: " << stream_2 << std::endl;
    
    std::cout << name << "starting threads" << std::endl;
    glthread.startCall();

    Window window_id1 = glthread.createWindow();
    glthread.makeCurrent(window_id1);
    std::cout << "new x window 1" << window_id1 << std::endl;
    
    Window window_id2 = glthread.createWindow();
    glthread.makeCurrent(window_id2);
    std::cout << "new x window 2" << window_id2 << std::endl;

    livethread.startCall();
    
    nvthread1.startCall();
    nvthread2.startCall();

    // sleep_for(2s);
    std::cout << name << "threads started" << std::endl;

    // render context
    // create render group & context
    glthread.newRenderGroupCall(window_id1);
    int i1 = glthread.newRenderContextCall(2, window_id1, 0);
    std::cout << "got render context id 1 " << i1 << std::endl;

    glthread.newRenderGroupCall(window_id2);
    int i2 = glthread.newRenderContextCall(3, window_id2, 0);
    std::cout << "got render context id 2 " << i2 << std::endl;

    nvthread1.decodingOnCall();
    nvthread2.decodingOnCall();

    std::cout << name << "registering streams" << std::endl;
    LiveConnectionContext ctx1 = LiveConnectionContext(
        LiveConnectionType::rtsp,
        std::string(stream_1),
        2,
        &in_filter1); // Request livethread to write into filter info
    livethread.registerStreamCall(ctx1);

    LiveConnectionContext ctx2 = LiveConnectionContext(
        LiveConnectionType::rtsp,
        std::string(stream_2),
        3,
        &in_filter2); // Request livethread to write into filter info
    livethread.registerStreamCall(ctx2);

    // sleep_for(1s);

    std::cout << name << "playing stream !" << std::endl;
    livethread.playStreamCall(ctx1);
    livethread.playStreamCall(ctx2);

    // sleep_for(3s);
    sleep_for(2*15s);
    // sleep_for(604800s); //one week

    std::cout << name << "stopping threads" << std::endl;
    livethread.stopCall();
    
    nvthread1.stopCall();
    nvthread2.stopCall();
    
    glthread.stopCall();
    // nvthread.  stopCall();
    // nvthread destructor => Thread destructor => stopCall (Thread::stopCall or nvthread::stopCall ..?) .. in destructor, virtual methods are not called
    //
}

void test_5()
{

    const char *name = "@TEST: dectest: test 5: ";
    std::cout << name << "** @@DESCRIPTION **" << std::endl;
}

int main(int argc, char **argcv)
{
    if (argc < 2)
    {
        std::cout << argcv[0] << " needs an integer argument.  Second interger argument (optional) is verbosity" << std::endl;
    }
    else
    {

        if (argc > 2)
        { // choose verbosity
            switch (atoi(argcv[2]))
            {
            case (0): // shut up
                ffmpeg_av_log_set_level(0);
                fatal_log_all();
                break;
            case (1): // normal
                break;
            case (2): // more verbose
                ffmpeg_av_log_set_level(100);
                debug_log_all();
                break;
            case (3): // extremely verbose
                ffmpeg_av_log_set_level(100);
                crazy_log_all();
                break;
            default:
                std::cout << "Unknown verbosity level " << atoi(argcv[2]) << std::endl;
                exit(1);
                break;
            }
        }

        switch (atoi(argcv[1]))
        { // choose test
        case (1):
            test_1();
            break;
        case (2):
            test_2();
            break;
        case (3):
            test_3();
            break;
        case (4):
            test_4();
            break;
        case (5):
            test_5();
            break;
        default:
            std::cout << "No such test " << argcv[1] << " for " << argcv[0] << std::endl;
        }
    }
}
