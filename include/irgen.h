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
	INSTR_COMPUTE
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

#endif
