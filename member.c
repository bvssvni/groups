/*
 *  member.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

/*
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met: 
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer. 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution. 
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies, 
 either expressed or implied, of the FreeBSD Project.
 */

#include <stdio.h>
#include <stdlib.h>

#include "variable.h"
#include "sorting.h"

#include "readability.h"

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
	
	obj->m_ready = false;
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

member* member_InitWithMember(member* obj, member* b)
{
	obj->variables = b->variables;
	obj->m_ready = b->m_ready;
	obj->m_variableArray = b->m_variableArray;
	
	b->variables = NULL;
	b->m_ready = false;
	b->m_variableArray = NULL;
	
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

void member_AddBool(member* obj, int propId, bool val)
{
	variable_InitWithBool(variable_AllocWithGC(obj->variables), propId, val);
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

