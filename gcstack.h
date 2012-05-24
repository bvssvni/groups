/*
 *  gcstack.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 22.05.12.
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
 
#ifdef __cplusplus
extern "C" {
#endif

#ifndef memgroups_gcstack
#define memgroups_gcstack

// Items for garbage collection.
typedef struct gcstack_item gcstack_item;
struct gcstack_item
{
	gcstack_item* previous;
	gcstack_item* next;
	void(*free)(void* p);
};

// Garbage collector stack.
// This works as the container of a double-linked list.
// The root is a stat item on the top of the stack.
// The length is the depth of the stack according to counting.
typedef struct gcstack
{
	int length;
	gcstack_item* root;
} gcstack;

// This function is handy when you have one gc
// in an object that contains the members,
// you can swap one above the current level and one
// below so the old one gets deleted.
void gcstack_Swap
(void* a, void* b);

// Pops item from the stack, calls the free method and frees the pointer.
void gcstack_free
(gcstack* gc, void* p);

gcstack* gcstack_Alloc
();

// Deletes the gcstack and all the data it refers to.
void gcstack_Delete
(gcstack* gc);

// Initializes a garbage collected item.
gcstack_item* gcstack_malloc
(gcstack* gc, int size, void(*free)(void* p));

gcstack* gcstack_Init
(gcstack* gc);

int	gcstack_Start
(gcstack const* gc);

void gcstack_End
(gcstack* gc, int level);

void gcstack_Pop
(gcstack* gc, void* p);

void gcstack_Push
(gcstack* gc, void* p);

// Create an array of items that are on the stack.
// This array can be sorted with sorting_Sort.
gcstack_item** gcstack_CreateItemsArray
(gcstack const* gc);

// Print out a list of items on the stack to command window.
void gcstack_Print
(gcstack const* gc, void(*print)(void*a));

#endif

#ifdef __cplusplus
}
#endif
