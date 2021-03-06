#ifndef ASSEMBLERX86_H
#define ASSEMBLERX86_H

//============================== 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;
//============================== 

//============================== lexical analysis
enum TokenID {
    TID_LP, TID_RP, TID_LBRACE, TID_RBRACE, TID_LBRACKET, TID_RBRACKET, TID_COMMA, TID_SEMICELON,
    TID_IF, TID_ELSE, TID_FOR, TID_WHILE, TID_CONTINUE, TID_BREAK, TID_RETURN,
    TID_OP_NOT, TID_OP_INC, TID_OP_DEC,
    TID_OP_ASSIGN,
    TID_OP_AND, TID_OP_OR,
    TID_OP_ADD, TID_OP_SUB, TID_OP_MUL, TID_OP_DIV, TID_OP_MOD,   
    TID_OP_LESS, TID_OP_LESSEQ, TID_OP_GREATER, TID_OP_GREATEREQ, TID_OP_EQUAL, TID_OP_NEQUAL, 
    TID_TYPE_INT, TID_TYPE_STRING, TID_TYPE_VOID,
    TID_TRUE, TID_FALSE,
    // special 
    TID_INT, TID_ID, TID_STRING,
    TID_EOF,
};

class x86Label { 
public:
    x86Label(): m_address(NULL){}
    ~x86Label() { ASSERT(m_address != NULL); }
    void mark(char *address) {
        ASSERT(m_address == NULL);
        m_address = address;
        bindRefs();
    }
    void addRef(char *ref) {
        m_refs.push_back(ref);
        bindRefs();
    }
private:
    void bindRefs() {
        if (m_address == NULL) return;
        for (int i = 0; i < (int)m_refs.size(); ++i) {
            *(int*)m_refs[i] = int(m_address - (m_refs[i] + 4));
        }
        m_refs.clear();
    }
private:
    char *m_address;
    vector<char*> m_refs;
};

class x86AssemblyBuilder {
public:
    x86AssemblyBuilder(char *codeBuf): m_codeBuf(codeBuf), m_codeSize(0){}
    int getCodeSize() const{ return m_codeSize;}

    void beginBuild(){
        emit(1, 0x52); // push rdx
        emit(1, 0x55); // push rbp
    }
    void endBuild(){
        emit(1, 0x5d); // pop rbp  
        emit(1, 0x5a); // pop rdx  
        emit(1, 0xc3); // ret
    }

    void loadImm(int imm){
        emit(1, 0x68); emitValue(imm); // push imm
    }

    void loadLocal(int idx){
		emit(3, 0x48, 0x89, 0xeb);		//mov rbp rbx
		emit(4, 0x48, 0x83, 0xc3, localIdx2RbpOff(idx)); //add idxoff, rbx
		emit(3, 0x48, 0x8b, 0x03);						// mov QWORD PTR [rbx],rax
	    emit(3, 0xff, 0x70, localIdx2ValueOff(idx)); // push QWORD PTR [rax+idxvalueoff]
	    emit(3, 0xff, 0x70, localIdx2ValueOff(idx)+0x8); // push QWORD PTR [rax+idxvalueoff+1]
	}
    void storeLocal(int idx) {
		emit(3, 0x48, 0x89, 0xeb);		//mov rbp rbx
		emit(4, 0x48, 0x83, 0xc3, localIdx2RbpOff(idx)); //add idxoff, rbx
		emit(3, 0x48, 0x8b, 0x03);						// mov QWORD PTR [rbx],rax
        emit(1, 0x5b);              									// popq rbx
        emit(4, 0x48, 0x89, 0x58, localIdx2ValueOff(idx)+0x8);	// mov rbx, dword ptr [rax + idxvalueoff+1]
		emit(1, 0x5b);              									// popq rbx
        emit(4, 0x48, 0x89, 0x58, localIdx2ValueOff(idx));	// mov rbx, dword ptr [rax + idxvalueoff]

		//emit(1, 0xcc);					//sigabort
    }
    void incLocal(int idx) {
        emit(2, 0xff, 0x85); emitValue(localIdx2RbpOff(idx)); // inc dword ptr [rbp + idxOff]
    }
    void decLocal(int idx) {
        emit(2, 0xff, 0x8d); emitValue(localIdx2RbpOff(idx)); // dec dword ptr [rbp + idxOff]
    }
    void pop(int n){
        emit(2, 0x81, 0xc4); emitValue(n * 4); // add esp, n * 4
    }
    void dup(){
        emit(3, 0xff, 0x34, 0x24); // push dword ptr [esp]
    }

