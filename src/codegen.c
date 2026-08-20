#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "irgen.h"
#include "types.h"

#define MAX_ASM_INSTRS 10000
#define MAX_ASM_INSTR_LEN 100
#define MAX_VARS 2044
#define SLOT_WIDTH 4
#define BASE_OFFSET 16
#define STACK_BYTES (MAX_VARS * SLOT_WIDTH + BASE_OFFSET)

char *scratch_regs[] = {"w9", "w10", "w11", "w12"};

struct var {
	char var_name[32];
	int  offset;
};

struct frame_table {
	struct var vars[MAX_VARS];
};

struct codegen {
	char assembly[MAX_ASM_INSTRS][MAX_ASM_INSTR_LEN];
	int pos;
	struct frame_table table;
	int scratch_pos;
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

int frame_table_offset(struct codegen *cg, char *var_name)
{
	for (int i = 0; i < MAX_VARS; i++) {
		if (!strcmp(cg->table.vars[i].var_name, var_name)) {
			return BASE_OFFSET + i * SLOT_WIDTH;
		} else if (!cg->table.vars[i].offset) {
			strncpy(cg->table.vars[i].var_name, var_name, MAX_ID_LEN - 1);
			cg->table.vars[i].var_name[MAX_ID_LEN - 1] = '\0';
			cg->table.vars[i].offset = BASE_OFFSET + i * SLOT_WIDTH;
			return cg->table.vars[i].offset;
		}
	}
	fprintf(stderr, "too many local variables\n");
	exit(1);
}

char *get_scratch(struct codegen *cg)
{
	return scratch_regs[cg->scratch_pos++ % (sizeof(scratch_regs) / sizeof(scratch_regs[0]))];
}

char *stack_load(struct codegen *cg, char *var_name)
{
	char *reg = scratch_regs[cg->scratch_pos++ % (sizeof(scratch_regs) / sizeof(scratch_regs[0]))];
	int offset = frame_table_offset(cg, var_name);
	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	sprintf(cg->assembly[cg->pos++], "\t%-7s%s, [x29, #%d]", "ldr", reg, offset);
	return reg;
}

void stack_store(struct codegen *cg, char *var_name, char *reg)
{
	int offset = frame_table_offset(cg, var_name);
	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	sprintf(cg->assembly[cg->pos++], "\t%-7s%s, [x29, #%d]", "str", reg, offset);
}

static void codegen_assign(struct codegen *cg, struct instr *instr)
{
	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	char *dest_reg = get_scratch(cg);
	if (instr->op1.kind == OPERAND_LITERAL) {
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d",
			"mov", dest_reg, instr->op1.val);
	} else if (instr->op1.kind == OPERAND_NAME) {
		char *src = stack_load(cg, instr->op1.name);
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s",
			"mov", dest_reg, src);
	}
	stack_store(cg, instr->dest_name, dest_reg);
}

static void codegen_compute(struct codegen *cg, struct instr *instr)
{
	char op1_reg[4];
	char op2_reg[4];
	if (instr->op1.kind == OPERAND_LITERAL) {
		if (cg->pos == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		strcpy(op1_reg, get_scratch(cg));
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d", "mov", op1_reg, instr->op1.val);
	} else {
		strcpy(op1_reg, stack_load(cg, instr->op1.name));
	}

	if (instr->op2.kind == OPERAND_LITERAL) {
		if (cg->pos == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		strcpy(op2_reg, get_scratch(cg));
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d", "mov", op2_reg, instr->op2.val);
	} else {
		strcpy(op2_reg, stack_load(cg, instr->op2.name));
	}

	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	char opcode[5];
	char *dest_reg = get_scratch(cg);
	if (!strcmp(instr->op, "+")) {
		strcpy(opcode, "add");
	} else if (!strcmp(instr->op, "-") && instr->i_type == INSTR_BIN) {
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
	} else if (!strcmp(instr->op, "%")) {
		if (cg->pos + 1 == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		char *temp1_reg = get_scratch(cg);
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s, %s",
			"sdiv", temp1_reg, op1_reg, op2_reg);
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s, %s, %s",
			"msub", dest_reg, temp1_reg, op2_reg, op1_reg);
		stack_store(cg, instr->dest_name, dest_reg);
		return;
	} else if (!strcmp(instr->op, "<") || !strcmp(instr->op, ">")
		|| !strcmp(instr->op, "<=") || !strcmp(instr->op, ">=")
		|| !strcmp(instr->op, "==") || !strcmp(instr->op, "!=")) {
		if (cg->pos + 1 == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "cmp", op1_reg, op2_reg);
		char cset_code[3];
		if (!strcmp(instr->op, "<")) {
			strcpy(cset_code, "lt");
		} else if (!strcmp(instr->op, ">")) {
			strcpy(cset_code, "gt");
		} else if (!strcmp(instr->op, "<=")) {
			strcpy(cset_code, "le");
		} else if (!strcmp(instr->op, ">=")) {
			strcpy(cset_code, "ge");
		} else if (!strcmp(instr->op, "==")) {
			strcpy(cset_code, "eq");
		} else if (!strcmp(instr->op, "!=")) {
			strcpy(cset_code, "ne");
		}
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "cset", dest_reg, cset_code);
		stack_store(cg, instr->dest_name, dest_reg);
		return;
	} else if (!strcmp(instr->op, "-")) {
		if (instr->i_type == INSTR_UN) {
			sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "neg", dest_reg, op1_reg);
		}
		stack_store(cg, instr->dest_name, dest_reg);
		return;
	} else if (!strcmp(instr->op, "~")) {
		if (instr->i_type == INSTR_UN) {
			sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "mvn", dest_reg, op1_reg);
		}
		stack_store(cg, instr->dest_name, dest_reg);
		return;
	} else if (!strcmp(instr->op, "!")) {
		if (instr->i_type == INSTR_UN) {
			sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "cmp", op1_reg, "#0");
			sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "cset", dest_reg, "eq");
		}
		stack_store(cg, instr->dest_name, dest_reg);
		return;
	} else {
		fprintf(stderr, "assembly gen error\n");
		exit(1);
	}
	sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s, %s", opcode, dest_reg, op1_reg, op2_reg);
	stack_store(cg, instr->dest_name, dest_reg);
}

