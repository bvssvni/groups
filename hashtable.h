/*
 *  hashtable.h
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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef memgroups_hashtable
#define memgroups_hashtable
	
#include <string.h>
    
#include "gcstack.h"
#include "variable.h"
	
	typedef struct hash_layer {
		gcstack_item gc;
		int n;
		int* indices;
        void** data;
	} hash_layer;
	
	typedef struct hash_table {
        gcstack_item gc;
		gcstack* layers;
		int m_lastPrime;
	} hash_table;
	
	void hashLayer_Delete(void* p);
	
	hash_layer* hashLayer_AllocWithGC(gcstack* gc);
	
	hash_layer* hashLayer_InitWithSize(hash_layer* hashLayer, int n);
	
	int hashLayer_NextPrime(int prime);
	
	void hashTable_Delete(void* p);
	
    
    
	hash_table* hashTable_AllocWithGC(gcstack* gc);
	
	hash_table* hashTable_Init(hash_table* hash);
	
    hash_table* hashTable_InitWithMember(hash_table* obj, hash_table* b);
    
	void hashTable_Set(hash_table* hash, int id, void* value);
	
    // Returns a pointer that can not be changed because it can only be freed
    // by the hash table and if you need to change it you have to copy it.
	const void*         hashTable_Get       
    (hash_table* hash, int id);
	
    void                hashTable_SetDouble 
    (hash_table* obj, int propId, double val);
    
    void                hashTable_SetString 
    (hash_table* obj, int propId, char const* val);
    
    void hashTable_SetInt
    (hash_table* obj, int propId, int val);
    
    void hashTable_SetBool
    (hash_table* obj, int propId, int val);
    
#endif
	
#ifdef __cplusplus
}
#endif
