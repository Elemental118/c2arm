#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "irgen.h"
#include "parser.h"
#include "types.h"

#define MAX_INSTRS 100

struct irgen {
	struct instr instrs[MAX_INSTRS];
	int pos;
};

static void irgen_stmt(struct irgen *i, struct node *n)
{
	if (i->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	i->instrs[i->pos].i_type = INSTR_ASSIGN;
	i->instrs[i->pos].d_type = n->children[0]->d_type;
	strcpy(i->instrs[i->pos].dest_name, n->children[0]->name);
	i->instrs[i->pos].op1_kind = OPERAND_LITERAL;
	i->instrs[i->pos++].op1_val = n->children[1]->val;
}