static void codegen_jmp(struct codegen *cg, struct instr *instr)
{
	char op1_reg[32];
	if (instr->op1.kind == OPERAND_LITERAL) {
		if (cg->pos == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		strcpy(op1_reg, get_scratch(cg));
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d", "mov", op1_reg, instr->op1.val);
	} else {
		strcpy(op1_reg, stack_load(cg, instr->op1.name));
	}

	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	if (!strcmp(instr->op, "j")) {
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s", "b", instr->dest_name);
	} else {
		if (cg->pos == MAX_ASM_INSTRS - 1) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "cmp", op1_reg, "#0");
		if (!strcmp(instr->op, "t")) {
			sprintf(cg->assembly[cg->pos++], "\t%-7s%s", "b.ne", instr->dest_name);
		} else if (!strcmp(instr->op, "f")) {
			sprintf(cg->assembly[cg->pos++], "\t%-7s%s", "b.eq", instr->dest_name);
		} else {
			fprintf(stderr, "assembly gen error\n");
			exit(1);
		}
	}	
}

static void codegen_prologue(struct codegen *cg)
{
	if (cg->pos == MAX_ASM_INSTRS - 1) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	if (STACK_BYTES % 4096 != 0) {
		fprintf(stderr, "illegal stack slot count\n");
		exit(1);
	}	
	sprintf(cg->assembly[cg->pos++], "\t%-7ssp, sp, #%d", "sub", STACK_BYTES);
	sprintf(cg->assembly[cg->pos++], "\t%-7sx29, x30, [sp]", "stp");
	sprintf(cg->assembly[cg->pos++], "\t%-7sx29, sp", "mov");
}

static void codegen_epilogue(struct codegen *cg)
{
	if (cg->pos == MAX_ASM_INSTRS - 1) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	if ((STACK_BYTES) % 4096 != 0){
		fprintf(stderr, "illegal stack slot count\n");
		exit(1);
	}	
	sprintf(cg->assembly[cg->pos++], "\t%-7sx29, x30, [sp]", "ldp");
	sprintf(cg->assembly[cg->pos++], "\t%-7ssp, sp, #%d", "add", STACK_BYTES);
	strcpy(cg->assembly[cg->pos++], "\tret");
}


static void codegen_label(struct codegen *cg, struct instr *instr, bool global)
{
	if (global) {
		if (cg->pos == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		sprintf(cg->assembly[cg->pos++], ".global %s", instr->dest_name);
	}
	if (cg->pos == MAX_ASM_INSTRS) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	sprintf(cg->assembly[cg->pos++], "%s:", instr->dest_name);
}

static void codegen_ret(struct codegen *cg, struct instr *instr)
{
	if (!strcmp(instr->op, "r")) {
		char op1_name[32];
		if (instr->op1.kind == OPERAND_LITERAL) {
			if (cg->pos == MAX_ASM_INSTRS) {
				fprintf(stderr, "too many assembly instructions\n");
				exit(1);
			}
			strcpy(op1_name, get_scratch(cg));
			sprintf(cg->assembly[cg->pos++], "\t%-7s%s, #%d",
				"mov", op1_name, instr->op1.val);
		} else {
			strcpy(op1_name, stack_load(cg, instr->op1.name));
		}
		if (cg->pos == MAX_ASM_INSTRS) {
			fprintf(stderr, "too many assembly instructions\n");
			exit(1);
		}
		sprintf(cg->assembly[cg->pos++], "\t%-7s%s, %s", "mov", "w0", op1_name);
	}
	if (cg->pos == MAX_ASM_INSTRS ) {
		fprintf(stderr, "too many assembly instructions\n");
		exit(1);
	}
	codegen_epilogue(cg);
}

void codegen_prog(struct codegen *cg, struct instr *ir)
{
	for (int i = 0; ; i++) {
		struct instr *instr = &ir[i];
		switch (instr->i_type) {
		case INSTR_ASSIGN:
			codegen_assign(cg, instr);
			break;
		case INSTR_UN:
		case INSTR_BIN:
			codegen_compute(cg, instr);
			break;
		case INSTR_JMP:
			codegen_jmp(cg, instr);
			break;
		case INSTR_LABEL:
			codegen_label(cg, instr, false);
			break;
		case INSTR_FUNC_START:
			codegen_label(cg, instr, true);
			codegen_prologue(cg);
			break;
		case INSTR_FUNC_END:
			break;
		case INSTR_RET:
			codegen_ret(cg, instr);
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
