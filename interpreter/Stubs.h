
#ifndef STUBS_H
#define STUBS_H

#include "../runtime/JSVM.h"
#include "../runtime/JSFunction.h"
#include "../runtime/JSArray.h"
#include "../runtime/JSString.h"

#include "../bytecode/common.h"
#include "../jit-x86/AssemblerX86.h"
#include "../jit-x86/JITStubsX86.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

char jitedcode[64];

bool ProtectMemory(void * addr, int flags)
{
    // Constant holding the page size value
    const size_t pageSize = sysconf(_SC_PAGE_SIZE);

    // Calculate relative page offset
    size_t temp = (size_t) addr;
    temp -= temp % pageSize;

    // Update address
    addr = (void*) temp;

    // Update memory area protection
    return !mprotect(addr, pageSize, flags);
}

enum StubType {
    S_NewFunction, S_NewArray,

    S_Move,
    S_Not, S_Minus, S_Len,
    S_SetGlobal, S_GetGlobal,

    S_Add, S_Sub, S_Mul, S_Div, S_Mod, S_Pow,
    S_Less, S_LessEq, S_Greater, S_GreaterEq, S_Equal, S_NEqual, 
    S_SetArray, S_GetArray,

    S_Jump, S_TrueJump, S_FalseJump,

    S_Call,
};

#define DECODE_1(bw1, v1) \
    int v1;\
    v1 = (code >> 8) & ((1 << bw1) - 1);
#define DECODE_2(bw1, bw2, v1, v2) \
    int v1, v2;\
    v1 = (code >> 8) & ((1 << bw1) - 1);\
    v2 = (code >> (8 + bw1)) & ((1 << bw2) - 1);
#define DECODE_3(bw1, bw2, bw3, v1, v2, v3) \
    int v1, v2, v3;\
    v1 = (code >> 8) & ((1 << bw1) - 1);\
    v2 = (code >> (8 + bw1)) & ((1 << bw2) - 1);\
    v3 = (code >> (8 + bw1 + bw2)) & ((1 << bw3) - 1);

template<int>
struct StubHandler;

template<>
struct StubHandler<S_NewFunction> {
    FORCE_INLINE static FuncMetaPtr getMetaFromCode(int code) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_META_IDX, destID, metaIdx);
        (void)destID;
        return JSVM::instance()->getMetaFromIdx(metaIdx);
    }
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_META_IDX, destID, metaIdx);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        *dest = JSValue::fromFunction(JSFunction::create(JSVM::instance()->getMetaFromIdx(metaIdx)));
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_META_IDX, destID, metaIdx);
        return format("newfunction %s<-%d", VarID(destID).toString(meta).c_str(), metaIdx);
    }
};

