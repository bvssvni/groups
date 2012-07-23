/*
 *  gcstack.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 19.05.12.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "errorhandling.h"
#include "readability.h"

#define memgroups_gcstack_internal
#include "gcstack.h"

// This structure is used when pushing pointers to the stack that does
// not have "gcstack_item gc;" declared at beginning of struct.
typedef struct gcpointer {
	gcstack_item gc;
	void* pointer;
	void (*cleanUp)(void* const p);
} gcpointer;

gcstack* gcstack_Alloc()
{
	return malloc(sizeof(gcstack));
}

gcstack* gcstack_Init(gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	gc->length = 0;
	gc->root = malloc(sizeof(gcstack_item));
	gc->root->next = NULL;
	gc->root->previous = NULL;
	gc->root->freeSubPointers = NULL;
	return gc;
}

void gcstack_Delete(void* const p)
{
	macro_err_return(p == NULL);
	
	gcstack* const gc = (gcstack*)p;
	
	gcstack_End(gc, NULL);
	if (gc->root != NULL) {
		free(gc->root);
		gc->root = NULL;
	}
}

void gcstack_ReverseWithLevel(gcstack* const gc, const int level) {
	macro_err_return(gc == NULL);
	macro_err_return(level < 0);
	
	const int len = gc->length;
	
	gcstack_item* cursor = gc->root->next;
	gcstack_item* start = cursor;
	gcstack_item* last = NULL;
	gcstack_item* next = NULL;
	
	int i;
	for (i = len; i > level; i--) {
		// Swap previous and next for each item.
		next = cursor->next;
		cursor->next = cursor->previous;
		cursor->previous = next;
		last = cursor;
		cursor = next;
	}
	
	if (last != NULL)
		last->previous = gc->root;
	gc->root->next = last;
	
	if (start != NULL)
		start->next = next;
	if (next != NULL)
		next->previous = start;
}

void gcstack_ReverseToOtherStackWithLevel
(gcstack* const from, gcstack* const to, const int level) 
{
	macro_err_return(from == NULL);
	macro_err_return(to == NULL);
	macro_err_return(level < 0);
	
	const int length = from->length;
	
	gcstack_item* cursor = from->root->next;
	gcstack_item* next;
	
	// Push the item onto the other stack until reaching level.
	int i;
	for (i = length; i > level; i--) {
		next = cursor->next;
		
		gcstack_Pop(from, cursor);
		gcstack_Push(to, cursor);
		cursor = next;
	}
}

gcstack_item** gcstack_CreateItemsArray(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	gcstack_item** const arr = malloc(gc->length*sizeof(void*));
	
	gcstack_item* cursor = gc->root->next;
	int i;
	for (i = 0; i < length; i++)
	{
		arr[i] = (void*)cursor;
		cursor = cursor->next;
	}
	return arr;
}

int* gcstack_CreateIntArray(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	int* const arr = malloc(gc->length*sizeof(int));
	
	gcstack_item* cursor = gc->root->next;
	gcint* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcint*)cursor;
		arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

double* gcstack_CreateDoubleArray(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	double* const arr = malloc(gc->length*sizeof(double));
	
	gcstack_item* cursor = gc->root->next;
	gcdouble* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcdouble*)cursor;
		arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

bool* gcstack_CreateBoolArray(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	bool* const arr = malloc(gc->length*sizeof(bool));
	
	gcstack_item* cursor = gc->root->next;
	gcbool* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcbool*)cursor;
		arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

string* gcstack_CreateStringArray(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	string* const arr = malloc(gc->length*sizeof(string));
	
	gcstack_item* cursor = gc->root->next;
	gcstring* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcstring*)cursor;
		arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

gcstack_item** gcstack_CreateItemsArrayBackward(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	gcstack_item** const arr = malloc(length*sizeof(void*));
	
	gcstack_item* cursor = gc->root->next;
	int i;
	for (i = 0; i < length; i++)
	{
		arr[length-i-1] = (void*)cursor;
		cursor = cursor->next;
	}
	return arr;
}

int* gcstack_CreateIntArrayBackward(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	int* const arr = malloc(gc->length*sizeof(int));
	
	gcstack_item* cursor = gc->root->next;
	gcint* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcint*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

double* gcstack_CreateDoubleArrayBackward(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	double* const arr = malloc(length*sizeof(double));
	
	gcstack_item* cursor = gc->root->next;
	gcdouble* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcdouble*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

string* gcstack_CreateStringArrayBackward(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	string* const arr = malloc(length*sizeof(string));
	
	gcstack_item* cursor = gc->root->next;
	gcstring* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcstring*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

bool* gcstack_CreateBoolArrayBackward(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	const int length = gc->length;
	bool* const arr = malloc(gc->length*sizeof(bool));
	
	gcstack_item* cursor = gc->root->next;
	gcbool* item;
	int i;
	for (i = 0; i < length; i++)
	{
		item = (gcbool*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

void gcstack_Print(const gcstack* const gc, void(* const print)(void*a))
{
	macro_err_return(gc == NULL);
	macro_err_return(print == NULL);
	
	const int length = gc->length;
	gcstack_item* item = gc->root->next;
	int i;
	for (i = 0; i < length; i++)
	{
		print(item);
		
		if (item->next == NULL) 
			break;
		
		item = item->next;
	}
}

void gcstack_PrintInt(const gcstack* const gc)
{
	macro_err_return(gc == NULL);
	
	const int length = gc->length;
	gcstack_item* item = gc->root->next;
	gcint* intItem;
	int i;
	for (i = 0; i < length; i++)
	{
		intItem = (gcint*)item;
		printf("%i ", intItem->val);
		
		// print(cursor);
		if (item->next == NULL) break;
		item = item->next;
	}
	printf("\r\n");
}

void gcstack_PrintString(const gcstack* const gc)
{
	macro_err_return(gc == NULL);
	
	const int length = gc->length;
	gcstack_item* item = gc->root->next;
	gcstring* stringItem;
	int i;
	for (i = 0; i < length; i++)
	{
		stringItem = (gcstring*)item;
		printf("%s ", stringItem->val);
		
		// print(cursor);
		if (item->next == NULL) break;
		item = item->next;
	}
	printf("\r\n");
}

const gcstack_item* gcstack_Start(const gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	return gc->root->next;
}

void gcstack_EndLevel(gcstack* const gc, const int level)
{
	macro_err_return(gc == NULL);
	macro_err_return(level < 0);
	
	gcstack_item* cursor = gc->root->next;
	gcstack_item* next;
	while (cursor != NULL && gc->length > level) {
		next = cursor->next;
		
		// Ignore if previous is set to null.
		if (cursor->previous != NULL)
		{
			if (cursor->freeSubPointers != NULL)
			{
				cursor->freeSubPointers(cursor);
			}
			
			free(cursor);
		}
		cursor = next;
		
		gc->length--;
	}
	gc->root->next = cursor;
}

void gcstack_End(gcstack* const gc, const gcstack_item* const end)
{
	macro_err_return(gc == NULL);
	
	gcstack_item* cursor = gc->root->next;
	gcstack_item* next;
	while (cursor != end) {
		next = cursor->next;
		
		// Ignore if previous is set to null.
		if (cursor->previous != NULL)
		{
			if (cursor->freeSubPointers != NULL)
			{
				cursor->freeSubPointers(cursor);
			}
			
			free(cursor);
		}
		cursor = next;
		
		gc->length--;
	}
	gc->root->next = cursor;
}

void gcstack_free(gcstack* const gc, gcstack_item* const item)
{
	macro_err_return(item == NULL);
	
	if (gc != NULL) {
		gcstack_Pop(gc, item);
	}
	if (item->freeSubPointers != NULL)
	{
		item->freeSubPointers(item);
	}
	free(item);
}

gcstack_item* gcstack_malloc
(gcstack* const gc, const int size, 
 void(* const freeSubPointers)(void* const p))
{
	macro_err(size < 0);
	
	gcstack_item* const item = malloc(size);
	
	// Reset all bits to 0.
	memset(item, 0, size);
	
	item->freeSubPointers = freeSubPointers;
	
	if (gc == NULL)
	{
		// Set next and previous to NULL since it is outside any list.
		item->next = NULL;
		item->previous = NULL;
		return item;
	}
	
	gcstack_item* const root = gc->root;
	gcstack_item* const rootNext = root->next;
	
	// Set previous.
	if (rootNext != NULL)
		rootNext->previous = item;
	item->previous = root;
	
	// Set the root to point to the item.
	root->next = item;
	item->next = rootNext;
	
	gc->length++;
	
	return item;
}

void gcstack_Swap(void* const aPtr, void* const bPtr)
{
	macro_err_return(aPtr == NULL);
	macro_err_return(bPtr == NULL);
	
	gcstack_item* const a = (gcstack_item*)aPtr;
	gcstack_item* const b = (gcstack_item*)bPtr;
	gcstack_item* const prevA = a->previous;
	gcstack_item* const nextA = a->next;
	gcstack_item* const prevB = b->previous;
	gcstack_item* const nextB = b->next;
	
	a->previous = prevB;
	a->next = nextB;
	
	b->previous = prevA;
	b->next = nextA;
	
	if (prevA != NULL) 
		prevA->next = b;
	if (nextA != NULL) 
		nextA->previous = b;
	
	if (prevB != NULL) 
		prevB->next = a;
	if (nextB != NULL) 
		nextB->previous = a;
}

void gcstack_Pop(gcstack* const gc, void* const p)
{
	macro_err_return(gc == NULL);
	macro_err_return(p == NULL);
	
	gcstack_item* const item = (gcstack_item*)p;
	// Detach from old stack.
	if (item->previous != NULL)
	{
		item->previous->next = item->next;
		if (item->next != NULL)
			item->next->previous = item->previous;
		item->next = NULL;
		item->previous = NULL;
	}
	gc->length--;
}

void gcstack_Push(gcstack* const gc, gcstack_item* const item)
{
	macro_err_return(gc == NULL);
	macro_err_return(item == NULL);
	
	// Detach from old stack.
	if (item->previous != NULL)
	{
		item->previous->next = item->next;
		if (item->next != NULL)
			item->next->previous = item->previous;
	}
	
	// Attach to new stack.
	if (gc->root->next != NULL)
		gc->root->next->previous = item;
	
	item->previous = gc->root;
	item->next = gc->root->next;
	
	gc->root->next = item;
	gc->length++;
}

void gcpointer_Delete(void* const p);

void gcpointer_Delete(void* const p) 
{
	macro_err_return(p == NULL);
	
	gcpointer* const gcp = (gcpointer*)p;
	
	// Clean up the internal structure of the pointer.
	if (gcp->cleanUp != NULL) {
		gcp->cleanUp(gcp->pointer);
	}
	
	// Free the pointer.
	if (gcp->pointer != NULL) {
		free(gcp->pointer);
		gcp->pointer = NULL;
	}
}

void gcstack_PushPointer
(gcstack* const gc, void* const p, void (* const cleanUp)(void* const p)) 
{
	macro_err(gc == NULL); macro_err(p == NULL);
	
	gcpointer* const gcp = (gcpointer*)gcstack_malloc
	(gc, sizeof(gcpointer), gcpointer_Delete);
	
	gcp->pointer = p;
	gcp->cleanUp = cleanUp;
}

gcstack_item* gcstack_PushDouble(gcstack* const gc, const double val)
{
	macro_err(gc == NULL);
	
	gcdouble* const d = (gcdouble*)gcstack_malloc
	(gc, sizeof(gcdouble), NULL);
	
	d->val = val;
	
	return (gcstack_item*)d;
}

gcstack_item* gcstack_PushInt(gcstack* const gc, const int val)
{
	macro_err(gc == NULL);
	
	gcint* const d = (gcint*)gcstack_malloc(gc, sizeof(gcint), NULL);
	d->val = val;
	
	return (gcstack_item*)d;
}

gcstack_item* gcstack_PushBool(gcstack* const gc, const bool val)
{
	macro_err(gc == NULL);
	
	gcbool* const d = (gcbool*)gcstack_malloc(gc, sizeof(gcbool), NULL);
	d->val = val;
	
	return (gcstack_item*)d;
}

void gcstring_Delete(void* const p);

void gcstring_Delete(void* const p)
{
	macro_err_return(p == NULL);
	
	gcstring* const d = (gcstring* const)p;
	free(d->val);
	
	d->val = NULL;
}

gcstack_item* gcstack_PushString(gcstack* const gc, const char* const val)
{
	macro_err(gc == NULL); macro_err(val == NULL);
	
	gcstring* const d = (gcstring*)gcstack_malloc
	(gc, sizeof(gcstring), gcstring_Delete);
	
	d->val = malloc(strlen(val)*sizeof(char));
	strcpy(d->val, val);
	return (gcstack_item*)d;
}

double gcstack_PopDouble(gcstack* const gc)
{
	macro_err_return_zero(gc == NULL);
	
	gcstack_item* const item = gc->root->next;
	if (item == NULL) 
		return 0.0;
	
	gcstack_Pop(gc, item);
	gcdouble* const d = (gcdouble*)item;
	const double val = d->val;
	free(d);
	
	return val;
}

int gcstack_PopInt(gcstack* const gc)
{
	macro_err_return_zero(gc == NULL);
	
	gcstack_item* const item = gc->root->next;
	if (item == NULL) 
		return -1;
	
	gcstack_Pop(gc, item);
	
	gcint* const d = (gcint*)item;
	const int val = d->val;
	free(d);
	
	return val;
}

bool gcstack_PopBool(gcstack* const gc)
{
	macro_err_return_zero(gc == NULL);
	
	gcstack_item* const item = gc->root->next;
	if (item == NULL) 
		return false;
	
	gcstack_Pop(gc, item);
	
	gcbool* const d = (gcbool*)item;
	const bool val = d->val;
	free(d);
	
	return val;
}

char* gcstack_PopString(gcstack* const gc)
{
	macro_err_return_null(gc == NULL);
	
	gcstack_item* const item = gc->root->next;
	if (item == NULL) 
		return NULL;
	
	// Since we will need a pointer to return
	// we can reuse the string instead of copying it.
	// Therefore there is no call to 'gcstring_Delete'.
	gcstack_Pop(gc, item);
	
	gcstring* const d = (gcstring*)item;
	char* const val = d->val;
	free(d);
	
	return val;
}

double gcstack_PopDoubleWithItem(gcstack* const gc, gcstack_item* const item)
{
	macro_err_return_zero(gc == NULL);
	macro_err_return_zero(item == NULL);
	
	gcstack_Pop(gc, item);
	
	gcdouble* const d = (gcdouble*)item;
	const double val = d->val;
	free(d);
	
	return val;
}

int gcstack_PopIntWithItem(gcstack* const gc, gcstack_item* const item)
{
	macro_err_return_zero(gc == NULL);
	macro_err_return_zero(item == NULL);
	
	gcstack_Pop(gc, item);
	
	gcint* const d = (gcint*)item;
	const int val = d->val;
	free(d);
	
	return val;
}


bool gcstack_PopBoolWithItem(gcstack* const gc, gcstack_item* const item)
{
	macro_err_return_zero(gc == NULL);
	macro_err_return_zero(item == NULL);
	
	gcstack_Pop(gc, item);
	
	gcbool* const d = (gcbool*)item;
	const bool val = d->val;
	
	free(d);
	
	return val;
}

const char* gcstack_PopStringWithItem
(gcstack* const gc, gcstack_item* const item)
{
	macro_err_return_null(gc == NULL);
	macro_err_return_null(item == NULL);
	
	// Since we will need a pointer to return
	// we can reuse the string instead of copying it.
	// Therefore there is no call to 'gcstring_Delete'.
	gcstack_Pop(gc, item);
	
	gcstring* const d = (gcstring*)item;
	char* const val = d->val;
	free(d);
	
	return val;
}

char* gcstack_PopIntsAsString(gcstack* const gc) {
	char* const str = malloc(sizeof(char)*(gc->length+1));
	gcstack_item* cursor = gc->root->next;
	int index = gc->length-1;
	
	for (; cursor != NULL; cursor = cursor->next) {
		str[index] = (char)((gcint*)cursor)->val;
		index--;
	}
	
	str[gc->length] = '\0';
	gcstack_End(gc, NULL);
	
	return str;
}


