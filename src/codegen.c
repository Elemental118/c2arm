#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "irgen.h"
#include "types.h"

#define MAX_ASM_INSTRS 100
#define MAX_ASM_INSTR_LEN 25
#define MAX_REGS 7
#define REGS_BEGIN 9

struct reg {
	char var_name[32];
	char reg_name[4];
	bool free;
};

struct regtable {
	struct reg regs[MAX_REGS];
};

struct codegen {
	char assembly[MAX_ASM_INSTRS][MAX_ASM_INSTR_LEN];
	int pos;
	struct regtable table;
};

struct codegen *codegen_create(void)
{
	struct codegen *cg = calloc(1, sizeof(*cg));
	for (int i = 0; i < MAX_REGS; i++) {
		sprintf(cg->table.regs[i].reg_name, "w%d", i + REGS_BEGIN);
		cg->table.regs[i].free = true;
	}
	return cg;
}

void codegen_free(struct codegen *cg)
{
	free(cg);
}

char *regtable_store(struct codegen *cg, char *var_name)
{
	for (int i = 0; i < MAX_REGS; i++) {
		if (cg->table.regs[i].free) {
			strcpy(cg->table.regs[i].var_name, var_name);
			cg->table.regs[i].free = false;
			return cg->table.regs[i].reg_name;
		}
		if (!strcmp(cg->table.regs[i].var_name, var_name)) {
			return cg->table.regs[i].reg_name;
		}
	}
	fprintf(stderr, "register allocation error\n");
	exit(1);
}

void codegen_stmt(struct codegen *cg, struct instr *i)
{
	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d", "mov", regtable_store(cg, i->dest_name), i->op1_val);
}
