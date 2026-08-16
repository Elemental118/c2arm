#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "irgen.h"
#include "types.h"

#define MAX_ASM_INSTRS 100
#define MAX_ASM_INSTR_LEN 33
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
			strncpy(cg->table.regs[i].var_name, var_name, MAX_ID_LEN - 1);
			cg->table.regs[i].var_name[MAX_ID_LEN - 1] = '\0';
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

static void codegen_assign(struct codegen *cg, struct instr *instr)
{
	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	if (instr->op1.kind == OPERAND_LITERAL) {
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d",
			"mov", regtable_store(cg, instr->dest_name), instr->op1.val);
	} else if (instr->op1.kind == OPERAND_NAME) {
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s",
			"mov", regtable_store(cg, instr->dest_name), regtable_store(cg, instr->op1.name));
	}
}

static void codegen_compute(struct codegen *cg, struct instr *instr)
{
	char op1_name[32];
	char op2_name[32];
	if (instr->op1.kind == OPERAND_LITERAL) {
		if (cg->pos == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d",
			"mov", regtable_store(cg, "u0"), instr->op1.val);
		strcpy(op1_name, "u0");
	} else {
		strncpy(op1_name, instr->op1.name, MAX_ID_LEN - 1);
		op1_name[MAX_ID_LEN - 1] = '\0';
	}

	if (instr->op2.kind == OPERAND_LITERAL) {
		if (cg->pos == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d",
			"mov", regtable_store(cg, "u1"), instr->op2.val);
		strcpy(op2_name, "u1");
	} else {
		strncpy(op2_name, instr->op2.name, MAX_ID_LEN - 1);
		op2_name[MAX_ID_LEN - 1] = '\0';
	}

	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	char opcode[5];
	if (!strcmp(instr->op, "+")) {
		strcpy(opcode, "add");
	} else if (!strcmp(instr->op, "-")) {
		strcpy(opcode, "sub");
	} else if (!strcmp(instr->op, "*")) {
		strcpy(opcode, "mul");
	} else if (!strcmp(instr->op, "/")) {
		strcpy(opcode, "sdiv");
	} else if (!strcmp(instr->op, "&")) {
		strcpy(opcode, "and");
	} else if (!strcmp(instr->op, "|")) {
		strcpy(opcode, "orr");
	} else if (!strcmp(instr->op, "^")) {
		strcpy(opcode, "eor");
	} else {
		fprintf(stderr, "assembly gen error\n");
		exit(1);
	}
	sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s, %s",
		opcode, regtable_store(cg, instr->dest_name),
		regtable_store(cg, op1_name), regtable_store(cg, op2_name));
}

static void codegen_func_stmt(struct codegen *cg, struct instr *instr)
{
	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	sprintf(cg->assembly[cg->pos++], "%s:", instr->dest_name);
}

void codegen_prog(struct codegen *cg, struct instr *ir)
{
	for (int i = 0; ; i++) {
		struct instr *instr = &ir[i];
		switch (instr->i_type) {
		case INSTR_ASSIGN:
			codegen_assign(cg, instr);
			break;
		case INSTR_COMPUTE:
			codegen_compute(cg, instr);
			break;
		case INSTR_FUNC_START:
			codegen_func_stmt(cg, instr);
			break;
		case INSTR_FUNC_END:
			break;
		case INSTR_EOF:
			return;
		case INSTR_ERR:
		default:
			fprintf(stderr, "assembly gen error\n");
			exit(1);
		}
	}
}

void codegen_print(struct codegen *cg)
{
	for (int i = 0; i < MAX_ASM_INSTRS; i++) {
		printf("%s\n", cg->assembly[i]);
		if (!cg->assembly[i][0]) {
			return;
		}
	}
}
