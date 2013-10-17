
#ifndef BYTE_CODE_H
#define BYTE_CODE_H

struct StackFrame;
struct FuncMeta;
typedef shared_ptr<FuncMeta> FuncMetaPtr;
 
void emitCode(const FuncMetaPtr &meta);
#endif
