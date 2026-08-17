#ifndef IRGEN_H
#define IRGEN_H

#include "parser.h"

#define MAX_INSTRS 100

enum operand_kind {
	OPERAND_NAME,
	OPERAND_LITERAL
};

enum instr_type {
	INSTR_ERR,
	
	INSTR_ASSIGN,
	INSTR_UN,
	INSTR_BIN,
	INSTR_FUNC_START,
	INSTR_FUNC_END,
	INSTR_EOF
};

struct operand {
	enum operand_kind kind;
	enum data_type d_type;
	union {
		char name[32];
		int  val;
	};
};

struct instr {
	enum instr_type i_type;
	enum data_type d_type;
	char dest_name[32];

	struct operand op1;

	// ONLY IF INSTR_BIN
	char op[3];
	struct operand op2;
	
};

struct irgen *irgen_create_and_load(void);
void irgen_free(struct irgen *irg);

struct instr *irgen_prog(struct irgen *irg, struct node *n);
void ir_free(struct instr *instrs);
void ir_print(struct instr *ir);

#endif
