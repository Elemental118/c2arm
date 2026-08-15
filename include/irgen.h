#ifndef IRGEN_H
#define IRGEN_H

#include "parser.h"

enum operand_kind {
	OPERAND_NAME,
	OPERAND_LITERAL
};

enum instr_type {
	INSTR_ERR,
	
	INSTR_ASSIGN,
	INSTR_COMPUTE,
	INSTR_FUNC_START,
	INSTR_FUNC_END,
	INSTR_EOF
};

struct instr {
	enum instr_type i_type;
	enum data_type d_type;
	char dest_name[32];

	enum operand_kind op1_kind;
	union {
		char op1_name[32];
		int  op1_val;
	};

	// ONLY IF INSTR_COMPUTE
	char op;
	enum operand_kind op2_kind;
	union {
		char op2_name[32];
		int  op2_val;
	};
};

struct irgen *irgen_create_and_load(void);
void irgen_free(struct irgen *ir);

struct instr *irgen_prog(struct irgen *ir, struct node *n);
void instr_free(struct instr *instrs);

#endif
