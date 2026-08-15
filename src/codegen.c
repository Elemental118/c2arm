#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "irgen.h"
#include "types.h"

#define MAX_ASM_INSTRS 100
#define MAX_ASM_INSTR_LEN 25
#define MAX_REGS 7

struct reg {
	char reg_name[32];
	char var_name[32];
	int val;
};

struct regtable {
	struct reg regs[MAX_REGS];
	int pos;
};

struct codegen {
	char assembly[MAX_ASM_INSTRS][MAX_ASM_INSTR_LEN];
	int pos;
	struct regtable table;
};

struct codegen *codegen_create(void)
{
	struct codegen *cg = calloc(1, sizeof(*cg));
	return cg;
}

void codegen_free(struct codegen *cg)
{
	free(cg);
}
