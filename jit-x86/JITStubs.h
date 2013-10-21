#ifndef JITSTUBS_H
#define JISSTUBS_H

#include "../runtime/JSValue.h"
#include "../runtime/JSVM.h"

struct JITStackFrame {
    JSValue args[4];			//sizeof(JSValue) == 16
	void* padding[1];		

    void* code;					//0x48
    StackFrame* stack;			
	void* temp0;				//0x58
	void* temp1;
	void* temp2;
	void* temp3;

    void* savedRBX;
    void* savedR15;
    void* savedR14;
    void* savedR13;
    void* savedR12;
    void* savedRBP;
    void* savedRIP;
};


extern "C" JSValue ctiTrampolinejjs(void* code, StackFrame*, void*, void*, void*, void*);

#endif
