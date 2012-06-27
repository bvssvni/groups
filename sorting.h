/*
 *  sorting.h
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
	
#ifndef memgroups_sorting
#define memgroups_sorting
	
	//
	//	Uses QuickSort to sort data pointed by an array.
	//
	//	arr	The array of pointers to the data to sort.
	//		The data must start with gcstac_item (not pointer)
	//		in the beginning of the struct.
	//
	//	beg	The first index to start sorting.
	//
	//	end	The index after the last position in 
	//		the array to sort.
	//		Typically you can just set this to the
	//		'length' of the array.
	//
	//	stride	The size of each block of data.
	//		This is used to swap the content of the
	//		of the data. The sorting algorithm
	//		does not swap pointers, so the data can be sorted
	//		in the buffer where they are stored.
	//		This means you can 'sort' to remove fragmentations.
	//
	//	t	A temporary buffer for swapping at 
	//		the size of the stride.
	//
	//	compare	The function to compare.
	//			Returns a < b	->	-1
	//				a > b	->	1
	//				a = b	->	0
	//
	void		sorting_Sort
	(
	 void * arr[],
	 int beg, 
	 int end, 
	 int stride, 
	 void*t,
	 int (*compare) (void const* a,void const* b)
	 );
	
	//
	//	Searches a table of pointers for a variable.
	//	The table must be sorted.
	//
	//	n	The number of items in the array.
	//
	//	arr	The array of pointers.
	//
	//	name	A pointer to the data to search for.
	//
	//	compare	The function to compare.
	//		Returns a < b	->		-1
	//			a > b	->		1
	//			a = b	->		0
	//	This is used to detect whether we have found it.
	//
	int sorting_SearchBinary
	(
	 int n, 
	 void const* arr[], 
	 void const* data,
	 int(*compare)(void const*a,void const*b)
	 );
	
	
#endif
	
#ifdef __cplusplus
}
#endif
