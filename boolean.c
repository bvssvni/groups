//
//  boolean.c
//  MemGroups
//
//  Created by Sven Nilsen on 29.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "memgroups.h"
#include "parsing.h"

#include "errorhandling.h"
#include "readability.h"

void boolean_eval_BinaryOp(gcstack* st);

void boolean_eval_BinaryOp(gcstack* const st)
{
	const gcint* const opItem = (gcint*)st->root->next->next;
	const char op = opItem->val;
	
	// Pop arguments and operator from stack.
	bitstream* const arg2 = (bitstream*)st->root->next;
	bitstream* const arg1 = (bitstream*)st->root->next->next->next;
	
	switch (op) {
		case '*':
			bitstream_And(st, arg1, arg2);
			break;
		case '+':
			bitstream_Or(st, arg1, arg2);
			break;
		case '-':
			bitstream_Except(st, arg1, arg2);
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

bitstream* boolean_Eval
(gcstack* const gc, groups* const g, const char* const expr, 
 void (* const err)(int pos, const char* message))
{
	macro_err(g == NULL); macro_err(expr == NULL);
	
	string state = "eveoeveop";
	int stateIndex = 0;
	int repeatIndex = 1;
	
	const char read_white_space = 'e';
	const char read_variable = 'v';
	const char read_operator = 'o';
	const char precedence_check = 'p';
	const char error = 'r';
	
	gcstack* st = gcstack_Init(gcstack_Alloc());
	
	int exprLength = strlen(expr);
	int pos = 0;
	int delta = 0;
	char op;
	const char* variableName = NULL;
	const char* errorMessage = NULL;
	int propId = 0;
	char op1;
	char op2;
	
NEW_STATE:
	switch (state[stateIndex]) {
		case read_white_space:
			delta = parsing_SkipWhiteSpace(expr+pos);
			break;
		case read_variable:
			variableName = parsing_ReadVariableName
			(expr+pos, "", &delta);
			
			// Check that a variable name was read.
			if (delta == 0) {
				errorMessage = "Expected variable";
				stateIndex = error;
				goto NEW_STATE;
			}
			
			propId = groups_GetProperty(g, variableName);
			
			if (propId == -1) {
				errorMessage = "Unknown property";
				stateIndex = error;
				goto NEW_STATE;
			}
			
			groups_GetBitstream(st, g, propId);
			break;
		case read_operator:
			op = parsing_ReadOneCharacterOf
			(expr+pos, "*+-", &delta);
			
			if (delta == 0) {
				errorMessage = "Expected operator";
				stateIndex = error;
				goto NEW_STATE;
			}
			
			gcstack_PushInt(st, op);
			break;
		case precedence_check:
			// We can access the operators directly on the stack.
			op2 = ((gcint*)(st->root->next))->val;
			op1 = ((gcint*)(st->root->next->next->next))->val;
			
			// The ascii table is sorted by negative precedence.
			// * < + < -
			if (op2 >= op1) {
				op = gcstack_PopInt(st);
				boolean_eval_BinaryOp(st);
				gcstack_PushInt(st, op);
			}
			
			stateIndex = repeatIndex;
			break;
		case error:
			if (err != NULL)
				err(pos, errorMessage);
			else
				macro_errExp(errorMessage, pos, expr);
			goto CLEAN_UP;
			break;
	}
	
	pos += delta;
	
	if (pos >= exprLength)
		goto CLEAN_UP;
	
	stateIndex++;
	goto NEW_STATE;
	
CLEAN_UP:
	// Evaluate all operators.
	while (st->length > 1) {
		boolean_eval_BinaryOp(st);
	}
	
	bitstream* b = (bitstream*)st->root->next;
	if (gc != NULL)
		gcstack_Push(gc, (gcstack_item*)b);
	else
		gcstack_Pop(st, (gcstack_item*)b);
	
	gcstack_Delete(st);
	free(st);
	
	return b;
}

void boolean_RunUnitTests(void)
{
	printf("Boolean unit tests - ");
	
	{
		gcstack* gc = gcstack_Init(gcstack_Alloc());
		groups* g = groups_Init(groups_AllocWithGC(gc));
		int propName = groups_AddProperty(g, "Name", "string");
		hash_table* mem = hashTable_Init(hashTable_AllocWithGC(gc));
		hashTable_SetString(mem, propName, "Andrew");
		groups_AddMember(g, mem);
		bitstream* b = boolean_Eval(gc, g, "Name", NULL);
		macro_test_int(b->length, 2);
		macro_test_int(b->pointer[0], 0);
		macro_test_int(b->pointer[1], 1);
		gcstack_Delete(gc);
		free(gc);
	}
	
	{
		gcstack* gc = gcstack_Init(gcstack_Alloc());
		groups* g = groups_Init(groups_AllocWithGC(gc));
		int propName = groups_AddProperty(g, "Name", "string");
		int propLastName = groups_AddProperty(g, "LastName", "string");
		hash_table* mem = hashTable_Init(hashTable_AllocWithGC(gc));
		hashTable_SetString(mem, propName, "Andrew");
		hashTable_SetString(mem, propLastName, "Snow");
		groups_AddMember(g, mem);
		bitstream* b = boolean_Eval(gc, g, "Name * LastName", NULL);
		macro_test_int(b->length, 2);
		macro_test_int(b->pointer[0], 0);
		macro_test_int(b->pointer[1], 1);
		gcstack_Delete(gc);
		free(gc);
	}
	
	{
		gcstack* gc = gcstack_Init(gcstack_Alloc());
		groups* g = groups_Init(groups_AllocWithGC(gc));
		int propName = groups_AddProperty(g, "Name", "string");
		groups_AddProperty(g, "LastName", "string");
		hash_table* mem = hashTable_Init(hashTable_AllocWithGC(gc));
		hashTable_SetString(mem, propName, "Andrew");
		groups_AddMember(g, mem);
		bitstream* b = boolean_Eval(gc, g, "LastName * Name", NULL);
		macro_test_int(b->length, 0);
		gcstack_Delete(gc);
		free(gc);
	}
	
	{
		gcstack* gc = gcstack_Init(gcstack_Alloc());
		groups* g = groups_Init(groups_AllocWithGC(gc));
		int propName = groups_AddProperty(g, "Name", "string");
		groups_AddProperty(g, "LastName", "string");
		hash_table* mem = hashTable_Init(hashTable_AllocWithGC(gc));
		hashTable_SetString(mem, propName, "Andrew");
		groups_AddMember(g, mem);
		bitstream* b = boolean_Eval(gc, g, "Name - LastName", NULL);
		macro_test_int(b->length, 2);
		macro_test_int(b->pointer[0], 0);
		macro_test_int(b->pointer[1], 1);
		gcstack_Delete(gc);
		free(gc);
	}
	
	printf("OK\r\n");
}
