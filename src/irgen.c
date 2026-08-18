#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "irgen.h"
#include "lexer.h"
#include "parser.h"
#include "types.h"

struct irgen {
	struct instr *instrs;
	int pos;
	int tmp_count;
	int label_count;
};

static struct operand irgen_expr(struct irgen *irg, struct node *n)
{
	struct operand op;
	op.d_type = n->d_type;
	if (n->n_type == NODE_INT_LIT) {
		op.kind = OPERAND_LITERAL;
		op.val = n->val;
	} else if (n->n_type == NODE_VAR_NAME) {
		op.kind = OPERAND_NAME;
		snprintf(op.name, MAX_ID_LEN, "%s_%d", n->name, n->id);
	} else if (n->n_type == NODE_UN) {
		op.kind = OPERAND_NAME;
		struct operand left = irgen_expr(irg, n->children[0]);
		sprintf(op.name, "t%d", irg->tmp_count);

		if (irg->pos == MAX_INSTRS) {
			fprintf(stderr, "too many IR instructions\n");
			exit(1);
		}
		irg->instrs[irg->pos].op1 = left;

		irg->instrs[irg->pos].i_type = INSTR_UN;
		irg->instrs[irg->pos].d_type = n->d_type;
		strcpy(irg->instrs[irg->pos].op, n->op);
		sprintf(irg->instrs[irg->pos++].dest_name, "t%d", irg->tmp_count++);
	} else if (n->n_type == NODE_BIN && strcmp(n->op, "=")) {
		op.kind = OPERAND_NAME;
		struct operand left = irgen_expr(irg, n->children[0]);
		struct operand right = irgen_expr(irg, n->children[1]);
		sprintf(op.name, "t%d", irg->tmp_count);

		if (irg->pos == MAX_INSTRS) {
			fprintf(stderr, "too many IR instructions\n");
			exit(1);
		}
		irg->instrs[irg->pos].op1 = left;
		irg->instrs[irg->pos].op2 = right;

		irg->instrs[irg->pos].i_type = INSTR_BIN;
		irg->instrs[irg->pos].d_type = n->d_type;
		strcpy(irg->instrs[irg->pos].op, n->op);
		sprintf(irg->instrs[irg->pos++].dest_name, "t%d", irg->tmp_count++);
	} else if (n->n_type == NODE_BIN) {
		struct operand right = irgen_expr(irg, n->children[1]);
		if (irg->pos == MAX_INSTRS) {
			fprintf(stderr, "too many IR instructions\n");
			exit(1);
		}
		irg->instrs[irg->pos].op1 = right;
		irg->instrs[irg->pos].i_type = INSTR_ASSIGN;
		irg->instrs[irg->pos].d_type = n->children[0]->d_type;
		snprintf(irg->instrs[irg->pos].dest_name, MAX_ID_LEN, "%s_%d", n->children[0]->name, n->children[0]->id);
		op.kind = OPERAND_NAME;
		strcpy(op.name, irg->instrs[irg->pos++].dest_name);
	} else {
		fprintf(stderr, "IR gen error\n");
		exit(1);
	}
	return op;
}

static void irgen_stmt(struct irgen *irg, struct node *n);

static void irgen_if(struct irgen *irg, struct node *n)
{
	struct operand cond = irgen_expr(irg, n->children[0]);
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	int false_target = irg->label_count++;
	irg->instrs[irg->pos].op1 = cond;
	irg->instrs[irg->pos].i_type = INSTR_JMP;
	strcpy(irg->instrs[irg->pos].op, "f");
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", false_target);
	irgen_stmt(irg, n->children[1]);
	int end;
	if (n->children_num == 3) {
		if (irg->pos == MAX_INSTRS) {
			fprintf(stderr, "too many IR instructions\n");
			exit(1);
		}
		end = irg->label_count++;
		irg->instrs[irg->pos].i_type = INSTR_JMP;
		strcpy(irg->instrs[irg->pos].op, "j");
		snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", end);
	}
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_LABEL;
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", false_target);
	if (n->children_num == 3) {
		irgen_stmt(irg, n->children[2]);
		if (irg->pos == MAX_INSTRS) {
			fprintf(stderr, "too many IR instructions\n");
			exit(1);
		}
		irg->instrs[irg->pos].i_type = INSTR_LABEL;
		snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", end);
	}
}

static void irgen_while(struct irgen *irg, struct node *n)
{
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_LABEL;
	int start = irg->label_count++;
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", start);

	struct operand cond = irgen_expr(irg, n->children[0]);
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	int end = irg->label_count++;
	irg->instrs[irg->pos].op1 = cond;
	irg->instrs[irg->pos].i_type = INSTR_JMP;
	strcpy(irg->instrs[irg->pos].op, "f");
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", end);

	irgen_stmt(irg, n->children[1]);

	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_JMP;
	strcpy(irg->instrs[irg->pos].op, "j");
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", start);

	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_LABEL;
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", end);
}

