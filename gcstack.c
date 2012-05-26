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

#include "gcstack.h"

gcstack* gcstack_Alloc()
{
	return malloc(sizeof(gcstack));
}

gcstack* gcstack_Init(gcstack* gc)
{
	gc->length = 0;
	gc->root = malloc(sizeof(gcstack_item));
	gc->root->next = NULL;
	gc->root->previous = NULL;
	gc->root->free = NULL;
	return gc;
}

void gcstack_Delete(gcstack* gc)
{
	gcstack_End(gc, NULL);
	if (gc->root != NULL) {
		free(gc->root);
		gc->root = NULL;
	}
}


gcstack_item** gcstack_CreateItemsArray(gcstack const* gc)
{
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

gcstack_item** gcstack_CreateItemsArrayBackward(gcstack const* gc)
{
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


void gcstack_Print(gcstack const* gc, void(*print)(void*a))
{
	int length = gc->length;
	gcstack_item* item = gc->root->next;
	for (int i = 0; i < length; i++)
	{
		print(item);
		
		// print(cursor);
		if (item->next == NULL) break;
		item = item->next;
	}
}

gcstack_item* gcstack_Start(gcstack const* gc)
{
	return gc->root->next;
}

void gcstack_End(gcstack* gc, gcstack_item* end)
{
	//
	if (gc->root == NULL)
	{
		printf("gc->root == NULL");
		exit(1);
	}
	//*/
	
	/*/
	if (remove < 0) {
		printf("gcstack_End: remove < 0");
		exit(1);
	}
	//*/
	
	gcstack_item* cursor = gc->root->next;
	gcstack_item* next;
	while (cursor != end) {
		next = cursor->next;
		
		// Ignore if previous is set to null.
		if (cursor->previous != NULL)
		{
			if (cursor->free != NULL)
			{
				cursor->free(cursor);
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
	if (gc != NULL) {
		gcstack_Pop(gc, p);
	}
	gcstack_item* item = (gcstack_item*)p;
	item->free(item);
	free(item);
}

gcstack_item* gcstack_malloc(gcstack* gc, int size, void(*free)(void* p))
{
	gcstack_item* item = malloc(size);
	item->free = free;
	
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




