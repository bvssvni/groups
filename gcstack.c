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

#include "gcstack.h"

gcstack* gcstack_Alloc()
{
	return malloc(sizeof(gcstack));
}

gcstack* gcstack_Init(gcstack* gc)
{
    _err(gc == NULL);
    
	gc->length = 0;
	gc->root = malloc(sizeof(gcstack_item));
	gc->root->next = NULL;
	gc->root->previous = NULL;
	gc->root->freeSubPointers = NULL;
	return gc;
}

void gcstack_Delete(gcstack* gc)
{
    _err(gc == NULL);
    
	gcstack_End(gc, NULL);
	if (gc->root != NULL) {
		free(gc->root);
		gc->root = NULL;
	}
}

void gcstack_ReverseWithLevel(gcstack* gc, int level) {
    _err(gc == NULL); _err(level < 0);
    
    gcstack_item* cursor = gc->root->next;
    gcstack_item* start = cursor;
    gcstack_item* last = NULL;
    int len = gc->length;
    gcstack_item* next = NULL;
    
    for (int i = len; i > level; i--) {
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

void gcstack_ReverseToOtherStackWithLevel(gcstack* from, gcstack* to, int level) {
    _err(from == NULL); _err(to == NULL); _err(level < 0);
    
    gcstack_item* cursor = from->root->next;
    gcstack_item* next;
    int length = from->length;
    
    // Push the item onto the other stack until reaching level.
    for (int i = length; i > level; i--) {
        next = cursor->next;
        
        gcstack_Pop(from, cursor);
        gcstack_Push(to, cursor);
        cursor = next;
    }
}

gcstack_item** gcstack_CreateItemsArray(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	gcstack_item** arr = malloc(gc->length*sizeof(void*));
	for (int i = 0; i < length; i++)
	{
		arr[i] = (void*)cursor;
		cursor = cursor->next;
	}
	return arr;
}

int* gcstack_CreateIntArray(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	int* arr = malloc(gc->length*sizeof(int));
    gcint* item;
	for (int i = 0; i < length; i++)
	{
		item = (gcint*)cursor;
        arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

double* gcstack_CreateDoubleArray(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	double* arr = malloc(gc->length*sizeof(double));
    gcdouble* item;
	for (int i = 0; i < length; i++)
	{
		item = (gcdouble*)cursor;
        arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

bool* gcstack_CreateBoolArray(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	bool* arr = malloc(gc->length*sizeof(bool));
    gcbool* item;
	for (int i = 0; i < length; i++)
	{
		item = (gcbool*)cursor;
        arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

string* gcstack_CreateStringArray(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	string* arr = malloc(gc->length*sizeof(string));
    gcstring* item;
	for (int i = 0; i < length; i++)
	{
		item = (gcstring*)cursor;
        arr[i] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

gcstack_item** gcstack_CreateItemsArrayBackward(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	gcstack_item** arr = malloc(gc->length*sizeof(void*));
	for (int i = 0; i < length; i++)
	{
		arr[length-i-1] = (void*)cursor;
		cursor = cursor->next;
	}
	return arr;
}

int* gcstack_CreateIntArrayBackward(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	int* arr = malloc(gc->length*sizeof(int));
    gcint* item;
	for (int i = 0; i < length; i++)
	{
        item = (gcint*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

double* gcstack_CreateDoubleArrayBackward(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	double* arr = malloc(gc->length*sizeof(double));
    gcdouble* item;
	for (int i = 0; i < length; i++)
	{
        item = (gcdouble*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

string* gcstack_CreateStringArrayBackward(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	string* arr = malloc(gc->length*sizeof(string));
    gcstring* item;
	for (int i = 0; i < length; i++)
	{
        item = (gcstring*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

bool* gcstack_CreateBoolArrayBackward(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* cursor = gc->root->next;
	bool* arr = malloc(gc->length*sizeof(bool));
    gcbool* item;
	for (int i = 0; i < length; i++)
	{
        item = (gcbool*)cursor;
		arr[length-i-1] = item->val;
		cursor = cursor->next;
	}
	return arr;
}

void gcstack_Print(gcstack const* gc, void(*print)(void*a))
{
    _err(gc == NULL); _err(print == NULL);
    
	int length = gc->length;
	gcstack_item* item = gc->root->next;
	for (int i = 0; i < length; i++)
	{
		print(item);
		
		if (item->next == NULL) break;
		item = item->next;
	}
}

void gcstack_PrintInt(gcstack const* gc)
{
    _err(gc == NULL);
    
	int length = gc->length;
	gcstack_item* item = gc->root->next;
    gcint* intItem;
	for (int i = 0; i < length; i++)
	{
        intItem = (gcint*)item;
		printf("%i ", intItem->val);
		
		// print(cursor);
		if (item->next == NULL) break;
		item = item->next;
	}
    printf("\r\n");
}

gcstack_item* gcstack_Start(gcstack const* gc)
{
    _err(gc == NULL);
    
	return gc->root->next;
}

void gcstack_EndLevel(gcstack* gc, int level)
{
    _err(gc == NULL); _err(level < 0);
    
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

void gcstack_End(gcstack* gc, gcstack_item* end)
{
    _err(gc == NULL);
    
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

void gcstack_free(gcstack* gc, void* p)
{
    _err(p == NULL);
    
	if (gc != NULL) {
		gcstack_Pop(gc, p);
	}
    gcstack_item* item = (gcstack_item*)p;
    if (item->freeSubPointers != NULL)
    {
        item->freeSubPointers(item);
	}
    free(item);
}

gcstack_item* gcstack_malloc(gcstack* gc, int size, void(*freeSubPointers)(void* p))
{
    _err(size < 0);
    
	gcstack_item* item = malloc(size);
    
	item->freeSubPointers = freeSubPointers;
	
	if (gc == NULL)
	{
		// Set next and previous to NULL since it is outside any list.
		item->next = NULL;
		item->previous = NULL;
		return item;
	}
		
	gcstack_item* root = gc->root;
	gcstack_item* rootNext = root->next;
	
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

void gcstack_Swap(void* aPtr, void* bPtr)
{
    _err(aPtr == NULL); _err(bPtr == NULL);
    
	gcstack_item* a = (gcstack_item*)aPtr;
	gcstack_item* b = (gcstack_item*)bPtr;
	gcstack_item* prevA = a->previous;
	gcstack_item* nextA = a->next;
	gcstack_item* prevB = b->previous;
	gcstack_item* nextB = b->next;
	
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

void gcstack_Pop(gcstack* gc, void* p)
{
    _err(gc == NULL); _err(p == NULL);
    
	gcstack_item* item = (gcstack_item*)p;
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

void gcstack_Push(gcstack* gc, void* p)
{
    _err(gc == NULL); _err(p == NULL);
    
	gcstack_item* item = (gcstack_item*)p;
	
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

gcstack_item* gcstack_PushDouble(gcstack* gc, double val)
{
    _err(gc == NULL);
    
    gcdouble* d = (gcdouble*)gcstack_malloc(gc, sizeof(gcdouble), NULL);
    d->val = val;
    return (gcstack_item*)d;
}

gcstack_item* gcstack_PushInt(gcstack* gc, int val)
{
    _err(gc == NULL);
    
    gcint* d = (gcint*)gcstack_malloc(gc, sizeof(gcint), NULL);
    d->val = val;
    return (gcstack_item*)d;
}

gcstack_item* gcstack_PushBool(gcstack* gc, bool val)
{
    _err(gc == NULL);
    
    gcbool* d = (gcbool*)gcstack_malloc(gc, sizeof(gcbool), NULL);
    d->val = val;
    return (gcstack_item*)d;
}

void gcstring_Delete(void* p)
{
    _err(p == NULL);
    
    gcstring* d = (gcstring*)p;
    free(d->val);
    d->val = NULL;
}

gcstack_item* gcstack_PushString(gcstack* gc, string val)
{
    _err(gc == NULL); _err(val == NULL);
    
    gcstring* d = (gcstring*)gcstack_malloc(gc, sizeof(gcstring), gcstring_Delete);
    d->val = malloc(strlen(val)*sizeof(char));
    strcpy(d->val, val);
    return (gcstack_item*)d;
}

double gcstack_PopDouble(gcstack* gc)
{
    _err(gc == NULL);
    
    gcstack_item* item = gc->root->next;
    if (item == NULL) return 0.0;
    
    gcstack_Pop(gc, item);
    gcdouble* d = (gcdouble*)item;
    double val = d->val;
    free(d);
    return val;
}

int gcstack_PopInt(gcstack* gc)
{
    _err(gc == NULL);
    
    gcstack_item* item = gc->root->next;
    if (item == NULL) return -1;
    
    gcstack_Pop(gc, item);
    gcint* d = (gcint*)item;
    int val = d->val;
    free(d);
    return val;
}

bool gcstack_PopBool(gcstack* gc)
{
    _err(gc == NULL);
    
    gcstack_item* item = gc->root->next;
    if (item == NULL) return false;
    
    gcstack_Pop(gc, item);
    gcbool* d = (gcbool*)item;
    bool val = d->val;
    free(d);
    return val;
}

char* gcstack_PopString(gcstack* gc)
{
    _err(gc == NULL);
    
    gcstack_item* item = gc->root->next;
    if (item == NULL) return NULL;
    
    // Since we will need a pointer to return
    // we can reuse the string instead of copying it.
    // Therefore there is no call to 'gcstring_Delete'.
    gcstack_Pop(gc, item);
    gcstring* d = (gcstring*)item;
    char* val = d->val;
    free(d);
    return val;
}

double gcstack_PopDoubleWithItem(gcstack* gc, gcstack_item* item)
{
    _err(gc == NULL);
    
    if (item == NULL) return 0.0;
    
    gcstack_Pop(gc, item);
    gcdouble* d = (gcdouble*)item;
    double val = d->val;
    free(d);
    return val;
}

int gcstack_PopIntWithItem(gcstack* gc, gcstack_item* item)
{
    _err(gc == NULL);
    
    if (item == NULL) return -1;
    
    gcstack_Pop(gc, item);
    gcint* d = (gcint*)item;
    int val = d->val;
    free(d);
    return val;
}


bool gcstack_PopBoolWithItem(gcstack* gc, gcstack_item* item)
{
    _err(gc == NULL);
    
    if (item == NULL) return false;
    
    gcstack_Pop(gc, item);
    gcbool* d = (gcbool*)item;
    bool val = d->val;
    free(d);
    return val;
}

char* gcstack_PopStringWithItem(gcstack* gc, gcstack_item* item)
{
    _err(gc == NULL);
    
    if (item == NULL) return NULL;
    
    // Since we will need a pointer to return
    // we can reuse the string instead of copying it.
    // Therefore there is no call to 'gcstring_Delete'.
    gcstack_Pop(gc, item);
    gcstring* d = (gcstring*)item;
    char* val = d->val;
    free(d);
    return val;
}

