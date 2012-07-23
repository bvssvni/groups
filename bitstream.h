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
	
#ifndef memgroups_bitstream_internal
#undef private
#define private const
#else
#undef private
#define private
#endif
	
	//
	//	Bitstream is a mathematical object that contains one
	//	block for each section in a collection that satisfy a condition.
	//	Bitstreams are useful to calculate intersections (AND)
	//	unions (OR) or subtraction (EXCEPT).
	//
	//	While these operations are similar to those used in if-blocks
	//	and loops in programming, the difference is that Boolean algebra
	//	for one bit has no practical consequences for NOT operations.
	//	In the real world, a single NOT operation without another
	//	following operation gives no meaning.
	//	For example "I live in not-Switzerland."
	//	Instead, we use A*!B which correspond to EXCEPT.
	//	"I want to travel everywhere, except Switzerland".
	//	EXCEPT is always in context to something, therefore
	//	it is written as '-' in pseudo code.
	//	Bitstreams can be inverted, but it is not recommended
	//	as it is not safe. For example, if you have a bitstream that
	//	pointer to memory, and you try to access items in the inverted
	//	memory, you might overwrite data other places.
	//
	//	Here are the operators:
	//
	//	Ps.code	Boolean	Prec.	Notes
	//	+	OR	0
	//	-	EXCEPT	-1	A - B = A*!B
	//	*	AND	2
	//	!	NOT	3	Try use except when possible
	//
	//	EXCEPT is execute on every term at same level,
	//	
	//	A + B - C = A*!C + B*!C
	//
	
	typedef struct bitstream {
		private gcstack_item gc;
		private int length;
		private int* pointer;
	} bitstream;
	
	//
	//	Allocates on a garbage collector stack.
	//	This technique makes it easier to write safe code.
	//
	bitstream* bitstream_GcAlloc
	(gcstack* const gc);
	
	//
	//	Initializes with room for a number of int values.
	//	If you have an array of 40 items, you need only
	//	40 numbers in the worst case in the bitstream.
	//	You can use bitstream->pointer to set the elements manually.
	//	Each odd term (%2=1) marks the shift from false to true,
	//	while even term (%2=0) marks the shift from true to false.
	//
	bitstream* bitstream_InitWithSize
	(bitstream* const a, const int size);
	
	//
	//	Initializes with values.
	//
	bitstream* bitstream_InitWithValues
	(bitstream* const a, const int size, const int* const vals);
	
	//
	//	Sometimes you have an array of indices and you want to create
	//	a bitstream describing which indices are following each other
	//	and which are not. The indices that increment with 1 will
	//	be merged into same block in the bitstream.
	//  The indices have to be sorted from lowest index to highest.
	//
	bitstream* bitstream_InitWithIndices
	(bitstream* const a, const int size, const int* const vals);
	
	//
	//	This initializes a bitstream with a function that tells
	//	whether a given property is true for a member in an array.
	//	It loops through the array and adds the position to the
	//	bitstream each time property changes.
	//	The returned value is aligned to give a finite set.
	//
	bitstream* bitstream_InitWithFunction
	(bitstream* const a, const int arrc, const int stride, 
	const void* const arrv, 
	int (* const f)(const void* const p));
	
	//
	//      DELTA CHANGES
	//
	//      When you have one array of new values and one of old values,
	//      you can compare them and find which has changed and which has not.
	//      It is assumed that the arrays got the same size.
	//
	
	bitstream* bitstream_InitWithDeltaDouble
	(bitstream* const a, const int n, const double* const oldValues, 
	 const double* const newValues);
	
	bitstream* bitstream_InitWithDeltaInt
	(bitstream* const a, const int n, const int* const oldValues, 
	 const int* const newValues);
	
	bitstream* bitstream_InitWithDeltaBool
	(bitstream* const a, const int n, const int* const oldValues, 
	 const int* const newValues);
	
	bitstream* bitstream_InitWithDeltaString
	(bitstream* const a, const int n, const char** const oldValues, 
	 const char** const newValues);
	
	//
	//      TEXT PROCESSING
	//
	//      Sometimes it is nice to find out where words are in a string.
	//      Space characters are ignored if they are neighbors to a previous one, for example '  '.
	//      Split characters are not ignores when neighbor to a previous one, for example 'a,,b,c'.
	//
	bitstream* bitstream_InitWithWordsInString
	(bitstream* const a, const char* const text, 
	 const char* const spaceCharacters, const char* const splitCharacters);
	
	//
	//      Returns an array of strings by extracting word locations within a text.
	//
	char** bitstream_GetWords
	(bitstream* const a, const char* const text);
	
	//
	//	Cleans up the structure within a bistream.
	//	Frees up the pointer that is pointing to the data.
	//	The argument must be void* because that is the standard
	//	of garbage collected items.
	//
	//	You can reuse a bitstream for another if you do not
	//	release the pointer after calling bitstream_Delete.
	//
	void bitstream_Delete
	(void* const p);
	
	//
	//	Prints out the bitstream to the console.
	//	A bitstream is logically divided into blocks
	//	a=>b means the block starts at a and ends right before b.
	//
	void bitstream_Print
	(const bitstream* const a);
	
	//
	//	If you know all indices in a is less than b,
	//	then using DirectJoin is a faster operation than Or.
	//
	bitstream* bitstream_GcDirectJoin
	(gcstack* const gc, const bitstream* const a, const bitstream* const b);
	
	//
	//	Creates a copy of the bitstream.
	//
	bitstream* bitstream_GcClone
	(gcstack* const gc, const bitstream* const a);
	
	//
	//	Performs an Boolean 'And' operation between two bitstreams.
	//	This equal the intersection of amounts in a Venn diagram.
	//	The algorithm first does a simulation of the computation
	//	to figure out the needed size, then it does the actual job.
	//	The simulation algorithm is hidden in the .c file as 'countAnd'.
	//
	bitstream* bitstream_GcAnd
	(gcstack* const gc, const bitstream* const a, const bitstream* const b);
	
	//
	//	Performs a Boolean 'Or' operation between two bitstreams.
	//	This equals the sum of two amounts in a Venn diagram.
	//	The algorithm first does a simulation of the computation
	//	to figure out the needed size, then it does the actual job.
	//	The simulation algorithm is hidden in the .c file as 'countOr'.
	//
	bitstream* bitstream_GcOr
	(gcstack* const gc, const bitstream* const a, const bitstream* const b);
	
	//
	//	Performs a Boolean 'Except' operation between two bitstreams.
	//	It is like to cut of amounts or like when you eat a cookie.
	//	If 'a' is the cookie, then 'b' is the teeth marks after chewing.
	//	Instead of doing two operations, one NOT and one AND,
	//	this algorithm is exact copy of 'And' except for changing a flag.
	//	The algorihtm first does a simulation of the computation
	//	to figure out the needed size, then it does the actual job.
	//	The simulation algorithm is hidden in the .c file as 'countExcept'.
	//
	bitstream* bitstream_GcExcept
	(gcstack* const gc, const bitstream* const a, const bitstream* const b);
	
	//
	//	Performs a Boolean 'Except' operation between two bitstreams.
	//	It differs from 'bitstream_Except' by the way that the struct
	//	used to contain the values must be passed to the function.
	//
	void bitstream_ExceptTmp
	(const bitstream* const a, 
	 const bitstream* const b, bitstream* const tmp);
	
	//
	//	Performs an invert at a specified location in the bitstreams.
	//	Because bitstream are infinite in both directions,
	//	it is not possible to describe an inverted bitstream
	//	without adding or removing a number.
	//	Bitstreams that have odd length is inverted and infinite,
	//	while those with even length is finite.
	//	(bitstream is direct proof that there is infinite infinites).
	//	The algorithm first does a simulation fo the computation
	//	to figure out the needed size, then it does the actual job.
	//	The simulation algorithm is hidden in the .c file as 'countInvert'.
	//
	bitstream* bitstream_GcInvert
	(gcstack* const gc, bitstream* const a, const int inv);
	
	//
	//	Computes the area or size the bitstream covered with
	//	true values. You can only use this if you have even length
	//	of the bitstream, use 'bitstream_Abs' instead if you need
	//	to support bitstreams ending in infinity.
	//
	int bitstream_Size
	(const bitstream* const list);
	
	//
	//	Computes the area or size of the bitstream covered with
	//	true values. This sets a maximum limit in case the
	//	bitstream is inverted. If you prefer to receive
	//	negative value if the bitstream is infinite,
	//	then you can use 'bitstream_AbsSub' in the .c file.
	//	For usual applications it is enough with this method.
	//
	int bitstream_Abs
	(const bitstream* const list, const int maximum);
	
	//
	//	Returns the pointer at beginning of bitstream.
	//
	int* bitstream_ArrayPointer
	(const bitstream* const a);
	
	//
	//	Returns the amoutn of blocks in the bitstream.
	//
	int bitstream_NumberOfBlocks
	(const bitstream* const a);
	
	//
	//	Removes and returns an index from the beginning of the 
	//	bitstream. It is fast within a single block, but require extra 
	//	operations at the end of each block.
	//	Don't use it on inverted bitstreams.
	//
	int bitstream_PopStart
	(bitstream* const a);
	
	//
	//	Removes and returns an index from the end of the bitstream.
	//	This is faster than PopStart, because no need to move data.
	//	Don't use it on inverted bitstreams.
	//
	int bitstream_PopEnd
	(bitstream* const a);
	
#endif
	
#ifdef __cplusplus
}
#endif
