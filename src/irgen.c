#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "irgen.h"
#include "parser.h"
#include "types.h"

#define MAX_INSTRS 100

struct irgen {
	struct instr *instrs;
	int pos;
};

static void irgen_stmt(struct irgen *irg, struct node *n)
{
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_ASSIGN;
	irg->instrs[irg->pos].d_type = n->children[0]->d_type;
	strcpy(irg->instrs[irg->pos].dest_name, n->children[0]->name);
	irg->instrs[irg->pos].op1_kind = OPERAND_LITERAL;
	irg->instrs[irg->pos++].op1_val = n->children[1]->val;
}

static void irgen_func(struct irgen *irg, struct node *n)
{
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_FUNC_START;
	irg->instrs[irg->pos].d_type = n->d_type;
	strcpy(irg->instrs[irg->pos++].dest_name, n->name);
	for (int i = 0; i < n->children_num; i++) {
		irgen_stmt(irg, n->children[i]);
	}
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos++].i_type = INSTR_FUNC_END;
}

struct instr *irgen_prog(struct irgen *irg, struct node *ast)
{
	for (int i = 0; i < ast->children_num; i++) {
		irgen_func(irg, ast->children[i]);
	}
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos++].i_type = INSTR_EOF;
	struct instr *instrs = malloc(irg->pos * sizeof(*instrs));
	memcpy(instrs, irg->instrs, irg->pos * sizeof(*instrs));
	return instrs;

}

void instr_free(struct instr *instrs)
{
	free(instrs);
}

struct irgen *irgen_create_and_load(void)
{
	struct irgen *irg = calloc(1, sizeof(*irg));
	irg->instrs = malloc(MAX_INSTRS * sizeof(*irg->instrs));
	return irg;
}

void irgen_free(struct irgen *irg)
{
	free(irg->instrs);
	free(irg);
}
