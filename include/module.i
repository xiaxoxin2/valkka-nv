%module valkka_nv
%include <std_string.i>

// this has the effect of initializing numpy with call to "import_array()"
%pythoncode %{  from valkka import core %}

%{ // this is prepended in the wapper-generated c(pp) file
#define SWIG_FILE_WITH_INIT

#include "framefilter.h"
#include "thread.h"
#include "framefifo.h"
#include "decoderthread.h"
#include "nvthread.h"

// https://docs.scipy.org/doc/numpy/reference/c-api.array.html#importing-the-api
// https://github.com/numpy/numpy/issues/9309#issuecomment-311320497
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/ndarraytypes.h"
#include "numpy/arrayobject.h"

%}

%init %{
    // WARNING: numpy initialization is done from libValkka
%}

// Swig should not try to create a default constructor for the following classes as they're abstract (swig interface file should not have the constructors either):
%nodefaultctor FrameFilter;
%nodefaultctor Thread;

%typemap(in) (std::size_t) {
  $1=PyLong_AsSize_t($input);
}

%inline %{  
%}

// next, expose what is necessary
// autogenerate from this point on
 
class FrameFilter { // <pyapi>
public: // <pyapi>
    virtual ~FrameFilter();                                  ///< Virtual destructor // <pyapi>
protected: // <pyapi>
}; // <pyapi>
 
