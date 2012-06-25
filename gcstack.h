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
	
	//
	//      GARBAGE COLLECTION
	//
	//      Declare 'gcstack_item gc' at beginning of struct to support being added to this stack.
	//
	typedef struct gcstack_item gcstack_item;
	struct gcstack_item
	{
		gcstack_item* previous;
		gcstack_item* next;
		void(*freeSubPointers)(void* const p);
	};
	
	//
	//      STANDARD DATA TYPES
	//
	//      These types are commonly used and got their own data type to be easier to use.
	//
	typedef struct gcdouble
	{
		gcstack_item gc;
		double val;
	} gcdouble;
	
	typedef struct gcint
	{
		gcstack_item gc;
		int val;
	} gcint;
	
	typedef struct gcbool
	{
		gcstack_item gc;
		int val;
	} gcbool;
	
	typedef struct gcstring
	{
		gcstack_item gc;
		char* val;
	} gcstring;
	
	//
	// Garbage collector stack.
	// This works as the container of a double-linked list.
	// The root is a stat item on the top of the stack.
	// The length is the depth of the stack according to counting.
	//
	typedef struct gcstack
	{
		int length;
		gcstack_item* root;
	} gcstack;
	
	//
	// This function is handy when you have one gc
	// in an object that contains the members,
	// you can swap one above the current level and one
	// below so the old one gets deleted.
	//
	void gcstack_Swap
	(void* a, void* b);
	
	//
	// Pops item from the stack, calls the free method and frees the pointer.
	//
	void gcstack_free
	(gcstack* gc, void* p);
	
	gcstack* gcstack_Alloc
	();
	
	//
	// Deletes the gcstack and all the data it refers to.
	//
	void gcstack_Delete
	(gcstack* gc);
	
	//
	// Initializes a garbage collected item.
	//
	gcstack_item* gcstack_malloc
	(gcstack* gc, int size, void(*free)(void* const p));
	
	gcstack* gcstack_Init
	(gcstack* gc);
	
	//
	//      If you don't like to have items in reverse, you can reorder them.
	//
	void gcstack_ReverseWithLevel
	(gcstack* gc, int level);
	
	//
	//      When reversing to another stack, you need to specify a level where to stop.
	//
	void gcstack_ReverseToOtherStackWithLevel
	(gcstack* from, gcstack* to, int level);
	
	const gcstack_item* gcstack_Start
	(gcstack const* gc);
	
	//
	//      Destroys objects on the stack back to a certain pointer.
	//
	void gcstack_End
	(gcstack* gc, const gcstack_item* start);
	
	//
	//      Destroys objects on the stack back to a certain size.
	//
	void gcstack_EndLevel
	(gcstack* gc, int level);
	
	void gcstack_Pop
	(gcstack* gc, void* p);
	
	//
	// Pushes an item on the stack.
	//
	void gcstack_Push
	(gcstack* gc, gcstack_item* item);
	
	//
	//      SPECIAL TYPE PUSH AND POP
	//
	//      Pushes a double to the stack, using internal type for it.
	//      This makes it easier to use gcstack for numerical calculations.
	//
	gcstack_item* gcstack_PushDouble
	(gcstack* gc, double val);
	
	gcstack_item* gcstack_PushInt
	(gcstack* gc, int val);
	
	gcstack_item* gcstack_PushBool
	(gcstack* gc, int val);
	
	gcstack_item* gcstack_PushString
	(gcstack* gc, const char* val);
	
	//
	//      Pops double from the stack.
	//
	double gcstack_PopDouble
	(gcstack* gc);
	
	int gcstack_PopInt
	(gcstack* gc);
	
	int gcstack_PopBool
	(gcstack* gc);
	
	char* gcstack_PopString
	(gcstack* gc);
	
	double gcstack_PopDoubleWithItem
	(gcstack* gc, gcstack_item* item);
	
	int gcstack_PopIntWithItem
	(gcstack* gc, gcstack_item* item);
	
	int gcstack_PopBoolWithItem
	(gcstack* gc, gcstack_item* item);
	
	const char* gcstack_PopStringWithItem
	(gcstack* gc, gcstack_item* item);
	
	//
	//      Pops double from the stack with a specific item.
	//
	double gcstack_PopDoubleWithItem
	(gcstack* gc, gcstack_item* item);
	
	//
	//      Create an array of items that are on the stack.
	//      In a stack the first item has usually the last index.
	//      If you need the array to be in the same order as incremental index,
	//      Use the 'gcstack_CreateItemsArrayBackward' function instead.
	//      This array can be sorted with sorting_Sort.
	//
	gcstack_item** gcstack_CreateItemsArray
	(gcstack const* gc);
	
	int* gcstack_CreateIntArray
	(gcstack const* gc);
	
	double* gcstack_CreateDoubleArray
	(gcstack const* gc);
	
	int* gcstack_CreateBoolArray
	(gcstack const* gc);
	
	const char** gcstack_CreateStringArray
	(gcstack const* gc);
	
	//
	//      BACKWARD ARRAYS
	//
	//      These arrays are in the order expected from a queue.
	//
	//      Create an array of items that are on the stack backward.
	//      This array can be sorted with sorting_Sort.
	//
	gcstack_item** gcstack_CreateItemsArrayBackward(gcstack const* gc);
	
	int* gcstack_CreateIntArrayBackward
	(gcstack const* gc);
	
	double* gcstack_CreateDoubleArrayBackward
	(gcstack const* gc);
	
	const char** gcstack_CreateStringArrayBackward
	(gcstack const* gc);
	
	int* gcstack_CreateBoolArrayBackward
	(gcstack const* gc);
	
	//
	//      PRINTING
	//
	//      Print out a list of items on the stack to command window.
	//
	void gcstack_Print
	(gcstack const* gc, void(*print)(void*a));
	
	//
	//      Prints out the stack as if they all were ints.
	//      Don't do this if you are not sure.
	//
	void gcstack_PrintInt
	(gcstack const* gc);
	
	void gcstack_PrintString
	(gcstack const* gc);
	
	//
	//      Takes all int values on the stack and constructs a string from it.
	//
	char* gcstack_PopIntsAsString
	(gcstack* gc);
	
#endif
	
#ifdef __cplusplus
}
#endif
