
#ifndef INTERPRETER_H
#define INTERPRETER_H

struct StackFrame;
struct FuncMeta;
typedef shared_ptr<FuncMeta> FuncMetaPtr;

void execute(StackFrame *stopFrame);
void disassemble(ostream& so, const FuncMetaPtr &meta, int depth);

#endif
