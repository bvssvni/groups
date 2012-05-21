/*
 *  sorting.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 18.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */


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

#include "memgroups.h"

void swapData(byte*t, byte*a, byte*b, int stride)
{
	for (int i = 0; i < stride; i++)
	{
		t[i] = a[i];
		a[i] = b[i];
		b[i] = t[i];
	}
}

void sorting_Sort(byte**arr, 
				 int beg, int end, int stride, byte*t,
				 compareFunction())
{
	if (end <= beg + 1) return;
	
	byte* piv = arr[beg];
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
int sorting_SearchBinaryText(int n, void** arr, void* name, 
							 compareFunction())
{
	int l=0;
	int u=n-1;
	int mid = 0;
	int res = 0;
	while(l<=u){
		mid=(l+u)/2;
		res = compare(name, arr[mid]);
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