    void doArithmeticOp(TokenID opType) {
        emit(4, 0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4]
        switch (opType) {
            case TID_OP_ADD: 
                emit(3, 0x03, 0x04, 0x24); // add eax, dword ptr [esp]
                break;
            case TID_OP_SUB: 
                emit(3, 0x2b, 0x04, 0x24); // sub eax, dword ptr [esp]
                break;
            case TID_OP_MUL: 
                emit(4, 0x0f, 0xaf, 0x04, 0x24); // imul eax, dword ptr [esp]
                break;
            case TID_OP_DIV: 
            case TID_OP_MOD: 
                emit(2, 0x33, 0xd2); // xor edx, edx
                emit(3, 0xf7, 0x3c, 0x24); // idiv dword ptr [esp]
                if (opType == TID_OP_MOD) {
                    emit(2, 0x8b, 0xc2); // mov eax, edx
                }
                break;  
            default: ASSERT(0); break;
        }
        emit(4, 0x89, 0x44, 0x24, 0x04); // mov dword ptr [esp+4], eax
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
    }
    void cmp(TokenID cmpType) {
        x86Label label_1, label_0, label_end;
        emit(4, 0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4] 
        emit(3, 0x8b, 0x14, 0x24); // mov edx, dword ptr[esp]
        emit(2, 0x83, 0xc4); emitValue((char)8);// add esp, 8
        emit(2, 0x3b, 0xc2); // cmp eax, edx
        condJmp(cmpType, &label_1);
        jmp(&label_0);
        markLabel(&label_1);
        emit(2, 0x6a, 0x01); // push 1
        jmp(&label_end);
        markLabel(&label_0);
        emit(2, 0x6a, 0x00); // push 0
        markLabel(&label_end);
    }

    void markLabel(x86Label *label){ label->mark(m_codeBuf + m_codeSize); }
    void jmp(x86Label *label) { 
        emit(1, 0xe9);
        char *ref = m_codeBuf + m_codeSize;
        emitValue(NULL);
        label->addRef(ref); 
    }
    void trueJmp(x86Label *label) {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
        emit(2, 0x85, 0xc0); // test eax, eax
        condJmp(TID_OP_NEQUAL, label); 
    }
    void falseJmp(x86Label *label) {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
        emit(2, 0x85, 0xc0); // test eax, eax
        condJmp(TID_OP_EQUAL, label); 
    }
    void ret() { jmp(&m_retLabel); }
    void retExpr() {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(3, 0x83, 0xc4, 0x04); // add esp, 4
        jmp(&m_retLabel);
    }

private:
    void emit(int n, ...) {
        va_list args;
        va_start(args, n);
        for (int i = 0; i < n; ++i) m_codeBuf[m_codeSize++] = (char)va_arg(args, int);
        va_end(args);
    }
    template<typename T>
    void emitValue(T val) {
        memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
        m_codeSize += sizeof(val);
    }
private:
    void condJmp(TokenID tid, x86Label *label) {
        switch (tid) {
            case TID_OP_LESS: emit(2, 0x0f, 0x8c); break;
            case TID_OP_LESSEQ: emit(2, 0x0f, 0x8e); break;
            case TID_OP_GREATER: emit(2, 0x0f, 0x8f); break;
            case TID_OP_GREATEREQ: emit(2, 0x0f, 0x8d); break;
            case TID_OP_EQUAL: emit(2, 0x0f, 0x84); break;
            case TID_OP_NEQUAL: emit(2, 0x0f, 0x85); break;
        }
        char *ref = m_codeBuf + m_codeSize;
        emitValue(NULL);
        label->addRef(ref);
    }
private:
    int localIdx2RbpOff(int idx) { if (idx >> 7)  
										return 0x18;
									else
										return 0x10;
								}
    int localIdx2ValueOff(int idx) { return (idx & 0x3f) * 0x10; }
private:
    char *m_codeBuf;
    int m_codeSize;
    x86Label m_retLabel;
};

#endif
