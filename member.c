/*
 *  member.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "variable.h"
#include "sorting.h"

#include "member.h"

void member_Delete(void* p)
{
	member* obj = (member*)p;
	
	// Free variable stack.
	if (obj->variables != NULL)
	{
		gcstack_Delete(obj->variables);
		free(obj->variables);
		obj->variables = NULL;
	}
	
	// Free variable array.
	if (obj->m_variableArray != NULL)
	{
		free(obj->m_variableArray);
		obj->m_variableArray = NULL;
	}
}

member* member_AllocWithGC(gcstack* gc)
{
	return (member*)gcstack_malloc(gc, sizeof(member), member_Delete);
}

member* member_Init(member* obj)
{
	obj->variables = gcstack_Init(gcstack_Alloc());
	obj->m_ready = false;
	obj->m_variableArray = NULL;
	return obj;
}


void member_AddDouble(member* obj, int propId, double val)
{
	variable_InitWithDouble(variable_AllocWithGC(obj->variables), propId, val);
	obj->m_ready = false;
}

void member_AddString(member* obj, int propId, char const* val)
{
	variable_InitWithString(variable_AllocWithGC(obj->variables), propId, val);
	obj->m_ready = false;
}

void member_AddInt(member* obj, int propId, int val)
{
	variable_InitWithInt(variable_AllocWithGC(obj->variables), propId, val);
	obj->m_ready = false;
}

int comparePropertyId(const void* aPtr, const void* bPtr)
{
	int* a = (int*)aPtr;
	variable* b = (variable*)bPtr;
	int aId = *a;
	int bId = b->propId;
	return aId < bId ? -1 : aId > bId ? 1 : 0;
}

void sortVariables(member* obj)
{
	if (obj->m_ready) return;
	
	int length = obj->variables->length;
	variable** vars = (variable**)gcstack_CreateItemsArray(obj->variables);
	void* t = malloc(sizeof(variable));
	sorting_Sort((void*)vars, 0, length, sizeof(variable), t, comparePropertyId);
	free(t);
	
	if (obj->m_variableArray != NULL)
		free(obj->m_variableArray);
	obj->m_variableArray = vars;
	
	obj->m_ready = true;
}

int member_IndexOf(member* obj, int propId)
{
	sortVariables(obj);
	
	int length = obj->variables->length;
	variable** vars = obj->m_variableArray;
	return sorting_SearchBinary(length, (void*)vars, &propId, comparePropertyId);
}

