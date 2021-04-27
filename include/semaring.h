#ifndef semaring_HEADER_GUARD
#define semaring_HEADER_GUARD
/*
 * semaring.h : Ringbuffer book-keeping with a semaphore counter
 */

#include <iostream>

class SemaRingBuffer {

public:
    SemaRingBuffer(int n_max);
    ~SemaRingBuffer();
    
private:
    int     n_max;
    int     sema_count;
    int     prev_read;
    int     prev_write;

public:
    void reset();
    int write();
    int read();
    int getIndex();
    bool isEmpty();
};


#endif
