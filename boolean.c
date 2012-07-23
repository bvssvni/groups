//
//  boolean.c
//  MemGroups
//
//  Created by Sven Nilsen on 29.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "gcstack.h"
#include "hashtable.h"
#include "bitstream.h"
#include "groups.h"

#include "parsing.h"

#include "errorhandling.h"
#include "readability.h"

#include "boolean.h"

void boolean_eval_BinaryOp(gcstack* st);

void boolean_eval_BinaryOp(gcstack* const st)
{
	const gcint* const opItem = (gcint*)st->root->next->next;
	const char op = (char)opItem->val;
	
	// Pop arguments and operator from stack.
	bitstream* const arg2 = (bitstream*)st->root->next;
	bitstream* const arg1 = (bitstream*)st->root->next->next->next;
	
	switch (op) {
		case '*':
			bitstream_GcAnd(st, arg1, arg2);
			break;
		case '+':
			bitstream_GcOr(st, arg1, arg2);
			break;
		case '-':
			bitstream_GcExcept(st, arg1, arg2);
			break;
	}
	
	gcstack_Pop(st, (gcstack_item*)arg1);
	gcstack_Pop(st, (gcstack_item*)arg2);
	bitstream_Delete(arg1);
	bitstream_Delete(arg2);
	free(arg1);
	free(arg2);
	
	// Release the operator since the two values are going to be replaced.
	gcstack_free(st, (gcstack_item*)opItem);
}


typedef struct data
{
	gcstack* const st;
	const char* errorMessage;
	int delta;
	int stateIndex;
	groups* const g;
} data;

void boolean_eval_CheckPrecedence(data* data, const int repeatIndex);
void boolean_eval_CheckPrecedence(data* data, const int repeatIndex)
{
	// We can access the operators directly on the stack.
	char op2 = (char)((gcint*)(data->st->root->next))->val;
	char op1 = (char)((gcint*)(data->st->root->next->next->next))->val;
	
	// The ascii table is sorted by negative precedence.
	// * < + < -
	if (op2 >= op1) {
		char op = (char)gcstack_PopInt(data->st);
		boolean_eval_BinaryOp(data->st);
		gcstack_PushInt(data->st, op);
	}
	
	data->stateIndex = repeatIndex;
}

void boolean_eval_ReadOperator
(data *data, const int pos, const char* const expr, const char* const ops);

void boolean_eval_ReadOperator
(data *data, const int pos, const char* const expr, const char* const ops)
{
	char op = parsing_ReadOneCharacterOf
	(expr+pos, ops, &data->delta);
	
	if (data->delta == 0) {
		data->errorMessage = "Expected operator";
		return;
	}
	
	gcstack_PushInt(data->st, op);
}

void boolean_eval_ReadVariable
(data *data, const int pos, const char* const expr, const char* const ops);

void boolean_eval_ReadVariable
(data *data, const int pos, const char* const expr, const char* const ops)
{
	char* const variableName = parsing_ReadVariableName
	(expr+pos, ops, &data->delta);
	
	// Check that a variable name was read.
	if (data->delta == 0) {
		data->errorMessage = "Expected variable";
		return;
	}
	
	int propId = groups_get_property(data->g, variableName);
	
	if (propId == -1) {
		data->errorMessage = "Unknown property";
		return;
	}
	
	groups_gc_get_bitstream(data->st, data->g, propId);
	
	free(variableName);
}

bitstream* boolean_GcEval
(gcstack* const gc, groups* const g, const char* const expr, 
 void (* const err)(int pos, const char* message))
{
	macro_err(g == NULL); macro_err(expr == NULL);
	
	const char* const flow = " v o v op";
	const char* const valid_exits = "vp";
	
	data data = {.stateIndex = 0,
		.st = gcstack_Init(gcstack_Alloc()),
		.errorMessage = NULL,
		.delta = 0,
		.g = g
	};
	
	int repeatIndex = 3;
	
	enum {
		read_white_space = ' ',
		read_variable = 'v',
		read_operator = 'o',
		precedence_check = 'p',
		error = 'r'
	};
	
	size_t exprLength = strlen(expr);
	int pos = 0;
	const char* ops = "*+-";
	
NEW_STATE:
	switch (flow[data.stateIndex]) {
		case read_white_space:
			data.delta = parsing_SkipWhiteSpace(expr+pos);
			break;
		case read_variable:
			boolean_eval_ReadVariable
			(&data, pos, expr, ops);
			break;
		case read_operator:
			boolean_eval_ReadOperator
			(&data, pos, expr, ops);
			break;
		case precedence_check:
			boolean_eval_CheckPrecedence(&data, repeatIndex);
			break;
	}
	
	if (data.errorMessage != NULL) {
		if (err != NULL)
			err(pos, data.errorMessage);
		else
			macro_errExp(data.errorMessage, pos, expr);
		goto CLEAN_UP;
	}
	
	pos += data.delta;
	data.delta = 0;
	
	if (pos >= exprLength)
		goto CLEAN_UP;
	
	data.stateIndex++;
	goto NEW_STATE;
	
CLEAN_UP:
	// Check for incomplete expression. This can be done by checking
	// which state the flow was in before breaking cycle.
	if (strchr(valid_exits, flow[data.stateIndex]) == NULL) {
		data.errorMessage = "Incomplete expression";
		if (err != NULL)
			err(pos, data.errorMessage);
		else
			macro_errExp(data.errorMessage, pos, expr);
		
		gcstack_Delete(data.st);
		free(data.st);
		
		return NULL;
	}
	
	// Evaluate all operators.
	while (data.st->length > 1) {
		boolean_eval_BinaryOp(data.st);
	}
	
	bitstream* b = (bitstream*)data.st->root->next;
	if (gc != NULL)
		gcstack_Push(gc, (gcstack_item*)b);
	else
		gcstack_Pop(data.st, (gcstack_item*)b);
	
	gcstack_Delete(data.st);
	free(data.st);
	
	return b;
}
