#ifndef JITSTUBS_H
#define JISSTUBS_H

#include "../runtime/JSValue.h"

struct JITStackFrame {
    JITValue args[4];
    void* padding[2]; // Maintain 16-byte stack alignment.

    void* savedEBX;
    void* savedEDI;
    void* savedESI;
    void* savedEBP;
    void* savedEIP;

    void* code;
    Stack* stack;
};

#endif
