#ifndef JITSTUBS_H
#define JISSTUBS_H

#include "../runtime/JSValue.h"
#include "../runtime/JSVM.h"

struct JITStackFrame {
    JSValue args[4];
    void* padding[2]; // Maintain 16-byte stack alignment.

    void* savedEBX;
    void* savedEDI;
    void* savedESI;
    void* savedEBP;
    void* savedEIP;

    void* code;
    StackFrame* stack;
};


extern "C" JSValue ctiTrampoline(void* code, StackFrame*);

#endif
