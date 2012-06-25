/*
 *  bitstream.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include "errorhandling.h"
#include "readability.h"

#include "bitstream.h"

int bitstream_references = 0;

void bitstream_Delete(void* const p)
{
	macro_err(p == NULL);
	
	bitstream_references--;
	// printf("%i\r\n", bitstream_references);
	
	bitstream* const a = (bitstream* const)p;
	
	if (a->pointer != NULL)
	{
		free(a->pointer);
		a->pointer = NULL;
	}
	
	a->length = 0;
}

bitstream* bitstream_AllocWithGC(gcstack* gc) 
{
	return (bitstream*)gcstack_malloc(gc, sizeof(bitstream), bitstream_Delete);
}

bitstream* bitstream_InitWithSize(bitstream* a, int size) 
{
	macro_err(a == NULL); macro_err(size < 0);
	
	bitstream_references++;
	
	a->pointer = NULL;
	
	if (size == 0)
	{
		a->length = 0;
		a->pointer = NULL;
		return a;
	}
	
	a->length = size;
	
	if (size == 0) return a;
	
	a->pointer = malloc(sizeof(int)*size);
	for (int i = 0; i < size; i++)
		a->pointer[i] = 0;
	return a;
}

bitstream* bitstream_InitWithValues(bitstream* a, int size, int const vals[])
{
	macro_err(a == NULL); macro_err(size < 0); macro_err(vals == NULL);
	
	bitstream_references++;
	
	a->pointer = NULL;
	a->length = size;
	
	if (size == 0) return a;
	
	a->pointer = malloc(sizeof(int)*size);
	memcpy((void*)a->pointer, (void*)vals, size*sizeof(int));
	return a;
}

int countWithIndices(int size, int const vals[])
{
	// Property list = new Property(x.Length*2);
	int count = 0;
	int expected = 0;
	for (int i = 0; i < size; i++)
	{
		if (i > 0 && vals[i] != expected)
			// list.Add(expected);
			count++;
		else if (i > 0)
		{
			expected = vals[i]+1;
			continue;
		}
		
		// list.Add(x[i]);
		count++;
		
		expected = vals[i]+1;
	}
	if ((count%2) != 0)
	{
		// Add last.
		// list.Add(x[x.Length-1]+1);
		count++;
	}
	// return list;
	return count;
}

int* createArrayFromIndices(int count, int size, int const vals[])
{
	int* list = malloc(sizeof(int*)*count);
	int expected = 0;
	int k = 0;
	for (int i = 0; i < size; i++)
	{
		if (i > 0 && vals[i] != expected)
			list[k++] = expected;
		else if (i > 0)
		{
			expected = vals[i]+1;
			continue;
		}
		
		list[k++] = vals[i];
		
		expected = vals[i]+1;
	}
	if ((k%2) != 0)
	{
		// Add last.
		list[k++] = vals[size-1]+1;
	}
	return list;
}

bitstream* bitstream_InitWithIndices(bitstream* a, int size, int const vals[])
{
	macro_err(a == NULL); macro_err(size < 0); macro_err(vals == NULL);
	
	bitstream_references++;
	
	a->pointer = NULL;
	
	if (size == 0) {
		a->length = 0;
		return a;
	}
	
	a->length = countWithIndices(size, vals);
	a->pointer = createArrayFromIndices(a->length, size, vals);
	return a;
}

int countDeltaDouble(int n, const double* old, const double* new)
{
	int count = 0;
	bool was = false;
	bool is;
	for (int i = 0; i < n; i++) {
		is = new[i] != old[i];
		if (was != is)
			count++;
		was = is;
	}
	if (was) {
		count++;
	}
	return count;
}

int countDeltaInt(int n, const int* old, const int* new)
{
	int count = 0;
	bool was = false;
	bool is;
	for (int i = 0; i < n; i++) {
		is = new[i] != old[i];
		if (was != is)
			count++;
		was = is;
	}
	if (was) {
		count++;
	}
	return count;
}


int countDeltaBool(int n, const bool* old, const bool* new)
{
	int count = 0;
	bool was = false;
	bool is;
	for (int i = 0; i < n; i++) {
		is = new[i] != old[i];
		if (was != is)
			count++;
		was = is;
	}
	if (was) {
		count++;
	}
	return count;
}

int countDeltaString(int n, string* old, string* new)
{
	int count = 0;
	bool was = false;
	bool is;
	for (int i = 0; i < n; i++) {
		is = strcmp(new[i], old[i]) != 0;
		if (was != is)
			count++;
		was = is;
	}
	if (was) {
		count++;
	}
	return count;
}

bitstream* bitstream_InitWithDeltaDouble
(bitstream* a, int n, const double* oldValues, const double* newValues)
{
	macro_err(a == NULL); macro_err(n < 0); macro_err(oldValues == NULL); macro_err(newValues == NULL);
	
	a->pointer = NULL;
	
	if (n == 0) {
		a->length = 0;
		return a;
	}
	
	int count = countDeltaDouble(n, oldValues, newValues);
	a->length = count;
	a->pointer = malloc(sizeof(count));
	bool was = false;
	bool is;
	int k = 0;
	for (int i = 0; i < n; i++) {
		is = newValues[i] != oldValues[i];
		if (was != is)
			a->pointer[k++] = i;
		was = is;
	}
	if (was) {
		a->pointer[k++] = n;
	}
	return a;
}

bitstream* bitstream_InitWithDeltaInt
(bitstream* a, int n, const int* oldValues, const int* newValues)
{
	macro_err(a == NULL); macro_err(n < 0); macro_err(oldValues == NULL); macro_err(newValues == NULL);
	
	a->pointer = NULL;
	
	if (n == 0) {
		a->length = 0;
		return a;
	}
	
	int count = countDeltaInt(n, oldValues, newValues);
	a->length = count;
	a->pointer = malloc(sizeof(count));
	bool was = false;
	bool is;
	int k = 0;
	for (int i = 0; i < n; i++) {
		is = newValues[i] != oldValues[i];
		if (was != is)
			a->pointer[k++] = i;
		was = is;
	}
	if (was) {
		a->pointer[k++] = n;
	}
	return a;
}

bitstream* bitstream_InitWithDeltaBool
(bitstream* a, int n, const bool* oldValues, const bool* newValues)
{
	macro_err(a == NULL); macro_err(n < 0); macro_err(oldValues == NULL); macro_err(newValues == NULL);
	
	a->pointer = NULL;
	
	if (n == 0) {
		a->length = 0;
		return a;
	}
	
	int count = countDeltaBool(n, oldValues, newValues);
	a->length = count;
	a->pointer = malloc(sizeof(count));
	bool was = false;
	bool is;
	int k = 0;
	for (int i = 0; i < n; i++) {
		is = newValues[i] != oldValues[i];
		if (was != is)
			a->pointer[k++] = i;
		was = is;
	}
	if (was) {
		a->pointer[k++] = n;
	}
	return a;
}

bitstream* bitstream_InitWithDeltaString
(bitstream* a, int n, const string* oldValues, const string* newValues)
{
	macro_err(a == NULL); macro_err(n < 0); macro_err(oldValues == NULL); macro_err(newValues == NULL);
	
	a->pointer = NULL;
	
	if (n == 0) {
		a->length = 0;
		return a;
	}
	
	int count = countDeltaString(n, oldValues, newValues);
	a->length = count;
	a->pointer = malloc(sizeof(count));
	bool was = false;
	bool is;
	int k = 0;
	for (int i = 0; i < n; i++) {
		is = strcmp(newValues[i], oldValues[i]) != 0;
		if (was != is)
			a->pointer[k++] = i;
		was = is;
	}
	if (was) {
		a->pointer[k++] = n;
	}
	return a;
}

bitstream* bitstream_InitWithWordsInString(bitstream* a, string text, string spaceCharacters, string splitCharacters)
{
	macro_err(a == NULL); macro_err(text == NULL); macro_err(spaceCharacters == NULL); 
	macro_err(splitCharacters == NULL);
	
	// Loop through and find all sections that does not contain splitting characters.
	gcstack* words = gcstack_Init(gcstack_Alloc());
	int k = 0;
	bool wasSpace = true;
	bool isSpace = false;
	bool isSplit = false;
	char* spaceCh = NULL;
	char* splitCh = NULL;
	for (char ch = text[0]; ch != '\0'; ch = text[++k])
	{
		spaceCh = strchr(spaceCharacters, ch);
		splitCh = strchr(splitCharacters, ch);
		isSplit = splitCh != NULL;
		isSpace = spaceCh != NULL || isSplit;
		
		// Split characters are marked whether they follow another or not.
		if (isSplit) gcstack_PushInt(words, k);
		else if (!isSpace && wasSpace) gcstack_PushInt(words, k);
		else if (isSpace && !wasSpace) gcstack_PushInt(words, k);
		wasSpace = isSpace;
	}
	
	// If there is no split character at the end, we have to use end of text.
	if ((words->length % 2) != 0)
		gcstack_PushInt(words, k);
	
	int* arr = gcstack_CreateIntArrayBackward(words);
	bitstream_InitWithValues(a, words->length, arr);
	free(arr);
	gcstack_Delete(words);
	free(words);
	
	return a;
}

void bitstream_Print(bitstream const*a)
{
	macro_err(a == NULL);
	
	int length = a->length-1;
	for (int i = 0; i < length; i += 2) {
		int p1 = a->pointer[i];
		int p2 = a->pointer[i+1];
		printf("%i,%i ", p1, p2);
	}
	printf("\r\n");
}

bitstream* bitstream_DirectJoin(gcstack* gc, 
				bitstream const* a, bitstream const* b) {
	macro_err(a == NULL); macro_err(b == NULL);
	
	bitstream* arr = (bitstream*)gcstack_malloc(gc, sizeof(bitstream), bitstream_Delete);
	arr->length = a->length + b->length;
	arr->pointer = malloc(sizeof(int)*arr->length);
	
	memcpy((void*)arr->pointer, (void*)a->pointer, a->length*sizeof(int));
	memcpy((void*)(arr->pointer+a->length), (void*)b->pointer, b->length*sizeof(int));
	
	return arr;
}

bitstream* bitstream_Clone(gcstack* gc, bitstream const* a) {
	macro_err(a == NULL);
	
	bitstream* b = (bitstream*)gcstack_malloc(gc, sizeof(bitstream), bitstream_Delete);
	b->length = a->length;
	b->pointer = malloc(sizeof(int)*b->length);
	
	memcpy((void*)b->pointer, (void*)a->pointer, a->length*sizeof(int));
	
	return b;
}

int countAnd(bitstream const* a, bitstream const* b)
{
	int list = 0;
	
	if (a->length == 0 || b->length == 0)
		return list;
	
	int i = 0, j = 0;
	bool ba = false;
	bool bb = false;
	bool oldB = false;
	while (i < a->length || j < b->length)
	{
		int pa = a->pointer[i>=a->length?a->length-1:i];
		int pb = b->pointer[j>=b->length?b->length-1:j];
		
		if (pa == pb)
		{
			ba = !ba;
			bb = !bb;
			if ((ba && bb) != oldB)
				list++;
			i++;
			j++;
		}
		else if ((pa < pb || j >= b->length) && i < a->length)
		{
			ba = !ba;
			if ((ba && bb) != oldB)
				list++;
			i++;
		}
		else if (j < b->length)
		{
			bb = !bb;
			if ((ba && bb) != oldB)
				list++;
			j++;
		}
		else
			break;
		
		oldB = ba && bb;
	}
	
	return list;
}


bitstream* bitstream_And(gcstack* gc, bitstream const* a, bitstream const* b)
{
	macro_err(a == NULL); macro_err(b == NULL);
	
	int list = 0;
	bitstream* arr = bitstream_InitWithSize
	(bitstream_AllocWithGC(gc), countAnd(a, b));
	
	if (a->length == 0 || b->length == 0)
		return arr;
	
	int i = 0, j = 0;
	bool ba = false;
	bool bb = false;
	bool oldB = false;
	while (i < a->length || j < b->length)
	{
		int pa = a->pointer[i>=a->length?a->length-1:i];
		int pb = b->pointer[j>=b->length?b->length-1:j];
		
		if (pa == pb)
		{
			ba = !ba;
			bb = !bb;
			if ((ba && bb) != oldB)
			{
				arr->pointer[list++] = pa;
			}
			i++;
			j++;
		}
		else if ((pa < pb || j >= b->length) && i < a->length)
		{
			ba = !ba;
			if ((ba && bb) != oldB)
				arr->pointer[list++] = pa;
			i++;
		}
		else if (j < b->length)
		{
			bb = !bb;
			if ((ba && bb) != oldB)
				arr->pointer[list++] = pb;
			j++;
		}
		else
			break;
		
		oldB = ba && bb;
	}
	
	return arr;
}

int countOr(bitstream const* a, bitstream const* b)
{
	// var list = [];
	int counter = 0;
	
	if (a->length == 0 && b->length == 0)
		// return list;
		return counter;
	if (a->length == 0)
	{
		return b->length;
	}
	if (b->length == 0)
	{
		return a->length;
	}
	
	int i = 0, j = 0;
	bool ba = false;
	bool bb = false;
	bool oldB = false;
	while (i < a->length || j < b->length)
	{
		int pa = a->pointer[i>=a->length?a->length-1:i];
		int pb = b->pointer[j>=b->length?b->length-1:j];
		
		if (pa == pb)
		{
			ba = !ba;
			bb = !bb;
			if ((ba || bb) != oldB)
				// list.push(pa);
				counter++;
			i++;
			j++;
		}
		else if ((pa < pb || j >= b->length) && i < a->length)
		{
			ba = !ba;
			if ((ba || bb) != oldB)
				// list.push(pa);
				counter++;
			i++;
		}
		else if (j < b->length)
		{
			bb = !bb;
			if ((ba || bb) != oldB)
				// list.push(pb);
				counter++;
			j++;
		}
		else
			break;
		
		oldB = ba || bb;
	}
	
	// return list;
	return counter;
}


bitstream* bitstream_Or(gcstack* gc, bitstream const* a, bitstream const* b)
{
	macro_err(a == NULL); macro_err(b == NULL);
	
	int count = countOr(a,b);
	
	bitstream* list = bitstream_InitWithSize
	(bitstream_AllocWithGC(gc), count);
	
	int counter = 0;
	
	if (a->length == 0 && b->length == 0)
		return list;
	if (a->length == 0)
	{
		for (int k = 0; k < b->length; k++)
			list->pointer[counter++] = b->pointer[k];
		return list;
	}
	if (b->length == 0)
	{
		for (int k = 0; k < a->length; k++)
			list->pointer[counter++] = a->pointer[k];
		return list;
	}
	
	int i = 0, j = 0;
	bool ba = false;
	bool bb = false;
	bool oldB = false;
	while (i < a->length || j < b->length)
	{
		int pa = a->pointer[i>=a->length?a->length-1:i];
		int pb = b->pointer[j>=b->length?b->length-1:j];
		
		if (pa == pb)
		{
			ba = !ba;
			bb = !bb;
			if ((ba || bb) != oldB)
				list->pointer[counter++] = pa;
			i++;
			j++;
		}
		else if ((pa < pb || j >= b->length) && i < a->length)
		{
			ba = !ba;
			if ((ba || bb) != oldB)
				list->pointer[counter++] = pa;
			i++;
		}
		else if (j < b->length)
		{
			bb = !bb;
			if ((ba || bb) != oldB)
				list->pointer[counter++] = pb;
			j++;
		}
		else
			break;
		
		oldB = ba || bb;
	}
	
	return list;
}

int countInvert(bitstream* a, int inv)
{
	// var res = [];
	int resCount = 0;
	
	bool added = false;
	for (int i = 0; i < a->length; i++)
	{
		if (a->pointer[i] == inv)
		{
			added = true;
			continue;
		}
		if (a->pointer[i] > inv && !added)
		{
			// res.push(inv);
			resCount++;
			added = true;
		}
		// res.push(a[i]);
		resCount++;
	}
	if (!added)
		// res.push(inv);
		resCount++;
	
	// return res;
	return resCount;
}

bitstream* bitstream_Invert(gcstack* gc, bitstream* a, int inv)
{
	macro_err(a == NULL);
	
	bitstream* res = bitstream_InitWithSize
	(bitstream_AllocWithGC(gc), countInvert(a, inv));
	
	int resCount = 0;
	
	bool added = false;
	for (int i = 0; i < a->length; i++)
	{
		if (a->pointer[i] == inv)
		{
			added = true;
			continue;
		}
		if (a->pointer[i] > inv && !added)
		{
			res->pointer[resCount++] = inv;
			added = true;
		}
		res->pointer[resCount++] = a->pointer[i];
	}
	if (!added)
	{
		res->pointer[resCount] = inv;
	}
	
	return res;
}

int countExcept(bitstream const* a, bitstream const* b)
{
	int list = 0;
	
	int a_length = a->length;
	int b_length = b->length;
	if (a_length == 0 || b_length == 0)
		return list;
	
	int i = 0, j = 0;
	bool ba = false;
	bool bb = true;
	bool oldB = false;
	int pa;
	int pb;
	while (i < a_length || j < b_length)
	{
		pa = a->pointer[i>=a_length?a_length-1:i];
		pb = b->pointer[j>=b_length?b_length-1:j];
		
		if (pa == pb)
		{
			ba = !ba;
			bb = !bb;
			if ((ba && bb) != oldB)
				list++;
			i++;
			j++;
		}
		else if ((pa < pb || j >= b_length) && i < a_length)
		{
			ba = !ba;
			if ((ba && bb) != oldB)
				list++;
			i++;
		}
		else if (j < b_length)
		{
			bb = !bb;
			if ((ba && bb) != oldB)
				list++;
			j++;
		}
		else
			break;
		
		oldB = ba && bb;
	}
	
	return list;
}


bitstream* bitstream_Except(gcstack* gc, bitstream const* a, bitstream const* b)
{
	macro_err(a == NULL); macro_err(b == NULL);
	
	int a_length = a->length;
	int b_length = b->length;
	if (b_length == 0)
		return bitstream_Clone(gc, a);
	
	int list = 0;
	
	bitstream* arr = bitstream_InitWithSize
	(bitstream_AllocWithGC(gc), countExcept(a, b));
	
	if (a_length == 0 || b_length == 0)
		return arr;
	
	int i = 0, j = 0;
	bool ba = false;
	bool bb = true;
	bool oldB = false;
	int pa;
	int pb;
	while (i < a_length || j < b_length)
	{
		pa = a->pointer[i>=a_length?a_length-1:i];
		pb = b->pointer[j>=b_length?b_length-1:j];
		
		if (pa == pb)
		{
			ba = !ba;
			bb = !bb;
			if ((ba && bb) != oldB)
			{
				arr->pointer[list++] = pa;
			}
			i++;
			j++;
		}
		else if ((pa < pb || j >= b_length) && i < a_length)
		{
			ba = !ba;
			if ((ba && bb) != oldB)
				arr->pointer[list++] = pa;
			i++;
		}
		else if (j < b_length)
		{
			bb = !bb;
			if ((ba && bb) != oldB)
				arr->pointer[list++] = pb;
			j++;
		}
		else
			break;
		
		oldB = ba && bb;
	}
	
	return arr;
}


int bitstream_Size(bitstream const* list)
{
	macro_err(list == NULL);
	
	int listCount = list->length;
	int sum = 0;
	for (int i = 0; i < listCount; i+=2)
	{
		sum += list->pointer[i+1]-list->pointer[i];
	}
	return sum;
}

int bitstream_AbsSub(bitstream const* list)
{
	// If it contains an odd number of elements,
	// the absolute value of the list is infinite.
	// Then we need to return -1 to tell it's infinite.
	int listCount = list->length;
	if ((listCount%2) != 0) 
	{
		// If the bitstream is inverted, then
		// return a negative amount as if the
		// bitstream was not inverted.
		// This means whatever the size of list is,
		// the size of this vector is such and such
		// less than the size.
		int negSum = list->pointer[0];
		for (int i = 1; i < listCount; i += 2)
		{
			negSum += list->pointer[i+1]-list->pointer[i];
		}
		return -negSum;
	}
	
	int sum = 0;
	for (int i = 0; i < listCount; i+=2)
	{
		sum += list->pointer[i+1]-list->pointer[i];
	}
	return sum;
}

int bitstream_Abs(bitstream const* list, int maximum)
{
	macro_err(list == NULL);
	
	if (list->length == 0)
		return 0;
	int abs = bitstream_AbsSub(list);
	if (abs <= 0)
		return maximum+abs;
	return abs;
}

int* bitstream_ArrayPointer(bitstream const* a)
{
	macro_err(a == NULL);
	
	return a->pointer;
}

int bitstream_NumberOfBlocks(bitstream const* a)
{
	macro_err(a == NULL);
	
	return a->length/2;
}

int bitstream_PopStart(bitstream* a)
{
	macro_err(a == NULL);
	
	int length = a->length;
	if (length < 2) return -1;
	
	// Move the start of the first block.
	int id = ++a->pointer[0];
	
	// If the start crosses the end, then remove the block.
	if (a->pointer[0] >= a->pointer[1])
	{
		// Instead of allocating we move the bytes 2 places toward beginning.
		for (int i = 2; i < length; i++)
			a->pointer[i-2] = a->pointer[i];
		a->length -= 2;
	}
	
	return id;
}

int bitstream_PopEnd(bitstream* a)
{
	macro_err(a == NULL);
	
	int length = a->length;
	if (length < 2) return -1;
	
	// Move the end.
	int id = --a->pointer[length-1];
	
	// If the start crosses the end, then remove the block.
	if (a->pointer[length-1] <= a->pointer[length-2])
		a->length -= 2;
	
	return id;
}

char** bitstream_GetWords(bitstream* a, string text)
{
	macro_err(a == NULL);
	
	if (text == NULL) return NULL;
	
	int length = a->length/2;
	char** arr = malloc(sizeof(char*)*length);
	
	int start, end;
	int wordLength;
	char* str;
	int i, j;
	for (i = 0; i < length; i++) {
		start = a->pointer[i*2];
		end = a->pointer[i*2+1];
		wordLength = end - start;
		
		str = malloc((wordLength+1)*sizeof(char));
		for (j = 0; j < wordLength; j++)
			str[j] = text[start+j];
		str[wordLength] = '\0';
		
		arr[i] = str;
	}
	
	return arr;
}

