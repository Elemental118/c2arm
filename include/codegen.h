#ifndef CODEGEN_H
#define CODEGEN_H

struct codegen *codegen_create(void);
void codegen_free(struct codegen *cg);

struct instr;
void codegen_prog(struct codegen *cg, struct instr *ir);

#endif
