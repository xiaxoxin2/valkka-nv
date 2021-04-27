/*
 * semaring.h : Ringbuffer book-keeping with a semaphore counter
 */

#include "semaring.h"


SemaRingBuffer::SemaRingBuffer(int n_max) :
    n_max(n_max), sema_count(0), prev_read(-1), prev_write(-1) {
    }


SemaRingBuffer::~SemaRingBuffer() {
}


void SemaRingBuffer::reset() {
    sema_count=0;
    prev_read=-1;
    prev_write=-1;
}

int SemaRingBuffer::write() {
    if (sema_count >= n_max) {
        std::cout << "SemaRingBuffer: overflow" << std::endl;
        return -1;
    }
    sema_count++;
    prev_write++;
    if (prev_write >= (n_max-1)) {
        prev_write=0;
    }
    return prev_write;
}


int SemaRingBuffer::read() {
    if (sema_count <= 0) {
        std::cout << "SemaRingBuffer: underflow" << std::endl;
        return -1;
    }
    sema_count--;
    prev_read++;
    if (prev_read >= (n_max-1)) {
        prev_read=0;
    }
    return prev_read;
}

int SemaRingBuffer::getIndex() {
    return prev_write;
}

bool SemaRingBuffer::isEmpty() {
    if (sema_count <= 0) {
        return true;
    }
    return false;
}

