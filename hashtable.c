/*
 *  hashtable.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 27.05.12.
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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "gcstack.h"
#include "errorhandling.h"
#include "readability.h"

#define memgroups_hashtable_internal
#include "hashtable.h"

#define START_PRIME 5

void hashLayer_Delete(void* const p)
{
	macro_err_return(p == NULL);
	
	hash_layer* const hashLayer = (hash_layer* const)p;
	int* const indices = hashLayer->indices;
	if (indices != NULL) {
		void** data = hashLayer->data;
		int n = hashLayer->n;
		int i;
		for (i = 0; i < n; i++)
		{
			if (indices[i] == -1) continue;
			free(data[i]);
		}
		free(indices);
		hashLayer->indices = NULL;
		free(hashLayer->data);
		hashLayer->data = NULL;
		hashLayer->n = 0;
	}
}

hash_layer* hashLayer_GcAlloc(gcstack* const gc)
{
	return (hash_layer*)gcstack_malloc(gc, sizeof(hash_layer), 
					   hashLayer_Delete);
}

hash_layer* hashLayer_InitWithSize(hash_layer* const hashLayer, const int n)
{
	macro_err_return_null(hashLayer == NULL);
	macro_err_return_null(n < 1);
	
	hashLayer->indices = NULL;
	hashLayer->data = NULL;
	
	hashLayer->n = n;
	if (n == 0) 
		return hashLayer;
	
	hashLayer->indices = malloc(sizeof(int)*n);
	hashLayer->data = malloc(sizeof(void*)*n);
        
	int* indices = hashLayer->indices;
	int i;
	for (i = 0; i < n; i++)
		indices[i] = -1;
	return hashLayer;
}

int hashLayer_NextPrime(const int prime)
{
	macro_err(prime < 0);
	
	switch (prime) {
		case 2: return 3;
		case 3: return 5;
		case 5: return 7;
		case 7: return 11;
		case 11: return 13;
		case 13: return 17;
		case 17: return 19;
		case 19: return 23;
		case 23: return 29;
		case 29: return 31;
		case 31: return 33;
		case 33: return 37;
		case 37: return 41;
		case 41: return 43;
		case 43: return 47;
		case 47: return 53;
		case 53: return 59;
		case 59: return 61;
		case 61: return 67;
		case 67: return 71;
		case 71: return 73;
		case 73: return 79;
		case 79: return 83;
		case 83: return 89;
		case 89: return 97;
		case 97: return 101;
	}
	// Just expand with a static interval.
	return prime+257;
}

void hashTable_Delete(void* const p)
{
	macro_err_return(p == NULL);
	
	hash_table* const hash = (hash_table* const)p;
	
	hash->m_lastPrime = 0;
	if (hash->layers != NULL) {
		gcstack_Delete(hash->layers);
		free(hash->layers);
		hash->layers = NULL;
	}
}

hash_table* hashTable_GcAlloc(gcstack* const gc)
{
	return (hash_table*)gcstack_malloc(gc, sizeof(hash_table), 
					   hashTable_Delete);
}

hash_table* hashTable_Init(hash_table* const hash)
{
	macro_err_return_null(hash == NULL);
	
	hash->layers = gcstack_Init(gcstack_Alloc());
	gcstack* layers = hash->layers;
	hash->m_lastPrime = START_PRIME;
	hashLayer_InitWithSize(hashLayer_GcAlloc(layers), 
			       hash->m_lastPrime);
	return hash;
}

hash_table* hashTable_InitWithMember(hash_table* const obj, hash_table* const b)
{
	macro_err_return_null(obj == NULL);
	macro_err_return_null(b == NULL);
	
	obj->layers = b->layers;
	obj->m_lastPrime = b->m_lastPrime;
	
	b->layers = NULL;
	b->m_lastPrime = 0;
	
	return obj;
}

void hashTable_Set(hash_table* const hash, const int id, void* const value)
{
	macro_err_return(hash == NULL);
	macro_err_return(id < 0);
	
	const gcstack_item* cursor = hash->layers->root->next;
	const hash_layer* layer;
	const hash_layer* freeLayer = NULL;
	
	int n;
	int pos;
	int exId;
	int* indices;
	int freePos = -1;
	
	for (; cursor != NULL; cursor = cursor->next) {
		layer = (hash_layer*)cursor;
		
		n = layer->n;
		pos = id % n;
		indices = layer->indices;
		exId = indices[pos];
		
		if (exId == id && value != layer->data[pos])
		{
			// Replace the existing value.
			free(layer->data[pos]);
			indices[pos] = value == NULL ? -1 : id;
			layer->data[pos] = value;
			
			return;
		}
		if (exId == -1 && freePos == -1)
		{
			// Remember the first encountered free slot.
			freePos = pos;
			freeLayer = layer;
		}
	}
	
	// Don't put anything in if the value is NULL.
	// NULL is used to remove values from the hash table.
	if (value == NULL) return;
	
	// Add in free layer.
	if (freePos != -1)
	{
		freeLayer->indices[freePos] = id;
		freeLayer->data[freePos] = value;
		return;
	}
	
	// Create new layer.
	const int nextPrime = hashLayer_NextPrime(hash->m_lastPrime);
	hash_layer* newLayer = hashLayer_InitWithSize
	(hashLayer_GcAlloc(hash->layers), nextPrime);
	hash->m_lastPrime = nextPrime;
	pos = id % nextPrime;
	newLayer->indices[pos] = id;
	newLayer->data[pos] = value;
}


unsigned long
hashTable_GenerateHashId(const char * const text)
{
	const char* str = text;
	unsigned long hash = 5381;
	int c;
	
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	
	return hash;
}

void hashTable_SetStringHash(hash_table* const hash, char* const value)
{
	macro_err_return(hash == NULL);
	macro_err_return(value == NULL);
	
	int hashId = (int)hashTable_GenerateHashId(value);
	int id = hashId < 0 ? -hashId : hashId;
	const gcstack_item* cursor = hash->layers->root->next;
	const hash_layer* layer;
	const hash_layer* freeLayer = NULL;
	
	int n;
	int pos;
	int exId;
	int* indices;
	int freePos = -1;
	
	for (; cursor != NULL; cursor = cursor->next) {
		layer = (hash_layer*)cursor;
		
		n = layer->n;
		pos = id % n;
		indices = layer->indices;
		exId = indices[pos];
		
		if (exId == id && strcmp(value, layer->data[pos]) != 0)
		{
			// Replace the existing value.
			free(layer->data[pos]);
			indices[pos] = value == NULL ? -1 : id;
			layer->data[pos] = value;
			
			return;
		}
		if (exId == -1 && freePos == -1)
		{
			// Remember the first encountered free slot.
			freePos = pos;
			freeLayer = layer;
		}
	}
	
	// Don't put anything in if the value is NULL.
	// NULL is used to remove values from the hash table.
	if (value == NULL) 
		return;
	
	// Add in free layer.
	if (freePos != -1)
	{
		freeLayer->indices[freePos] = id;
		freeLayer->data[freePos] = value;
		return;
	}
	
	// Create new layer.
	const int nextPrime = hashLayer_NextPrime(hash->m_lastPrime);
	hash_layer* newLayer = hashLayer_InitWithSize
	(hashLayer_GcAlloc(hash->layers), nextPrime);
	hash->m_lastPrime = nextPrime;
	pos = id % nextPrime;
	newLayer->indices[pos] = id;
	newLayer->data[pos] = value;
}

const void* hashTable_Get(const hash_table* const hash, const int id)
{
	macro_err_return_null(hash == NULL);
	macro_err_return_null(id < 0);
	
	const gcstack_item* cursor = hash->layers->root->next;
	const hash_layer* layer;
	const int* indices;
	
	int n;
	int pos;
	int exId;
	for (; cursor != NULL; cursor = cursor->next) {
		layer = (hash_layer*)cursor;
		n = layer->n;
		pos = id % n;
		indices = layer->indices;
		exId = indices[pos];
		
		// Return if already set.
		if (exId == id) 
			return layer->data[pos];
	}
	
	return NULL;
}

bool hashTable_ContainsStringHash
(hash_table* const hash, const char* const value)
{
	macro_err_return_zero(hash == NULL);
	macro_err_return_zero(value == NULL);
	
	const int hashId = (int)hashTable_GenerateHashId(value);
	const int id = hashId < 0 ? -hashId : hashId;
	
	const gcstack_item* cursor = hash->layers->root->next;
	const hash_layer* layer;
	const int* indices;
	
	int n;
	int pos;
	int exId;
	for (; cursor != NULL; cursor = cursor->next) {
		layer = (hash_layer*)cursor;
		n = layer->n;
		pos = id % n;
		indices = layer->indices;
		exId = indices[pos];
		
		// Return if already set.
		if (exId == id && strcmp(value, layer->data[pos]) == 0) 
			return true;
	}
	
	return false;
}

void hashTable_SetDouble
(hash_table* const obj, const int propId, const double val)
{
	macro_err(obj == NULL);
	macro_err(propId < 0);
	
	double* const p = malloc(sizeof(double));
	*p = val;
	hashTable_Set(obj, propId, p);
}

void hashTable_SetString
(hash_table* const obj, const int propId, const char* const val)
{
	macro_err(obj == NULL);
	macro_err(propId < 0);
	
	if (val == NULL)
	{
		hashTable_Set(obj, propId, NULL);
		return;
	}
	
	char* const copy = malloc(strlen(val)*sizeof(char));
	strcpy(copy, val);
	hashTable_Set(obj, propId, copy);
}

void hashTable_SetInt(hash_table* const obj, const int propId, const int val)
{
	macro_err(obj == NULL);
	macro_err(propId < 0);
	
	if (val == -1)
	{
		hashTable_Set(obj, propId, NULL);
		return;
	}
	
	int* const p = malloc(sizeof(int));
	*p = val;
	hashTable_Set(obj, propId, p);
}

void hashTable_SetBool(hash_table* const obj, const int propId, const bool val)
{
	macro_err(obj == NULL);
	macro_err(propId < 0);
	
	if (val == 0)
	{
		hashTable_Set(obj, propId, 0);
		return;
	}
	
	bool* const p = malloc(sizeof(bool));
	*p = val;
	hashTable_Set(obj, propId, p);
}