template<>
struct StubHandler<S_NewArray> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, 8, destID, srcID, len);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto array = new JSArray();
        if (len > 0) {
            auto src = VarID(srcID).toValue(frame->localConstPtr);
            array->array.insert(array->array.end(), src, src + len);
        }
        *dest = JSValue::fromArray(array);
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, 8, destID, srcID, len);
        return format("newarray %s<-%s,%d", VarID(destID).toString(meta).c_str(), VarID(srcID).toString(meta).c_str(), len);
    }
};
template<>
struct StubHandler<S_Move> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto src = VarID(srcID).toValue(frame->localConstPtr);
        *dest = *src;
    }

    FORCE_INLINE static void compile(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);

		x86AssemblyBuilder* builder = new x86AssemblyBuilder(jitedcode);

		builder->beginBuild();
		//printf("srcID %x, destID %x\n", srcID, destID);
        //auto srcIDvalue = VarID(srcID).toValue(frame->localConstPtr);
		//printf("srcID value %x\n", srcIDvalue);

		builder->loadLocal(srcID);
		builder->storeLocal(destID);
		builder->endBuild();
    }

    FORCE_INLINE static void run(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
		void* temp0;
		void* temp1;
		void* temp2;
		void* temp3;
		//printf("%x, %x, %x, %x\n", jitedcode, frame, frame->localConstPtr[1], *(int*)(((void*)frame)+0x18));
		ProtectMemory((void*)jitedcode, PROT_EXEC | PROT_WRITE | PROT_READ);
        ctiTrampolinejjs(jitedcode, frame, temp0, temp1, temp2, temp3);
    }

    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        return format("move %s<-%s", VarID(destID).toString(meta).c_str(), VarID(srcID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Not> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto src = VarID(srcID).toValue(frame->localConstPtr);
        dest->data.b = !src->getBoolean();
        dest->type = JSVT_Boolean;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        return format("not %s<-%s", VarID(destID).toString(meta).c_str(), VarID(srcID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Len> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto src = VarID(srcID).toValue(frame->localConstPtr);
        ASSERT(src->type == JSVT_Array);
        dest->data.num = (int)src->data.array->array.size();
        dest->type = JSVT_Number;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        return format("len %s<-#%s", VarID(destID).toString(meta).c_str(), VarID(srcID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Minus> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto src = VarID(srcID).toValue(frame->localConstPtr);
        ASSERT(src->type == JSVT_Number);
        dest->data.num = -src->data.num;
        dest->type = JSVT_Number;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
        return format("minus %s<-%s", VarID(destID).toString(meta).c_str(), VarID(srcID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_SetGlobal> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, kID, vID);
        auto k = VarID(kID).toValue(frame->localConstPtr);
        auto v = VarID(vID).toValue(frame->localConstPtr);
        JSVM::instance()->setGlobal(*k, *v);
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, kID, vID);
        return format("setglobal _G[%s]=%s", VarID(kID).toString(meta).c_str(), VarID(vID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_GetGlobal> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, kID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto k = VarID(kID).toValue(frame->localConstPtr);
        *dest = JSVM::instance()->getGlobal(*k);
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_VAR_ID, destID, kID);
        return format("getglobal %s=_G[%s]", VarID(destID).toString(meta).c_str(), VarID(kID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Add> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        if (l->type == JSVT_Number) {
            ASSERT(r->type == JSVT_Number);
            dest->data.num = l->data.num + r->data.num;
            dest->type = JSVT_Number;
        } else if (l->type == JSVT_String) {
            *dest = JSValue::fromString((string(l->data.str->buf) + r->toString()).c_str());
        } else {
            ASSERT(0);
        }
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("add %s=%s+%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Sub> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.num = l->data.num - r->data.num;
        dest->type = JSVT_Number;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("sub %s=%s-%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Mul> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.num = l->data.num * r->data.num;
        dest->type = JSVT_Number;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("mul %s=%s*%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Div> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.num = l->data.num / r->data.num;
        dest->type = JSVT_Number;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("div %s=%s/%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Mod> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.num = ::fmod(l->data.num, r->data.num);
        dest->type = JSVT_Number;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("mod %s=%s%%%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Pow> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.num = ::pow(l->data.num, r->data.num);
        dest->type = JSVT_Number;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("pow %s=%s^%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Less> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.b = l->data.num < r->data.num;
        dest->type = JSVT_Boolean;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("less %s=%s<%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_LessEq> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.b = l->data.num <= r->data.num;
        dest->type = JSVT_Boolean;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("lessEq %s=%s<=%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Greater> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.b = l->data.num > r->data.num;
        dest->type = JSVT_Boolean;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("greater %s=%s>%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_GreaterEq> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.b = l->data.num >= r->data.num;
        dest->type = JSVT_Boolean;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("greaterEq %s=%s>=%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Equal> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.b = l->data.num == r->data.num;
        dest->type = JSVT_Boolean;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("equal %s=%s==%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_NEqual> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto l = VarID(lID).toValue(frame->localConstPtr);
        auto r = VarID(rID).toValue(frame->localConstPtr);
        ASSERT(l->type == JSVT_Number && r->type == JSVT_Number);
        dest->data.b = l->data.num != r->data.num;
        dest->type = JSVT_Boolean;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
        return format("nequal %s=%s!=%s", VarID(destID).toString(meta).c_str(), VarID(lID).toString(meta).c_str(), VarID(rID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_SetArray> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, arrayID, kID, vID);
        auto array = VarID(arrayID).toValue(frame->localConstPtr);
        auto k = VarID(kID).toValue(frame->localConstPtr);
        auto v = VarID(vID).toValue(frame->localConstPtr);
        ASSERT(array->type == JSVT_Array && k->type == JSVT_Number);
        int i = (int)k->data.num;
        auto &vec = array->data.array->array;
        ASSERT(i >= 0 && i < (int)vec.size());
        vec[i] = *v;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, arrayID, kID, vID);
        return format("setarray %s[%s]=%s", VarID(arrayID).toString(meta).c_str(), VarID(kID).toString(meta).c_str(), VarID(vID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_GetArray> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, arrayID, kID);
        auto dest = VarID(destID).toValue(frame->localConstPtr);
        auto array = VarID(arrayID).toValue(frame->localConstPtr);
        auto k = VarID(kID).toValue(frame->localConstPtr);
        ASSERT(array->type == JSVT_Array && k->type == JSVT_Number);
        int i = (int)k->data.num;
        auto &vec = array->data.array->array;
        ASSERT(i >= 0 && i < (int)vec.size());
        *dest = vec[i];
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, arrayID, kID);
        return format("getarray %s=%s[%s]", VarID(destID).toString(meta).c_str(), VarID(arrayID).toString(meta).c_str(), VarID(kID).toString(meta).c_str());
    }
};
template<>
struct StubHandler<S_Jump> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_1(BIT_W_IP, ip);
        frame->ip = ip - 1;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_1(BIT_W_IP, ip);
        return format("jump %d", ip + 1);
    }
};
template<>
struct StubHandler<S_TrueJump> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_IP, testID, ip);
        auto test = VarID(testID).toValue(frame->localConstPtr);
        if (test->getBoolean()) frame->ip = ip - 1;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_IP, testID, ip);
        return format("tjump %s,%d", VarID(testID).toString(meta).c_str(), ip + 1);
    }
};
template<>
struct StubHandler<S_FalseJump> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_IP, testID, ip);
        auto test = VarID(testID).toValue(frame->localConstPtr);
        if (!test->getBoolean()) frame->ip = ip - 1;
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_2(BIT_W_VAR_ID, BIT_W_IP, testID, ip);
        return format("fjump %s,%d", VarID(testID).toString(meta).c_str(), ip + 1);
    }
};
template<>
struct StubHandler<S_Call> {
    FORCE_INLINE static void execute(int code, StackFrame* frame) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, 8, funcID, argID, argCount);
        auto func = VarID(funcID).toValue(frame->localConstPtr);
        auto args = VarID(argID).toValue(frame->localConstPtr);
        ASSERT(func->type == JSVT_Function);
        func->data.func->callFromVM(args, args + argCount);
    }
    FORCE_INLINE static string disassemble(int code, FuncMeta* meta) {
        DECODE_3(BIT_W_VAR_ID, BIT_W_VAR_ID, 8, funcID, argID, argCount);
        return format("call %s,%s,%d", VarID(funcID).toString(meta).c_str(), VarID(argID).toString(meta).c_str(), argCount);
    }
};

#undef DECODE_1
#undef DECODE_2
#undef DECODE_3

#endif
