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
	
#ifndef memgroups_hashtable_internal
#undef private
#define private const
#else
#undef private
#define private
#endif
	
	typedef struct hash_layer {
		private gcstack_item gc;
		private int n;
		private int* private indices;
		private void** private private data;
	} private hash_layer;
	
	typedef struct hash_table {
		private gcstack_item gc;
		private gcstack* private layers;
		private int m_lastPrime;
	} hash_table;
	
	//
	//      HASH LAYERS
	//
	//      The sub structures of a hash table.
	//
	void hashLayer_Delete(void* const p);
	
	hash_layer* hashLayer_AllocWithGC(gcstack* const gc);
	
	hash_layer* hashLayer_InitWithSize
	(hash_layer* const hashLayer, const int n);
	
	int hashLayer_NextPrime(const int prime);
	
	//
	//      HASH TABLE
	//
	//      The structure used for storing by property id or hash key.
	//
	void hashTable_Delete
	(void* const p);
	
	hash_table* hashTable_AllocWithGC
	(gcstack* const gc);
	
	hash_table* hashTable_Init
	(hash_table* const hash);
	
	hash_table* hashTable_InitWithMember
	(hash_table* const obj, hash_table* const b);
	
	
	//
	//      HASHING
	//
	//      Using hashing of strings, you can check for the existence of a 
	//	string very fast.
	//      This is a different way to use hash table than storing pointers 
	//	by id.
	//      It should not be mixed with other usages.
	//
	unsigned long hashTable_GenerateHashId
	(const char * const str);
	
	//
	//      Sets a string, the id is a hash value of the string.
	//      The string is used to check for per match.
	//      Don't mix with      hashTable_Set
	//
	void                hashTable_SetStringHash
	(hash_table* const hash, char* const value);
	
	//
	//      Returns true if the hash table contains a string.
	//      The values in the table are stored as string and it uses this 
	//	for perfect matching.
	//      Don't mix with       hashTable_Get
	//
	int                hashTable_ContainsStringHash
	(hash_table* const hash, const char* const value);
	
	//
	//      POINTER BY ID
	//
	//      Sets a pointer in the hash table by an id.
	//      If you use a double data type, use hashTable_SetDouble.
	//
	void hashTable_Set
	(hash_table* const hash, const int id, void* const value);
	
	//
	//      Returns a pointer that can not be changed because it can only be
	//	freed
	//      by the hash table and if you need to change it you have to copy 
	//	it.
	//
	const void* hashTable_Get       
	(hash_table* const hash, const int id);
	
	
	void hashTable_SetDouble 
	(hash_table* const obj, const int propId, const double val);
	
	void hashTable_SetString 
	(hash_table* const obj, const int propId, const char* val);
	
	void hashTable_SetInt
	(hash_table* const obj, const int propId, const int val);
	
	void hashTable_SetBool
	(hash_table* const obj, const int propId, const int val);
	
#endif
	
#ifdef __cplusplus
}
#endif
