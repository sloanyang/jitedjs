#ifndef JIT_FUNCTION_H
#define JIT_FUNCTION_H

#include "../runtime/JSFunction.h"

struct JITFuncMeta {
	FuncMeta* _slowMeta
	int* codes;
	//Reserved a fixed length for code?
    JITFuncMeta(){ codes = (int*) malloc(sizeof(int) * 4096); }
}

#endif