static void irgen_do(struct irgen *irg, struct node *n)
{
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_LABEL;
	int start = irg->label_count++;
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", start);

	irgen_stmt(irg, n->children[0]);

	struct operand cond = irgen_expr(irg, n->children[1]);
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].op1 = cond;
	irg->instrs[irg->pos].i_type = INSTR_JMP;
	strcpy(irg->instrs[irg->pos].op, "t");
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", start);
}

static void irgen_for(struct irgen *irg, struct node *n)
{
	irgen_expr(irg, n->children[0]);

	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_LABEL;
	int start = irg->label_count++;
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", start);

	struct operand cond = irgen_expr(irg, n->children[1]);
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	int end = irg->label_count++;
	irg->instrs[irg->pos].op1 = cond;
	irg->instrs[irg->pos].i_type = INSTR_JMP;
	strcpy(irg->instrs[irg->pos].op, "f");
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", end);

	irgen_stmt(irg, n->children[3]);

	irgen_expr(irg, n->children[2]);

	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_JMP;
	strcpy(irg->instrs[irg->pos].op, "j");
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", start);

	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_LABEL;
	snprintf(irg->instrs[irg->pos++].dest_name, MAX_LABEL_LEN, "L%d", end);
}

static void irgen_block(struct irgen *irg, struct node *n);

static void irgen_stmt(struct irgen *irg, struct node *n)
{
	if (n->n_type == NODE_BLOCK) {
		irgen_block(irg, n);
		return;
	} else if (n->n_type == NODE_IF) {
		irgen_if(irg, n);
		return;
	} else if (n->n_type == NODE_WHILE) {
		irgen_while(irg, n);
		return;
	} else if (n->n_type == NODE_DO) {
		irgen_do(irg, n);
		return;
	} else if (n->n_type == NODE_FOR) {
		irgen_for(irg, n);
		return;
	} else {
		irgen_expr(irg, n);
	}
	
}

static void irgen_block(struct irgen *irg, struct node *n)
{
	for (int i = 0; i < n->children_num; i++) {
		irgen_stmt(irg, n->children[i]);
	}
}

static void irgen_func(struct irgen *irg, struct node *n)
{
	if (irg->pos == MAX_INSTRS) {
		fprintf(stderr, "too many IR instructions\n");
		exit(1);
	}
	irg->instrs[irg->pos].i_type = INSTR_FUNC_START;
	irg->instrs[irg->pos].d_type = n->d_type;
	strncpy(irg->instrs[irg->pos].dest_name, n->name, MAX_ID_LEN - 1);
	irg->instrs[irg->pos++].dest_name[MAX_ID_LEN - 1] = '\0';
	irgen_block(irg, n->children[0]);
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
	struct instr *ir = malloc(irg->pos * sizeof(*ir));
	memcpy(ir, irg->instrs, irg->pos * sizeof(*ir));
	return ir;

}

void ir_free(struct instr *ir)
{
	free(ir);
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

void ir_print(struct instr *ir)
{
	for (int i = 0; ir[i].i_type != INSTR_EOF; i++) {
		switch (ir[i].i_type) {
		case INSTR_ASSIGN:
			printf("    %s %s = ", ir[i].d_type == DTYPE_INT ? "INT" : "BOOL", ir[i].dest_name);
			if (ir[i].op1.kind == OPERAND_LITERAL) {
				printf("%d", ir[i].op1.val);
			} else {
				printf("%s", ir[i].op1.name);
			}
			printf("\n");
			break;
		case INSTR_UN:
		case INSTR_BIN:
			printf("    %s %s = %s %s ", ir[i].d_type == DTYPE_INT ? "INT" : "BOOL", ir[i].dest_name,
				ir[i].op1.d_type == DTYPE_INT ? "INT" : "BOOL", ir[i].op);
			if (ir[i].op1.kind == OPERAND_LITERAL) {
				printf("%d", ir[i].op1.val);
			} else {
				printf("%s", ir[i].op1.name);
			}
			if (ir[i].i_type == INSTR_BIN) {
				printf(", ");
				if (ir[i].op2.kind == OPERAND_LITERAL) {
					printf("%d", ir[i].op2.val);
				} else {
					printf("%s", ir[i].op2.name);
				}
			}
			printf("\n");
			break;
		
		case INSTR_JMP:
			printf("    %s %s %s, ", "JMP", ir[i].op, ir[i].dest_name);
			if (ir[i].op1.kind == OPERAND_LITERAL) {
				printf("%d", ir[i].op1.val);
			} else {
				printf("%s", ir[i].op1.name);
			}
			printf("\n");
			break;
		
		case INSTR_LABEL:
			printf("%s:\n", ir[i].dest_name);
			break;
		
		
		case INSTR_FUNC_START:
			printf("%s FUNC %s\n", ir[i].d_type == DTYPE_INT ? "INT" : "VOID", ir[i].dest_name);
			break;
		
		case INSTR_FUNC_END:
		case INSTR_EOF:
			break;
		
		case INSTR_ERR:
		default:
			printf("UNKNOWN\n");
			return;
		}
	}
}
