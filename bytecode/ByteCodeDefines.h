
#ifndef BYTE_CODE_DEFINES_H
#define BYTE_CODE_DEFINES_H

#include "../runtime/JSVM.h"
#include "../runtime/JSFunction.h"
#include "../runtime/JSArray.h"
#include "../runtime/JSString.h"

#include "common.h"

enum ByteCodeType {
    BC_NewFunction, BC_NewArray,

    BC_Move,
    BC_Not, BC_Minus, BC_Len,
    BC_SetGlobal, BC_GetGlobal,

    BC_Add, BC_Sub, BC_Mul, BC_Div, BC_Mod, BC_Pow,
    BC_Less, BC_LessEq, BC_Greater, BC_GreaterEq, BC_Equal, BC_NEqual, 
    BC_SetArray, BC_GetArray,

    BC_Jump, BC_TrueJump, BC_FalseJump,

    BC_Call,
};

#define ENCODE_1(codeType, bw1, v1) \
    static_assert(bw1 + 8 <= 32, "");\
    ASSERT(codeType < (1 << 8));\
    ASSERT(v1 < (1 << bw1));\
    code = codeType | (v1 << 8);
#define ENCODE_2(codeType, bw1, bw2, v1, v2) \
    static_assert(bw1 + bw2 + 8 <= 32, "");\
    ASSERT(codeType < (1 << 8));\
    ASSERT(v1 < (1 << bw1)); ASSERT(v2 < (1 << bw2));\
    code = codeType | (v1 << 8) | (v2 << (8 + bw1));
#define ENCODE_3(codeType, bw1, bw2, bw3, v1, v2, v3) \
    static_assert(bw1 + bw2 + bw3 + 8 <= 32, "");\
    ASSERT(codeType < (1 << 8));\
    ASSERT(v1 < (1 << bw1)); ASSERT(v2 < (1 << bw2)); ASSERT(v3 < (1 << bw3));\
    code = codeType | (v1 << 8) | (v2 << (8 + bw1)) | (v3 << (8 + bw1 + bw2));

template<int>
struct ByteCodeHandler;

template<>
struct ByteCodeHandler<BC_NewFunction> {
    FORCE_INLINE static void emitCode(int &code, int destID, int metaIdx) {
        ENCODE_2(BC_NewFunction, BIT_W_VAR_ID, BIT_W_META_IDX, destID, metaIdx);
    }
};

template<>
struct ByteCodeHandler<BC_NewArray> {
    FORCE_INLINE static void emitCode(int &code, int destID, int srcID, int len) {
        ENCODE_3(BC_NewArray, BIT_W_VAR_ID, BIT_W_VAR_ID, 8, destID, srcID, len);
    }
};
template<>
struct ByteCodeHandler<BC_Move> {
    FORCE_INLINE static void emitCode(int &code, int destID, int srcID) {
        ENCODE_2(BC_Move, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
    }
};
template<>
struct ByteCodeHandler<BC_Not> {
    FORCE_INLINE static void emitCode(int &code, int destID, int srcID) {
        ENCODE_2(BC_Not, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
    }
};
template<>
struct ByteCodeHandler<BC_Len> {
    FORCE_INLINE static void emitCode(int &code, int destID, int srcID) {
        ENCODE_2(BC_Len, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
    }
};
template<>
struct ByteCodeHandler<BC_Minus> {
    FORCE_INLINE static void emitCode(int &code, int destID, int srcID) {
        ENCODE_2(BC_Minus, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, srcID);
    }
};
template<>
struct ByteCodeHandler<BC_SetGlobal> {
    FORCE_INLINE static void emitCode(int &code, int kID, int vID) {
        ENCODE_2(BC_SetGlobal, BIT_W_VAR_ID, BIT_W_VAR_ID, kID, vID);
    }
};
template<>
struct ByteCodeHandler<BC_GetGlobal> {
    FORCE_INLINE static void emitCode(int &code, int destID, int kID) {
        ENCODE_2(BC_GetGlobal, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, kID);
    }
};
template<>
struct ByteCodeHandler<BC_Add> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Add, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Sub> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Sub, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Mul> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Mul, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Div> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Div, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Mod> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Mod, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Pow> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Pow, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Less> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Less, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_LessEq> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_LessEq, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Greater> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Greater, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_GreaterEq> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_GreaterEq, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_Equal> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_Equal, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_NEqual> {
    FORCE_INLINE static void emitCode(int &code, int destID, int lID, int rID) {
        ENCODE_3(BC_NEqual, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, lID, rID);
    }
};
template<>
struct ByteCodeHandler<BC_SetArray> {
    FORCE_INLINE static void emitCode(int &code, int arrayID, int kID, int vID) {
        ENCODE_3(BC_SetArray, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, arrayID, kID, vID);
    }
};
template<>
struct ByteCodeHandler<BC_GetArray> {
    FORCE_INLINE static void emitCode(int &code, int destID, int arrayID, int kID) {
        ENCODE_3(BC_GetArray, BIT_W_VAR_ID, BIT_W_VAR_ID, BIT_W_VAR_ID, destID, arrayID, kID);
    }
};
template<>
struct ByteCodeHandler<BC_Jump> {
    FORCE_INLINE static void emitCode(int &code, int ip) {
        ENCODE_1(BC_Jump, BIT_W_IP, ip);
    }
};
template<>
struct ByteCodeHandler<BC_TrueJump> {
    FORCE_INLINE static void emitCode(int &code, int testID, int ip) {
        ENCODE_2(BC_TrueJump, BIT_W_VAR_ID, BIT_W_IP, testID, ip);
    }
};
template<>
struct ByteCodeHandler<BC_FalseJump> {
    FORCE_INLINE static void emitCode(int &code, int testID, int ip) {
        ENCODE_2(BC_FalseJump, BIT_W_VAR_ID, BIT_W_IP, testID, ip);
    }
};
template<>
struct ByteCodeHandler<BC_Call> {
    FORCE_INLINE static void emitCode(int &code, int funcID, int argID, int argCount) {
        ENCODE_3(BC_Call, BIT_W_VAR_ID, BIT_W_VAR_ID, 8, funcID, argID, argCount);
    }
};


#undef ENCODE_1
#undef ENCODE_2
#undef ENCODE_3

#endif
