#ifndef ASSEMBLERX86_H
#define ASSEMBLERX86_H

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
        emit(1, 0x52); // push edx
        emit(1, 0x55); // push ebp
        emit(2, 0x8b, 0xec); // mov ebp, esp
        emit(2, 0x81, 0xec); emitValue(MAX_LOCAL_COUNT * 4); // sub esp, MAX_LOCAL_COUNT * 4
    }
    void endBuild(){
        markLabel(&m_retLabel);
        emit(2, 0x8b, 0xe5);  // mov esp,ebp 
        emit(1, 0x5d); // pop ebp  
        emit(1, 0x5a); // pop edx  
        emit(1, 0xc3); // ret
    }

    void loadImm(int imm){
        emit(1, 0x68); emitValue(imm); // push imm
    }

    void loadLocal(int idx){
        emit(2, 0xff, 0xb5); emitValue(localIdx2EbpOff(idx)); // push dword ptr [ebp + idxOff]
    }
    void storeLocal(int idx) {
        emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
        emit(2, 0x89, 0x85); emitValue(localIdx2EbpOff(idx)); // mov dword ptr [ebp + idxOff], eax
        emit(2, 0x83, 0xc4); emitValue((char)4); // add esp, 4
    }
    void incLocal(int idx) {
        emit(2, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc dword ptr [ebp + idxOff]
    }
    void decLocal(int idx) {
        emit(2, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec dword ptr [ebp + idxOff]
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
    int localIdx2EbpOff(int idx) { return idx < 0 ? 8 - idx * 4 : -(1 + idx) * 4; }
private:
    char *m_codeBuf;
    int m_codeSize;
    x86Label m_retLabel;
};

#endif
