/*
 *  sorting.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 18.05.12.
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

#include <string.h>
#include "gcstack.h"
#include "sorting.h"
#include "readability.h"

/*
 
 I am using QuickSort to sort data, but I had to adapt the algorithm
 for doing general sorting. Here is the original algorithm for
 controlling if you suspect it is anything wrong with the sorting.
 
 void swap(int *a, int *b)
 {
	int t=*a; *a=*b; *b=t;
 }
 void sort(int arr[], int beg, int end)
 {
	if (end > beg + 1)
	{
		int piv = arr[beg], l = beg + 1, r = end;
		while (l < r)
		{
			if (arr[l] <= piv)
				l++;
			else
				swap(&arr[l], &arr[--r]);
		}
		swap(&arr[--l], &arr[beg]);
		sort(arr, beg, l);
		sort(arr, r, end);
	}
 }
 
 */


//	The reasons this function takes bytes as arguments is because
//	it copies the bytes when swapping.
void swapData(byte*t, byte*a, byte*b, int stride)
{
	memcpy(t, a, stride);
	memcpy(a, b, stride);
	memcpy(b, t, stride);
	
	// Swap the items at the gc stack to avoid damage.
	// This results that the order which a and b will be
	// released will change, or they might change gc stack.
	gcstack_Swap(a, b);
}

void sorting_Sort(void* arr[], 
				 int beg, int end, int stride, void*t,
				 int(*compare)(void const* a, void const* b))
{
	if (end <= beg + 1) return;
	
	void const* piv = arr[beg];
	int l = beg + 1, r = end;
	while (l < r)
	{
		if (compare(arr[l], piv) <= 0)
			l++;
		else
		{
			r--;
			swapData(t, arr[l], arr[r], stride);
		}
	}
	l--;
	swapData(t, arr[l], arr[beg], stride);
	sorting_Sort(arr, beg, l, stride, t, compare);
	sorting_Sort(arr, r, end, stride, t, compare);
}



//
// Searches a list of variables for a string using binary search.
//
int sorting_SearchBinary(int n, void const* arr[], void const* data, 
							 int(*compare)(void const*a,void const* b))
{
	int l=0;
	int u=n-1;
	int mid = 0;
	int res = 0;
	while(l<=u){
		mid=(l+u)/2;
		res = compare(data, arr[mid]);
		if(res == 0) {
			return mid;
		}
		else if(res < 0)
			u=mid-1;
		else
			l=mid+1;
	}
	return -1;
}

