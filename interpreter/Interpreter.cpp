
#include "../runtime/pch.h"
#include "../bytecode/ByteCode.h"
#include "../bytecode/ByteCodeDefines.h"
#include "../runtime/AST.h"
#include "Stubs.h"

static void _execute(StackFrame *stopFrame) {
    auto vm = JSVM::instance();
    for (auto frame = vm->topFrame(); frame != stopFrame; frame = vm->topFrame()) {
        auto maxIp = (int)frame->func->meta->codes.size();
        auto codes = &frame->func->meta->codes[0];
        while (frame->ip < maxIp) {
            int code = codes[frame->ip];
            switch (code & 0xff) {
            case BC_NewFunction: StubHandler<BC_NewFunction>::execute(code, frame); break;
            case BC_NewArray: StubHandler<BC_NewArray>::execute(code, frame); break;
#if 1
            case BC_Move: {
							StubHandler<BC_Move>::compile(code, frame); 
							StubHandler<BC_Move>::run(code, frame); 
							break;
						  }
#else
            case BC_Move: StubHandler<BC_Move>::execute(code, frame); break;
#endif
            case BC_Not: StubHandler<BC_Not>::execute(code, frame); break;
            case BC_Minus: StubHandler<BC_Minus>::execute(code, frame); break;
            case BC_Len: StubHandler<BC_Len>::execute(code, frame); break;
            case BC_SetGlobal: StubHandler<BC_SetGlobal>::execute(code, frame); break;
            case BC_GetGlobal: StubHandler<BC_GetGlobal>::execute(code, frame); break;
            case BC_Add: StubHandler<BC_Add>::execute(code, frame); break;
            case BC_Sub: StubHandler<BC_Sub>::execute(code, frame); break;
            case BC_Mul: StubHandler<BC_Mul>::execute(code, frame); break;
            case BC_Div: StubHandler<BC_Div>::execute(code, frame); break;
            case BC_Mod: StubHandler<BC_Mod>::execute(code, frame); break;
            case BC_Pow: StubHandler<BC_Pow>::execute(code, frame); break;
            case BC_Less: StubHandler<BC_Less>::execute(code, frame); break;
            case BC_LessEq: StubHandler<BC_LessEq>::execute(code, frame); break;
            case BC_Greater: StubHandler<BC_Greater>::execute(code, frame); break;
            case BC_GreaterEq: StubHandler<BC_GreaterEq>::execute(code, frame); break;
            case BC_Equal: StubHandler<BC_Equal>::execute(code, frame); break;
            case BC_NEqual: StubHandler<BC_NEqual>::execute(code, frame); break;
            case BC_SetArray: StubHandler<BC_SetArray>::execute(code, frame); break;
            case BC_GetArray: StubHandler<BC_GetArray>::execute(code, frame); break;
            case BC_Jump: StubHandler<BC_Jump>::execute(code, frame); break;
            case BC_TrueJump: StubHandler<BC_TrueJump>::execute(code, frame); break;
            case BC_FalseJump: StubHandler<BC_FalseJump>::execute(code, frame); break;
            case BC_Call: StubHandler<BC_Call>::execute(code, frame); 
                          ++frame->ip;
                          goto l_endWhile;
            default: ASSERT(0); break;
            }
            ++frame->ip;
        }
l_endWhile:
        if (frame->ip == maxIp) {
            vm->popFrame();
        }
    }
}
void execute(StackFrame *stopFrame) {
    try {
        _execute(stopFrame);
    } catch(Exception& e) {
        for (;;) {
            auto frame = JSVM::instance()->topFrame();
            if (frame == stopFrame) break;
            auto meta = frame->func->meta;
            int line = meta->ip2line[frame->ip];
            e.addLine(format("%s(%d):", meta->fileName.c_str(), line));
            JSVM::instance()->popFrame();
        }
        throw;
    }
}
void disassemble(ostream& so, const FuncMetaPtr &meta, int depth) {
    for (int i = 0; i < (int)meta->codes.size(); ++i) {
        int code = meta->codes[i];

        so << format("%s%3d:", tabString(depth).c_str(), i + 1);
        string str;
        switch (code & 0xff) {
            case BC_NewFunction: str = StubHandler<BC_NewFunction>::disassemble(code, meta.get()); break;
            case BC_NewArray: str = StubHandler<BC_NewArray>::disassemble(code, meta.get()); break;
            case BC_Move: str = StubHandler<BC_Move>::disassemble(code, meta.get()); break;
            case BC_Not: str = StubHandler<BC_Not>::disassemble(code, meta.get()); break;
            case BC_Minus: str = StubHandler<BC_Minus>::disassemble(code, meta.get()); break;
            case BC_Len: str = StubHandler<BC_Len>::disassemble(code, meta.get()); break;
            case BC_SetGlobal: str = StubHandler<BC_SetGlobal>::disassemble(code, meta.get()); break;
            case BC_GetGlobal: str = StubHandler<BC_GetGlobal>::disassemble(code, meta.get()); break;
            case BC_Add: str = StubHandler<BC_Add>::disassemble(code, meta.get()); break;
            case BC_Sub: str = StubHandler<BC_Sub>::disassemble(code, meta.get()); break;
            case BC_Mul: str = StubHandler<BC_Mul>::disassemble(code, meta.get()); break;
            case BC_Div: str = StubHandler<BC_Div>::disassemble(code, meta.get()); break;
            case BC_Mod: str = StubHandler<BC_Mod>::disassemble(code, meta.get()); break;
            case BC_Pow: str = StubHandler<BC_Pow>::disassemble(code, meta.get()); break;
            case BC_Less: str = StubHandler<BC_Less>::disassemble(code, meta.get()); break;
            case BC_LessEq: str = StubHandler<BC_LessEq>::disassemble(code, meta.get()); break;
            case BC_Greater: str = StubHandler<BC_Greater>::disassemble(code, meta.get()); break;
            case BC_GreaterEq: str = StubHandler<BC_GreaterEq>::disassemble(code, meta.get()); break;
            case BC_Equal: str = StubHandler<BC_Equal>::disassemble(code, meta.get()); break;
            case BC_NEqual: str = StubHandler<BC_NEqual>::disassemble(code, meta.get()); break;
            case BC_SetArray: str = StubHandler<BC_SetArray>::disassemble(code, meta.get()); break;
            case BC_GetArray: str = StubHandler<BC_GetArray>::disassemble(code, meta.get()); break;
            case BC_Jump: str = StubHandler<BC_Jump>::disassemble(code, meta.get()); break;
            case BC_TrueJump: str = StubHandler<BC_TrueJump>::disassemble(code, meta.get()); break;
            case BC_FalseJump: str = StubHandler<BC_FalseJump>::disassemble(code, meta.get()); break;
            case BC_Call: str = StubHandler<BC_Call>::disassemble(code, meta.get()); break;
            default: ASSERT(0); break;
        }
        so << str << endl;
        if ((code & 0xff) == BC_NewFunction) {
            disassemble(so, StubHandler<BC_NewFunction>::getMetaFromCode(code), depth + 1);
        }
    }

}
