
//        //////  //////    /////   //   //  /////   ///////
//      //        //  //  //   //  //   //  //   // //
//     //  ///  /////    //   //  //   //  //////   //////
//    //   //  //  //   //   //  //   //  //            //
//   //////   //   //   /////    /////   //       //////

//			Groups is a library for group-oriented programming.
//			To learn more, read REAME.

/*
 *  memgroups.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 11.05.12.
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

#ifndef memgroups
#define memgroups

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>

// Items for garbage collection.
typedef struct gcstack_item gcstack_item;
struct gcstack_item
{
			gcstack_item* previous;
			gcstack_item* next;
			void(*free)(void* p);
};

typedef struct gcstack
{
			int length;
			gcstack_item* root;
} gcstack;

gcstack*	gcstack_Alloc
					();
gcstack*	gcstack_Init
					(gcstack* gc);
int			gcstack_Start
					(gcstack* gc);
void		gcstack_End
					(gcstack* gc, int level);
void		gcstack_Pop
					(gcstack* gc, void* p);
void		gcstack_Push
					(gcstack* gc, void* p);

//			Initializes a garbage collected item.
gcstack_item*
			gcstack_malloc
					(gcstack* gc, int size, void(*free)(void* p));

//			This function is handy when you have one gc
//			in an object that contains the members,
//			you can swap one above the current level and one
//			below so the old one gets deleted.
void		gcstack_Swap
					(void* a, void* b);

//			Deletes an item directly, causing it to step out of
//			the double-linked list and destruct itself.
void		gcstack_free
					(void* p);

//			Deletes the gcstack and all the data it refers to.
void		gcstack_Delete
					(gcstack* gc);

// Add some to make code look nicer.
#define false 0
#define true 1
typedef int bool;
typedef unsigned char byte;
typedef char* string;

typedef		struct bitstream	bitstream;
typedef		struct bitstream	property;
typedef		struct bitstream	member;

//			
typedef		struct bitstream	strings;
typedef		struct bitstream	doubles;
typedef		struct bitstream	ints;

//			This function returns an int that tells the order
//			of two objects. If 'a' is less than 'b', it returns -1.
//			If 'a' and 'b' are equal, it returns 0.
//			If 'a' is greater than 'b' it returns 1.
#define		compareFunction()	int(*compare)(void* a, void* b)

#define		_and(a,b)			bitstream_And(gc, a, b)
#define		_and3(a,b,c)		bitstream_And(gc, bitstream_And(gc, a, b), c)
#define		_and4(a,b,c,d)		bitstream_And(gc, bitstream_And(gc, bitstream_And(gc, a, b), c), d)

#define		_or(a,b)			bitstream_Or(gc, a, b)
#define		_or3(a,b,c)			bitstream_Or(gc, bitstream_Or(gc, a, b), c)
#define		_or4(a,b,c,d)		bitstream_Or(gc, bitstream_Or(gc, bitstream_Or(gc, a, b), c), d)

#define		_ex(a,b)			bitstream_ex(gc, a, b)
#define		_ex3(a,b,c)			bitstream_ex(gc, bitstream_Or(gc, a, b), c)
#define		_ex4(a,b,c,d)		bitstream_ex(gc, bitstream_Or(gc, bitstream_Or(gc, a, b), c), d)

struct bitstream {
	gcstack_item gc;
	int length;
	int* pointer;
};

bitstream*	bitstream_Init
					(gcstack* gc, int size);
void		bitstream_Delete
					(void* p);
bitstream*	bitstream_InitWithValues
					(gcstack* gc, int size, int vals[]);
void		bitstream_Print	(bitstream* a);
bitstream*	bitstream_DirectJoin
					(gcstack* gc, bitstream* a, bitstream* b);
bitstream*	bitstream_Clone
					(gcstack* gc, bitstream* a);

bitstream*	bitstream_And
					(gcstack*, bitstream* a, bitstream* b);
bitstream*	bitstream_Or
					(gcstack*, bitstream* a, bitstream* b);
bitstream*	bitstream_Except
					(gcstack*, bitstream* a, bitstream* b);
bitstream*	bitstream_Invert
					(gcstack* gc, bitstream* a, int inv);

int			bitstream_Size
					(bitstream* list);
int			bitstream_Abs
					(bitstream* list, int maximum);

int			string_Length
					(string str);
string		string_Copy
					(string str);
void		string_Delete
					(string str);
bool string_Equals	(string a, string b);

//
//			Uses QuickSort to sort data pointed by an array.
//				arr		The array of pointers to the data to sort.
//				beg		The first index to start sorting.
//				end		The index after the last position in 
//						the array to sort.
//				stride	The size of each block of data.
//				t		A temporary buffer for swapping at 
//						the size of the stride.
//				compare	The function to compare.
//
void		sorting_Sort
					(byte**arr, int beg, int end, 
					 int stride, byte*t,
					 compareFunction());

//			Searches a table of pointers to variable,
//			assuming they are sorted, looking for a name.
int			sorting_SearchBinaryText
					(int n, void** arr, void* name,
					 compareFunction());

#endif