class DummyFrameFilter : public FrameFilter { // <pyapi>
public:                                                                                // <pyapi>
    DummyFrameFilter(const char *name, bool verbose = true, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class InfoFrameFilter : public FrameFilter { // <pyapi>
public:                                                          // <pyapi>
    InfoFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class BriefInfoFrameFilter : public FrameFilter { // <pyapi>
public:                                                               // <pyapi>
    BriefInfoFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class ThreadSafeFrameFilter : public FrameFilter { // <pyapi>
public:                                                                // <pyapi>
    ThreadSafeFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class ForkFrameFilter : public FrameFilter { // <pyapi>
public: // <pyapi>
    ForkFrameFilter(const char *name, FrameFilter *next = NULL, FrameFilter *next2 = NULL); // <pyapi>
}; // <pyapi>
 
class ForkFrameFilter3 : public FrameFilter { // <pyapi>
public: // <pyapi>
    ForkFrameFilter3(const char *name, FrameFilter *next = NULL, FrameFilter *next2 = NULL, FrameFilter *next3 = NULL); // <pyapi>
}; // <pyapi>
 
class ForkFrameFilterN : public FrameFilter { // <pyapi>
public: // <pyapi>
    ForkFrameFilterN(const char *name); // <pyapi>
    virtual ~ForkFrameFilterN(); // <pyapi>
public: // <pyapi>
    bool connect(const char *tag, FrameFilter *filter); // <pyapi>
    bool disconnect(const char *tag); // <pyapi>
};                                    // <pyapi>
 
class SlotFrameFilter : public FrameFilter { // <pyapi>
public:                                                                             // <pyapi>
    SlotFrameFilter(const char *name, SlotNumber n_slot, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class PassSlotFrameFilter : public FrameFilter { // <pyapi>
public:                                                                                 // <pyapi>
    PassSlotFrameFilter(const char *name, SlotNumber n_slot, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class DumpFrameFilter : public FrameFilter { // <pyapi>
public:                                                          // <pyapi>
    DumpFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class CountFrameFilter : public FrameFilter { // <pyapi>
public:                                                           // <pyapi>
    CountFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class TimestampFrameFilter : public FrameFilter { // <pyapi>
public:                                                                                                                 // <pyapi>
    TimestampFrameFilter(const char *name, FrameFilter *next = NULL, long int msdiff_max = TIMESTAMP_CORRECT_TRESHOLD); // <pyapi>
}; // <pyapi>
 
class TimestampFrameFilter2 : public FrameFilter { // <pyapi>
public:                                                                                                                  // <pyapi>
    TimestampFrameFilter2(const char *name, FrameFilter *next = NULL, long int msdiff_max = TIMESTAMP_CORRECT_TRESHOLD); // <pyapi>
}; // <pyapi>
 
class DummyTimestampFrameFilter : public FrameFilter { // <pyapi>
public:                                                                    // <pyapi>
    DummyTimestampFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class RepeatH264ParsFrameFilter : public FrameFilter { // <pyapi>
public:                                                                    // <pyapi>
    RepeatH264ParsFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class GateFrameFilter : public FrameFilter { // <pyapi>
public:                                                          // <pyapi>
    GateFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
public:                      // <pyapi>
    void set();              // <pyapi>
    void unSet();            // <pyapi>
    void passConfigFrames(); // <pyapi>
    void noConfigFrames();   // <pyapi>
};                           // <pyapi>
 
class SwitchFrameFilter : public FrameFilter { // <pyapi>
public:                                                                                        // <pyapi>
    SwitchFrameFilter(const char *name, FrameFilter *next1 = NULL, FrameFilter *next2 = NULL); // <pyapi>
    void set1(); // <pyapi>
    void set2(); // <pyapi>
};               // <pyapi>
 
class CachingGateFrameFilter : public FrameFilter { // <pyapi>
public:                                                                 // <pyapi>
    CachingGateFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
public:           // <pyapi>
    void set();   // <pyapi>
    void unSet(); // <pyapi>
};                // <pyapi>
 
class SetSlotFrameFilter : public FrameFilter { // <pyapi>
public:                                                             // <pyapi>
    SetSlotFrameFilter(const char *name, FrameFilter *next = NULL); // <pyapi>
public:                             // <pyapi>
    void setSlot(SlotNumber n = 0); // <pyapi>
}; // <pyapi>
 
class TimeIntervalFrameFilter : public FrameFilter { // <pyapi>
public:                                                                                        // <pyapi>
    TimeIntervalFrameFilter(const char *name, long int mstimedelta, FrameFilter *next = NULL); // <pyapi>
}; // <pyapi>
 
class FifoFrameFilter : public FrameFilter { // <pyapi>
public: // <pyapi>
    FifoFrameFilter(const char *name, FrameFifo *framefifo); ///< Default constructor     // <pyapi>
}; // <pyapi>
 
class BlockingFifoFrameFilter : public FrameFilter { // <pyapi>
public:                                                              // <pyapi>
    BlockingFifoFrameFilter(const char *name, FrameFifo *framefifo); ///< Default constructor // <pyapi>
}; // <pyapi>
 
class SwScaleFrameFilter : public FrameFilter { // <pyapi>
public:                                                                                                  // <pyapi>
    SwScaleFrameFilter(const char *name, int target_width, int target_height, FrameFilter *next = NULL); ///< Default constructor // <pyapi>
    ~SwScaleFrameFilter();                                                                               ///< Default destructor                                              // <pyapi>
}; // <pyapi>
 
class Thread { // <pyapi>
public: // <pyapi>
    ~Thread(); // <pyapi>
public: // *** API ***                                                  // <pyapi>
    void setAffinity(int i); // <pyapi>
    void startCall(); // <pyapi>
    virtual void stopCall(); // <pyapi>
    virtual void requestStopCall(); // <pyapi>
    virtual void waitStopCall(); // <pyapi>
    virtual void waitReady(); // <pyapi>
}; // <pyapi>
 
struct FrameFifoContext {                                                                                                                                       // <pyapi>
  FrameFifoContext() : n_basic(50), n_avpkt(0), n_avframe(0), n_yuvpbo(0), n_setup(20), n_signal(20), n_marker(20), flush_when_full(DEFAULT_FRAMEFIFO_FLUSH_WHEN_FULL) {}     // <pyapi>
  FrameFifoContext(int n_basic, int n_avpkt, int n_avframe, int n_yuvpbo, int n_setup, int n_signal, bool flush_when_full) :                                    // <pyapi>
  n_basic(n_basic), n_avpkt(n_avpkt), n_avframe(n_avframe), n_yuvpbo(n_yuvpbo), n_setup(n_setup), n_signal(n_signal), n_marker(n_signal), flush_when_full(flush_when_full) {}       // <pyapi>
  FrameFifoContext(int n_signal) :                                                                                                                              // <pyapi>
  n_basic(0), n_avpkt(0), n_avframe(0), n_yuvpbo(0), n_setup(0), n_signal(n_signal), n_marker(n_signal), flush_when_full(DEFAULT_FRAMEFIFO_FLUSH_WHEN_FULL) {}  // <pyapi>
  int n_basic;     ///< data at payload                                                                                                                         // <pyapi>
  int n_avpkt;     ///< data at ffmpeg avpkt                                                                                                                    // <pyapi>
  int n_avframe;   ///< data at ffmpeg av_frame and ffmpeg av_codec_context                                                                                     // <pyapi>
  int n_yuvpbo;    ///< data at yuvpbo struct                                                                                                                   // <pyapi>
  int n_setup;     ///< setup data                                                                                                                              // <pyapi>
  int n_signal;    ///< signal to AVThread or OpenGLThread                                                                                                      // <pyapi>
  int n_marker;    ///< marks start/end of frame emission.  defaults to n_signal                                                                                // <pyapi>    
  bool flush_when_full; ///< Flush when filled                                                                                                                  // <pyapi>
};                                                                                                                                                              // <pyapi>
 
class DecoderThread : public Thread { // <pyapi>
public: // <pyapi>
    DecoderThread(const char* name, FrameFilter& outfilter, FrameFifoContext fifo_ctx=FrameFifoContext()); // <pyapi>
    virtual ~DecoderThread(); ///< Default destructor.  Calls AVThread::stopCall                                   // <pyapi>
public: // API <pyapi>
    void setTimeCorrection(bool val);             // <pyapi>
    FifoFrameFilter &getFrameFilter();            // <pyapi>
    FifoFrameFilter &getBlockingFrameFilter();    // <pyapi>
    void setTimeTolerance(long int mstol);    ///< API method: decoder will scrap late frames that are mstol milliseconds late.  Call before starting the thread. // <pyapi>
    void setNumberOfThreads(int n_threads);       // <pyapi>
    void decodingOnCall();   ///< API method: enable decoding        // <pyapi>
    void decodingOffCall();  ///< API method: pause decoding         // <pyapi>
    void requestStopCall();  ///< API method: Like Thread::stopCall() but does not block. // <pyapi>
}; // <pyapi>
bool NVcuInit(); // <pyapi>
PyObject* NVgetDevices(); // <pyapi>
 
class NVThread : public DecoderThread { // <pyapi>
public: // <pyapi>
    NVThread(const char* name, FrameFilter& outfilter, int gpu_index = 0, FrameFifoContext fifo_ctx=FrameFifoContext());   // <pyapi>
    virtual ~NVThread(); ///< Default destructor.  Calls AVThread::stopCall                             // <pyapi>
}; // <pyapi>
