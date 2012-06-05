/*
 *  bitstream.h
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

#ifndef memgroups_bitstream
#define memgroups_bitstream
	
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
	
#include "gcstack.h"
#include "readability.h"
	
	//
	//		Bitstream is a mathematical object that contains one
	//		block for each section in a collection that satisfy a condition.
	//		Bitstreams are useful to calculate intersections (AND)
	//		unions (OR) or subtraction (EXCEPT).
	//
	//		While these operations are similar to those used in if-blocks
	//		and loops in programming, the difference is that Boolean algebra
	//		for one bit has no practical consequences for NOT operations.
	//		In the real world, a single NOT operation without another
	//		following operation gives no meaning.
	//		For example "I live in not-Switzerland."
	//		Instead, we use A*!B which correspond to EXCEPT.
	//		"I want to travel everywhere, except Switzerland".
	//		EXCEPT is always in context to something, therefore
	//		it is written as '-' in pseudo code.
	//		Bitstreams can be inverted, but it is not recommended
	//		as it is not safe. For example, if you have a bitstream that
	//		pointer to memory, and you try to access items in the inverted
	//		memory, you might overwrite data other places.
	//
	//		Here are the operators:
	//
	//		Pseudo-code		Boolean		Precedence		Notes
	//		+				OR			0
	//		-				EXCEPT		-1				A - B = A*!B
	//		*				AND			2
	//		!				NOT			3				Try use except when possible
	//
	//		EXCEPT is execute on every term at same level,
	//	
	//		A + B - C = A*!C + B*!C
	//

typedef		struct bitstream	bitstream;

struct bitstream {
	gcstack_item gc;
	int length;
	int* pointer;
};

//
//			Allocates on a garbage collector stack.
//			This technique makes it easier to write safe code.
//
bitstream*	bitstream_AllocWithGC
(gcstack* gc);

//
//			Initializes with room for a number of int values.
//			If you have an array of 40 items, you need only
//			40 numbers in the worst case in the bitstream.
//			You can use bitstream->pointer to set the elements manually.
//			Each odd term (%2=1) marks the shift from false to true,
//			while even term (%2=0) marks the shift from true to false.
//
bitstream*	bitstream_InitWithSize
(bitstream* a, int size);

//
//			Initializes with values.
//
bitstream*	bitstream_InitWithValues
(bitstream* a, int size, int const vals[]);
	
//
//			Sometimes you have an array of indices and you want to create
//			a bitstream from it. The indices that increment with 1 will
//			be merged into same block in the bitstream.
//          The indices have to be sorted (monetonized).
//
bitstream* bitstream_InitWithIndices
(bitstream* a, int size, int const vals[]);
	
//
//			This looks for difference between a set of old values
//			and new values of type double.
//
bitstream* bitstream_InitWithDeltaDouble
(bitstream* a, int n, const double* oldValues, const double* newValues);

bitstream* bitstream_InitWithDeltaInt
(bitstream* a, int n, const int* oldValues, const int* newValues);
	
bitstream* bitstream_InitWithDeltaBool
(bitstream* a, int n, const bool* oldValues, const bool* newValues);
	
bitstream* bitstream_InitWithDeltaString
(bitstream* a, int n, const string* oldValues, const string* newValues);
	
//
//			Cleans up the structure within a bistream.
//			Frees up the pointer that is pointing to the data.
//			The argument must be void* because that is the standard
//			of garbage collected items.
//
//			You can reuse a bitstream for another if you do not
//			release the pointer after calling bitstream_Delete.
//
void		bitstream_Delete
(void* p);

//
//			Prints out the bitstream to the console.
//			A bitstream is logically divided into blocks
//			a=>b means the block starts at a and ends right before b.
//
void		bitstream_Print
(bitstream const* a);

//
//			If you know all indices in a is less than b,
//			then using DirectJoin is a faster operation than Or.
//
bitstream*	bitstream_DirectJoin
(gcstack* gc, bitstream const* a, bitstream const* b);

//
//			Creates a copy of the bitstream.
//
bitstream*	bitstream_Clone
(gcstack* gc, bitstream const* a);

//
//			Performs an Boolean 'And' operation between two bitstreams.
//			This equal the intersection of amounts in a Venn diagram.
//			The algorithm first does a simulation of the computation
//			to figure out the needed size, then it does the actual job.
//			The simulation algorithm is hidden in the .c file as 'countAnd'.
//
bitstream*	bitstream_And
(gcstack*, bitstream const* a, bitstream const* b);

//
//			Performs a Boolean 'Or' operation between two bitstreams.
//			This equals the sum of two amounts in a Venn diagram.
//			The algorithm first does a simulation of the computation
//			to figure out the needed size, then it does the actual job.
//			The simulation algorithm is hidden in the .c file as 'countOr'.
//
bitstream*	bitstream_Or
(gcstack*, bitstream const* a, bitstream const* b);

//
//			Performs a Boolean 'Except' operation between two bitstreams.
//			It is like to cut of amounts or like when you eat a cookie.
//			If 'a' is the cookie, then 'b' is the teeth marks after chewing.
//			Instead of doing two operations, one NOT and one AND,
//			this algorithm is exact copy of 'And' except for changing a flag.
//			The algorihtm first does a simulation of the computation
//			to figure out the needed size, then it does the actual job.
//			The simulation algorithm is hidden in the .c file as 'countExcept'.
//
bitstream*	bitstream_Except
(gcstack*, bitstream const* a, bitstream const* b);

//
//			Performs an invert at a specified location in the bitstreams.
//			Because bitstream are infinite in both directions,
//			it is not possible to describe an inverted bitstream
//			without adding or removing a number.
//			Bitstreams that have odd length is inverted and infinite,
//			while those with even length is finite.
//			(bitstream is direct proof that there is infinite infinites).
//			The algorithm first does a simulation fo the computation
//			to figure out the needed size, then it does the actual job.
//			The simulation algorithm is hidden in the .c file as 'countInvert'.
//
bitstream*	bitstream_Invert
(gcstack* gc, bitstream* a, int inv);

//
//			Computes the area or size the bitstream covered with
//			true values. You can only use this if you have even length
//			of the bitstream, use 'bitstream_Abs' instead if you need
//			to support bitstreams ending in infinity.
//
int			bitstream_Size
(bitstream const* list);

//
//			Computes the area or size of the bitstream covered with
//			true values. This sets a maximum limit in case the
//			bitstream is inverted. If you prefer to receive
//			negative value if the bitstream is infinite,
//			then you can use 'bitstream_AbsSub' in the .c file.
//			For usual applications it is enough with this method.
//
int			bitstream_Abs
(bitstream const* list, int maximum);

//
//			Returns the pointer at beginning of bitstream.
//
int* 		bitstream_ArrayPointer(bitstream const* a);

//
//			Returns the amoutn of blocks in the bitstream.
//
int			bitstream_NumberOfBlocks(bitstream const* a);

//
//			Removes and returns an index from the beginning of the bitstream.
//			It is fast within a single block, but require extra operations
//			at the end of each block.
//			Don't use it on inverted bitstreams.
//
int			bitstream_PopStart(bitstream* a);

//
//			Removes and returns an index from the end of the bitstream.
//			This is faster than PopStart, because no need to move data.
//			Don't use it on inverted bitstreams.
//
int			bitstream_PopEnd(bitstream* a);
	
#endif

#ifdef __cplusplus
}
#endif