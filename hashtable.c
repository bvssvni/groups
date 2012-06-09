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

#include "readability.h"

#include "hashtable.h"

#define START_PRIME 5

void hashLayer_Delete(void* p)
{
	hash_layer* hashLayer = (hash_layer*)p;
	int* indices = hashLayer->indices;
	if (indices != NULL) {
        void** data = hashLayer->data;
        int n = hashLayer->n;
        for (int i = 0; i < n; i++)
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

hash_layer* hashLayer_AllocWithGC(gcstack* gc)
{
	return (hash_layer*)gcstack_malloc(gc, sizeof(hash_layer), hashLayer_Delete);
}

hash_layer* hashLayer_InitWithSize(hash_layer* hashLayer, int n)
{
    if (hashLayer == NULL) {
        fprintf(stderr, "hashLayer_InitWithSize: hashLayer == NULL\r\n");
        pthread_exit(NULL);
    }
    if (n < 1) {
        fprintf(stderr, "hashLayer_InitWithSize: n < 1\r\n");
        pthread_exit(NULL);
    }
    
	hashLayer->n = n;
	
	hashLayer->indices = malloc(sizeof(int)*n);
    hashLayer->data = malloc(sizeof(void*)*n);
    
	int* indices = hashLayer->indices;
	for (int i = 0; i < n; i++)
		indices[i] = -1;
	return hashLayer;
}

int hashLayer_NextPrime(int prime)
{
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

void hashTable_Delete(void* p)
{
	hash_table* hash = (hash_table*)p;
    
    if (hash == NULL) {
        fprintf(stderr, "hashTable_Delete: hash == NULL\r\n");
        pthread_exit(NULL);
    }
    
	hash->m_lastPrime = 0;
	if (hash->layers != NULL) {
		gcstack_Delete(hash->layers);
		free(hash->layers);
		hash->layers = NULL;
	}
}

hash_table* hashTable_AllocWithGC(gcstack* gc)
{
    return (hash_table*)gcstack_malloc(gc, sizeof(hash_table), hashTable_Delete);
}

hash_table* hashTable_Init(hash_table* hash)
{
    if (hash == NULL) {
        fprintf(stderr, "hashTable_Init: hash == NULL\r\n");
        pthread_exit(NULL);
    }
    
	hash->layers = gcstack_Init(gcstack_Alloc());
	gcstack* layers = hash->layers;
	hash->m_lastPrime = START_PRIME;
	hashLayer_InitWithSize(hashLayer_AllocWithGC(layers), hash->m_lastPrime);
	return hash;
}

hash_table* hashTable_InitWithMember(hash_table* obj, hash_table* b)
{
    if (obj == NULL) {
        fprintf(stderr, "hashTable_InitWithMember: obj == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (b == NULL) {
        fprintf(stderr, "hashTable_InitWithMember: b == NULL\r\n");
        pthread_exit(NULL);
    }
    
    obj->layers = b->layers;
    obj->m_lastPrime = b->m_lastPrime;
    
    b->layers = NULL;
    b->m_lastPrime = 0;
    
	return obj;
}

void hashTable_Set(hash_table* hash, int id, void* value)
{
    if (hash == NULL) {
        fprintf(stderr, "hashTable_Set: hash == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (id < 0) {
        fprintf(stderr, "hashTable_Set: id < 0\r\n");
        pthread_exit(NULL);
    }
    
	gcstack_item* cursor = hash->layers->root->next;
	hash_layer* layer;
	int n;
	int pos;
	int exId;
	int* indices;
    int freePos = -1;
    hash_layer* freeLayer = NULL;
    
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
	int nextPrime = hashLayer_NextPrime(hash->m_lastPrime);
	hash_layer* newLayer = hashLayer_InitWithSize
	(hashLayer_AllocWithGC(hash->layers), nextPrime);
	hash->m_lastPrime = nextPrime;
    pos = id % nextPrime;
	newLayer->indices[pos] = id;
    newLayer->data[pos] = value;
}


unsigned long
hashStringId(const char *str)
{
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    
    return hash;
}

void hashTable_SetStringHash(hash_table* hash, char* value)
{
    if (hash == NULL) {
        fprintf(stderr, "hashTable_SetStringHash: hash == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (value == NULL) {
        fprintf(stderr, "hashTable_SetStringHash: value == NULL\r\n");
        pthread_exit(NULL);
    }
    
    int id = hashStringId(value);
    id = id < 0 ? -id : id;
	gcstack_item* cursor = hash->layers->root->next;
	hash_layer* layer;
	int n;
	int pos;
	int exId;
	int* indices;
    int freePos = -1;
    hash_layer* freeLayer = NULL;
    
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
    if (value == NULL) return;
	
    // Add in free layer.
    if (freePos != -1)
    {
        freeLayer->indices[freePos] = id;
        freeLayer->data[freePos] = value;
        return;
    }
    
	// Create new layer.
	int nextPrime = hashLayer_NextPrime(hash->m_lastPrime);
	hash_layer* newLayer = hashLayer_InitWithSize
	(hashLayer_AllocWithGC(hash->layers), nextPrime);
	hash->m_lastPrime = nextPrime;
    pos = id % nextPrime;
	newLayer->indices[pos] = id;
    newLayer->data[pos] = value;
}

const void* hashTable_Get(hash_table* hash, int id)
{
    if (hash == NULL) {
        fprintf(stderr, "hashTable_Get: hash == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (id < 0) {
        fprintf(stderr, "hashTable_Get: id < 0\r\n");
        pthread_exit(NULL);
    }
    
	gcstack_item* cursor = hash->layers->root->next;
	hash_layer* layer;
	int n;
	int pos;
	int exId;
	int* indices;
	for (; cursor != NULL; cursor = cursor->next) {
		layer = (hash_layer*)cursor;
		n = layer->n;
		pos = id % n;
		indices = layer->indices;
		exId = indices[pos];
        
		// Return if already set.
		if (exId == id) return layer->data[pos];
	}
    
	return NULL;
}

bool hashTable_ContainsStringHash(hash_table* hash, const char* value)
{
    if (hash == NULL) {
        fprintf(stderr, "hashTable_ContainsStringHash: hash == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (value == NULL) {
        fprintf(stderr, "hashTable_ContainsStringHash: value == NULL\r\n");
        pthread_exit(NULL);
    }
    
    int id = hashStringId(value);
    id = id < 0 ? -id : id;
	gcstack_item* cursor = hash->layers->root->next;
	hash_layer* layer;
	int n;
	int pos;
	int exId;
	int* indices;
	for (; cursor != NULL; cursor = cursor->next) {
		layer = (hash_layer*)cursor;
		n = layer->n;
		pos = id % n;
		indices = layer->indices;
		exId = indices[pos];
        
		// Return if already set.
		if (exId == id && strcmp(value, layer->data[pos]) == 0) return true;
	}
    
	return false;
}

void hashTable_SetDouble(hash_table* obj, int propId, double val)
{
    if (obj == NULL) {
        fprintf(stderr, "hashTable_SetDouble: obj == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "hashTable_SetDouble: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    double* p = malloc(sizeof(double));
    *p = val;
    hashTable_Set(obj, propId, p);
}

void hashTable_SetString(hash_table* obj, int propId, char const* val)
{
    if (obj == NULL) {
        fprintf(stderr, "hashTable_SetString: obj == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "hashTable_SetString: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (val == NULL)
    {
        hashTable_Set(obj, propId, NULL);
        return;
    }
    
    char* copy = malloc(strlen(val)*sizeof(char));
    strcpy(copy, val);
    hashTable_Set(obj, propId, copy);
}

void hashTable_SetInt(hash_table* obj, int propId, int val)
{
    if (obj == NULL) {
        fprintf(stderr, "hashTable_SetInt: obj == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "hashTable_SetInt: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (val == -1)
    {
        hashTable_Set(obj, propId, NULL);
        return;
    }
    
    int* p = malloc(sizeof(int));
    *p = val;
    hashTable_Set(obj, propId, p);
}

void hashTable_SetBool(hash_table* obj, int propId, bool val)
{
    if (obj == NULL) {
        fprintf(stderr, "hashTable_SetBool: obj == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "hashTable_SetBool: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (val == 0)
    {
        hashTable_Set(obj, propId, 0);
        return;
    }
    
    bool* p = malloc(sizeof(bool));
    *p = val;
    hashTable_Set(obj, propId, p);
}
