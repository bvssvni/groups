/*
 *  groups.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
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
#include <string.h>

#include "gcstack.h"
#include "bitstream.h"
#include "sorting.h"
#include "property.h"
#include "readability.h"

#include "groups.h"

/***************COPY AND PUT ON SLASHES TO THIS LINE...********there ->***/
 
 #define foreach_reverse(a) \
 int __start##a, __end##a, __i##a, __j##a; \
 for (__i##a = a->length-2; __i##a >= 0; __i##a -= 2) { \
 __start##a = a->pointer[__i##a]; \
 __end##a = a->pointer[__i##a+1]; \
 for (__j##a = __end##a-1; __j##a >= __start##a; __j##a--) {
 
 #define foreach(a) \
 int __len##a = a->length-1; \
 int __start##a, __end##a, __i##a, __j##a; \
 for (__i##a = 0; __i##a < __len##a; __i##a += 2) { \
 __start##a = a->pointer[__i##a]; \
 __end##a = a->pointer[__i##a+1]; \
 for (__j##a = __start##a; __j##a < __end##a; __j##a++) {
 
 #define end_foreach(a) }}__BREAK_BITSTREAM_##a:;
 
 #define _break(a)     goto __BREAK_BITSTREAM_##a
 
 #define _pos(a)    __j##a


#define hashTable_foreach(a) gcstack_item* __cursor##a = a->layers->root->next; \
hash_layer* __layer##a; \
int* __indices##a; \
int __n##a, __i##a; \
for (; __cursor##a != NULL; __cursor##a = __cursor##a->next) { \
__layer##a = (hash_layer*)__cursor##a; \
__indices##a = __layer##a->indices; \
__n##a = __layer##a->n; \
for (__i##a = 0; __i##a < __n##a; __i##a++) { \
if (__indices##a[__i##a] == -1) continue;

#define _hashTable_id(a) __indices##a[__i##a]
#define _hashTable_value(a) __layer##a->data[__i##a]
#define _hashTable_double(a) *(double*)__layer##a->data[__i##a]
#define _hashTable_int(a) *(int*)__layer##a->data[__i##a]
#define _hashTable_bool(a) *(bool*)__layer##a->data[__i##a]
#define _hashTable_string(a) (char*)__layer##a->data[__i##a]

 
/****<- and there*****...TO KNOW START AND END. GOOD BOY!*****************/

void groups_Delete(void* p)
{
	groups* g = (groups*)p;
	
	// Free bitstream stuff.
	gcstack* bitstreams = g->bitstreams;
	if (bitstreams != NULL)
	{
		gcstack_Delete(bitstreams);
		free(bitstreams);
		g->bitstreams = NULL;
	}
	if (g->m_bitstreamsArray != NULL)
	{
		free(g->m_bitstreamsArray);
		g->m_bitstreamsArray = NULL;
	}
	if (g->m_deletedBitstreams != NULL)
	{
		free(g->m_deletedBitstreams);
		g->m_deletedBitstreams = NULL;
	}
	
	// Free property stuff.
	gcstack* properties = g->properties;
	if (properties != NULL)
	{
		gcstack_Delete(properties);
		free(properties);
		g->properties = NULL;
	}
	if (g->m_sortedPropertyItems != NULL)
	{
		free(g->m_sortedPropertyItems);
		g->m_sortedPropertyItems = NULL;
	}
	
	// Free member data stuff.
	gcstack* members = g->members;
	if (members != NULL)
	{
		gcstack_Delete(members);
		free(members);
		g->members = NULL;
	}
	if (g->m_memberArray != NULL)
	{
		free(g->m_memberArray);
		g->m_memberArray = NULL;
	}
	
	// Free the bitstream that contains deleted member indices.
	if (g->m_deletedMembers != NULL)
	{
		bitstream_Delete(g->m_deletedMembers);
		
		free(g->m_deletedMembers);
	}
}

groups* groups_AllocWithGC(gcstack* gc)
{
	return (groups*)gcstack_malloc(gc, sizeof(groups), groups_Delete);
}

groups* groups_Init(groups* g)
{
	g->bitstreams = gcstack_Init(gcstack_Alloc());
	g->m_bitstreamsReady = false;
	g->m_bitstreamsArray = NULL;
	g->m_deletedBitstreams = bitstream_InitWithSize(bitstream_AllocWithGC(NULL), 0);
	
	g->properties = gcstack_Init(gcstack_Alloc());
	g->m_propertiesReady = false;
	g->m_sortedPropertyItems = NULL;
	
	g->members = gcstack_Init(gcstack_Alloc());
	g->m_membersReady = false;
	g->m_memberArray = NULL;
	g->m_deletedMembers = bitstream_InitWithSize(bitstream_AllocWithGC(NULL), 0);
	return g;
}


//
// The first argument is string.
// The second argument is property.
//
int compareStringVSProperty(const void* a, const void* b)
{
	char const* str = (char const*)a;
	property const* prop = (property const*)b;
	return strcmp(str, prop->name);
}

int compareProperties(void const* a, void const* b)
{
	property const* aProp = (property const*)a;
	property const* bProp = (property const*)b;
	char const* aName = aProp->name;
	char const* bName = bProp->name;
	if (aName == NULL && bName == NULL) return 0;
	else if (aName == NULL) return -1;
	else if (bName == NULL) return 1;
	return strcmp(aName, bName);
}


void sortProperties(groups* g)
{
	if (g->m_propertiesReady) return;
	
	int length = g->properties->length;
	
	if (length == 0)
	{
		if (g->m_sortedPropertyItems != NULL)
		{
			free(g->m_sortedPropertyItems);
			g->m_sortedPropertyItems = NULL;
		}
		return;
	}
	
	gcstack_item** items;
	items = gcstack_CreateItemsArrayBackward(g->properties);
	property* t = malloc(sizeof(property));
	
	// Use QuickSort to sort the properties in right order.
	sorting_Sort((void*)items, 0, length, sizeof(property), t, (void*)compareProperties);
	
	free(t);
	if (g->m_sortedPropertyItems != NULL)
		free(g->m_sortedPropertyItems);
	g->m_sortedPropertyItems = items;
	
	g->m_propertiesReady = true;
}

void createBitstreamArray(groups* g)
{
	if (g->m_bitstreamsReady) return;
	
	// Create array of pointers to each bitstream to match the stack.
	if (g->m_bitstreamsArray != NULL)
		free(g->m_bitstreamsArray);
	g->m_bitstreamsArray = (bitstream**)gcstack_CreateItemsArrayBackward(g->bitstreams);
	g->m_bitstreamsReady = true;
}

int groups_AddProperty(groups* g, const void* name, const void* propType)
{
	// We use the length of the bitstream stack to generate ids,
	// because those bitstreams are set to empty instead of deleted.
	int propIndex = g->bitstreams->length;
	
	// Here we do a binary search trick to avoid some calls to strcmp.
	// You just build the code like a search tree and it will run faster.
	//
	// bool (node
	// double (node)
	// int <- root
	// string (node)
	//
	int propId = propIndex;
	if (strcmp(propType, "int") == 0)
		propId += TYPE_INT*TYPE_STRIDE;
	else if (strcmp(propType, "bool") == 0)
		propId += TYPE_BOOL*TYPE_STRIDE;
	else if (strcmp(propType, "double") == 0)
		propId += TYPE_DOUBLE*TYPE_STRIDE;
	else if (strcmp(propType, "string") == 0)
		propId += TYPE_STRING*TYPE_STRIDE;
	else
		propId += TYPE_UNKNOWN*TYPE_STRIDE;
	
	// It might seem like a waste to sort before inserting,
	// but we need to check if the property already exists.
	if (g->properties->length > 0)
	{
		sortProperties(g);
		int length = g->bitstreams->length;
		gcstack_item** items = g->m_sortedPropertyItems;
		int index = sorting_SearchBinary
		(length, (void*)items, name, compareStringVSProperty);
		
		if (index >= 0)
		{
			property* prop = (property*)items[index];
			int existingPropId = prop->propId;
			
			int oldType = existingPropId/TYPE_STRIDE;
			
			// We can not be sure how to check for compatibility with
			// unknown data types, so we have to tell it's type collision.
			if (oldType == TYPE_UNKNOWN) return -1;
			
			int newType = propId/TYPE_STRIDE;
			if (oldType == newType)
				// The same name and same type already exists.
				// Since it is compatible, we can return the existing id.
				return existingPropId;
			
			// The types are different, so we need to tell it is type collision here.
			return -1;
		}
	}
	
	
	if (g->m_deletedBitstreams->length > 0)
	{
		// Reuse deleted bitstream.
		createBitstreamArray(g);
		propIndex = bitstream_PopEnd(g->m_deletedBitstreams);
		propId = (propId-propId%TYPE_STRIDE) + propIndex;
		bitstream_InitWithSize(g->m_bitstreamsArray[propIndex], 0);
	}
	else
		// Create a new empty bitstream for that property.
		bitstream_InitWithSize(bitstream_AllocWithGC(g->bitstreams), 0);
	
	// Create new property that links name to id.
	property_InitWithNameAndId
	(property_AllocWithGC(g->properties), name, propId);
	
	
	g->m_propertiesReady = false;
	g->m_bitstreamsReady = false;
	
	return propId;
}


int groups_GetProperty(groups* g, char const* name)
{
	int length = g->properties->length;
	if (length == 0) return -1;
	
	sortProperties(g);
	gcstack_item** items = g->m_sortedPropertyItems;
	
	int index = sorting_SearchBinary(length, (void*)items, name, compareStringVSProperty);
	
	if (index < 0) return -1;
	
	// Return the property id.
	property* prop = (property*)items[index];
	return prop->propId;
}

bitstream* groups_GetBitstream(groups* g, int propId)
{
	// Filter out the type information.
	propId %= TYPE_STRIDE;
	
	createBitstreamArray(g);
	
	// Even when the bitstream is deleted, we can return this
	// safely because it is not removed from the stack and got length 0.
	// It means it will function as an empty set.
	return g->m_bitstreamsArray[propId];
}

void groups_RemoveProperty(groups* g, int propId)
{
	int index = propId % TYPE_STRIDE;
	
	// Delete content, but do not move from stack of bitstreams.
	createBitstreamArray(g);
	bitstream* a = g->m_bitstreamsArray[index];
	bitstream_Delete(a);
	
	// Add the property id to deleted bitstream for reuse.
	bitstream* b = bitstream_InitWithValues
	(bitstream_AllocWithGC(NULL), 2, (int[]){index, index+1});
	bitstream* c = g->m_deletedBitstreams;
	bitstream* d = bitstream_Or(NULL, c, b);
	gcstack_Swap(d, c);
	g->m_deletedBitstreams = d;
	bitstream_Delete(c);
	free(c);
	bitstream_Delete(b);
	free(b);
	
	// Loop through the stack to find the property to delete.
	gcstack_item* cursor = g->properties->root->next;
	property* prop = NULL;
	for (; cursor != NULL; cursor = cursor->next)
	{
		prop = (property*)cursor;
		if (prop->propId == propId) break;
	}
	
	if (prop == NULL) return;
	
	// Delete it, including freeing the pointer.
	gcstack_free(g->properties, prop);
	
	g->m_propertiesReady = false;
}

bool groups_IsDefaultVariable(const variable* var)
{
	int type = var->propId/TYPE_STRIDE;
	if (type == TYPE_DOUBLE) return false;
	else if (type == TYPE_INT)
	{
		int* val = (int*)var->data;
		if (*val == -1) return true;
	}
	else if (type == TYPE_BOOL)
	{
		bool* val = (bool*)var->data;
		if (*val == 0) return true;
	}
	else if (type == TYPE_STRING)
	{
		char* val = (char*)var->data;
		if (val == NULL) return true;
	}
	
	if (var->data == NULL) return true;
	return false;
}

void createMemberArray(groups* g)
{
	if (g->m_membersReady) return;
	
	hash_table** items = (hash_table**)gcstack_CreateItemsArrayBackward(g->members);
	if (g->m_memberArray != NULL)
		free(g->m_memberArray);
	g->m_memberArray = items;
	
	g->m_membersReady = true;
}

int groups_AddMember(groups* g, hash_table* obj)
{
	int id = g->members->length;
	hash_table* new;
    
    // Make sure it contains no id.
    hashTable_Set(obj, 0, NULL);
    
	if (g->m_deletedMembers->length > 0)
	{
        // Reuse an existing position.
		createMemberArray(g);
		id = bitstream_PopEnd(g->m_deletedMembers);
		new = hashTable_InitWithMember(g->m_memberArray[id], obj);
	}
	else
	{
		// There is no free positions, so we allocate new.
		new = hashTable_InitWithMember(hashTable_AllocWithGC(g->members), obj);
	}
	
	// Reinitialize the input so one can continue using same object to insert data.
	hashTable_Init(obj);
    
	// Prepare bitstreams to be searched.
	createBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
    int propId;
    int index;
    
    bitstream* a;
	bitstream* b;
	bitstream* c;
	b = bitstream_InitWithValues
	(bitstream_AllocWithGC(gc), 2, (const int[]){id, id+1});
    
    hashTable_foreach(new) {
        propId = _hashTable_id(new);
        index = propId%TYPE_STRIDE;
        
        a = g->m_bitstreamsArray[index];
        if (a == NULL) continue;
        
		c = bitstream_Or(gc, a, b);
		// Switch stacks so the new one is kept.
		gcstack_Swap(c, a);
    } end_foreach(new)
	
	gcstack_Delete(gc);
	free(gc); 
	
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	return id;
}

//
// This method sets all variables within a bitstream to a value.
//
void groups_SetDouble(groups* g, const bitstream* a, int propId, double val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
        hashTable_SetDouble(obj, propId, val);
	} end_foreach(a)
	
	// Double does not have a default value, so we need no condition here.
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	bitstream* c = bitstream_Or(NULL, b, a);
	gcstack_Swap(c, b);
	bitstream_Delete(b);
	free(b);
}

//
// This method sets all variables within a bitstream to a value.
//
void groups_SetString(groups* g, const bitstream* a, int propId, const char* val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
        hashTable_SetString(obj, propId, val);
	} end_foreach(a)
	
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	// If the string is NULL, then it is a default value
	// and we subtract from the bitstream instead of adding.
	bitstream* c = val == NULL ?
	bitstream_Except(NULL, b, a) :
	bitstream_Or(NULL, b, a);
	
	gcstack_Swap(c, b);
	bitstream_Delete(b);
	free(b);
}

void groups_SetInt(groups* g, const bitstream* a, int propId, int val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	bool isDefault = -1 == val;
	int i;
	hash_table* obj;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetInt(obj, propId, val);
	} end_foreach(a)
	
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	// If the string is NULL, then it is a default value
	// and we subtract from the bitstream instead of adding.
	bitstream* c = isDefault ?
	bitstream_Except(NULL, b, a) :
	bitstream_Or(NULL, b, a);
	
	gcstack_Swap(c, b);
	bitstream_Delete(b);
	free(b);
}

void groups_SetBool(groups* g, const bitstream* a, int propId, bool val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	bool isDefault = 0 == val;
	int i;
    hash_table* obj;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetBool(obj, propId, val);
	} end_foreach(a)
	
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	// If the string is NULL, then it is a default value
	// and we subtract from the bitstream instead of adding.
	bitstream* c = isDefault ?
	bitstream_Except(NULL, b, a) :
	bitstream_Or(NULL, b, a);
	
	gcstack_Swap(c, b);
	bitstream_Delete(b);
	free(b);
}


//
// This method sets variables to an array of double values.
// It is assumed that members are in the order you got them through the bitstream.
//
void groups_SetDoubleArray(groups* g, const bitstream* a, int propId, int n, const double* values)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// We need a counter to read the right value from the array.
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetDouble(obj, propId, values[k++]);
	} end_foreach(a)
	
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	bitstream* c = bitstream_Or(NULL, b, a);
	gcstack_Swap(c, b);
	bitstream_Delete(b);
	free(b);
}

void groups_SetStringArray
(groups* g, const bitstream* a, int propId, int n, const char** values)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* notDefaultIndices = malloc(n*sizeof(int));
	int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetString(obj, propId, values[k++]);
	} end_foreach(a)
	
	createBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];

	bitstream* notDef = bitstream_InitWithIndices
	(bitstream_AllocWithGC(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	bitstream* isDef = bitstream_Except(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	bitstream* c = bitstream_Except(gc, bitstream_Or(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}



void groups_SetIntArray
(groups* g, const bitstream* a, int propId, int n, const int* values)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* notDefaultIndices = malloc(n*sizeof(int));
	int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetInt(obj, propId, values[k++]);
	} end_foreach(a)
	
	createBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	bitstream* notDef = bitstream_InitWithIndices
	(bitstream_AllocWithGC(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	bitstream* isDef = bitstream_Except(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	bitstream* c = bitstream_Except(gc, bitstream_Or(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}

void groups_SetBoolArray
(groups* g, const bitstream* a, int propId, int n, const bool* values)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* notDefaultIndices = malloc(n*sizeof(int));
	int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetBool(obj, propId, values[k++]);
	} end_foreach(a)
	
	createBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	bitstream* notDef = bitstream_InitWithIndices
	(bitstream_AllocWithGC(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	bitstream* isDef = bitstream_Except(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	bitstream* c = bitstream_Except(gc, bitstream_Or(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}


double* groups_GetDoubleArray
(groups* g, const bitstream* a, int propId)
{
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	double* arr = malloc(sizeof(double)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
        arr[k++] = *((double*)hashTable_Get(obj, propId));
	} end_foreach(a)
	
	return arr;
}


int* groups_GetIntArray
(groups* g, const bitstream* a, int propId)
{
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	int* arr = malloc(sizeof(int)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
        arr[k++] = *((int*)hashTable_Get(obj, propId));
	} end_foreach(a)
	
	return arr;
}

bool* groups_GetBoolArray
(groups* g, const bitstream* a, int propId)
{
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	bool* arr = malloc(sizeof(bool)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
        arr[k++] = *((bool*)hashTable_Get(obj, propId));
	} end_foreach(a)
	
	return arr;
}

const char** groups_GetStringArray
(groups* g, const bitstream* a, int propId)
{
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	const char** arr = malloc(sizeof(string)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
        arr[k++] = (const char*)hashTable_Get(obj, propId);
	} end_foreach(a)
	
	return arr;
}

const char* groups_PropertyNameById(const groups* g, int propId)
{
	property* prop;
	gcstack_item* cursor = g->properties->root->next;
	for (; cursor != NULL; cursor = cursor->next)
	{
		prop = (property*)cursor;
		if (prop->propId == propId) return prop->name;
	}
	return NULL;
}

void groups_PrintMember(const groups* g, const hash_table* obj)
{
    int propId, type;
    
    hashTable_foreach(obj) {
            propId = _hashTable_id(obj);
            type = propId/TYPE_STRIDE;
            const char* name = groups_PropertyNameById(g, propId);
            if (type == TYPE_DOUBLE)
                printf("%s:%lg ", name, _hashTable_double(obj));
            else if (type == TYPE_INT)
                printf("%s:%i ", name, _hashTable_int(obj));
            else if (type == TYPE_BOOL)
                printf("%s:%i ", name, _hashTable_bool(obj));
            else if (type == TYPE_STRING)
                printf("%s:%s ", name, _hashTable_string(obj));
    } end_foreach(obj)
    printf("\r\n");
}

void groups_RemoveMember(groups* g, int index)
{
	createMemberArray(g);
	
	hash_table* obj = g->m_memberArray[index];
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	int propId;
	bitstream* a;
	bitstream* b = bitstream_InitWithValues
	(bitstream_AllocWithGC(gc), 2, (int[]){index,index+1});
	bitstream* c;
    hashTable_foreach(obj) {
        propId = _hashTable_id(obj);
        a = groups_GetBitstream(g, propId);
        if (a == NULL) continue;
        
		c = bitstream_Except(gc, a, b);
		gcstack_Swap(c, a);
    } end_foreach(obj)
    
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	// Free the member but don't delete it, in order to maintain index.
	hashTable_Delete(obj);
	
	// Add the member to bitstream of deleted members for reuse of index.
	bitstream* d = g->m_deletedMembers;
	bitstream* e = bitstream_Or(gc, d, b);
	gcstack_Swap(d, e);
	g->m_deletedMembers = e;
	
	gcstack_Delete(gc);
	
	free(gc);
}

void groups_RemoveMembers(groups* g, bitstream const* prop)
{
	createMemberArray(g);
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Remove the group from all bitstream properties.
	gcstack_item* cursor = g->bitstreams->root->next;
	bitstream* exProp;
	bitstream* tmpProp;
	for (; cursor != NULL; cursor = cursor->next)
	{
		exProp = (bitstream*)cursor;
		tmpProp = bitstream_Except(gc, exProp, prop);
		gcstack_Swap(tmpProp, exProp);
	}
	
	int index;
	hash_table* obj;
	foreach (prop) {
		// Free the member but don't delete it, in order to maintain index.
		index = _pos(prop);
		obj = g->m_memberArray[index];
		member_Delete(obj);
	} end_foreach (prop)
	
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	// Add the member to bitstream of deleted members for reuse of index.
	bitstream* d = g->m_deletedMembers;
	bitstream* e = bitstream_Or(gc, d, prop);
	gcstack_Swap(d, e);
	g->m_deletedMembers = e;
	
	gcstack_Delete(gc);
	
	free(gc);
}

bool groups_IsUnknown(int propId)
{
	return propId/TYPE_STRIDE == TYPE_UNKNOWN;
}

bool groups_IsDouble(int propId)
{
	return propId/TYPE_STRIDE == TYPE_DOUBLE;
}

bool groups_IsInt(int propId)
{
	return propId/TYPE_STRIDE == TYPE_INT;
}

bool groups_IsString(int propId)
{
	return propId/TYPE_STRIDE == TYPE_STRING;
}

bool groups_IsBool(int propId)
{
	return propId/TYPE_STRIDE == TYPE_BOOL;
}

void groups_PrintMemberToFile(FILE* f, const groups* g, const hash_table* obj)
{
    int propId, type;
    bool first = true;
    
    hashTable_foreach(obj) {
        propId = _hashTable_id(obj);
        type = propId/TYPE_STRIDE;
        
        // Write comma to separate the items in the document.
        if (!first) fprintf(f, ", ");
        first = false;
        
        const char* name = groups_PropertyNameById(g, propId);
        if (type == TYPE_DOUBLE)
            fprintf(f, "%s:%lg", name, _hashTable_double(obj));
        else if (type == TYPE_INT)
            fprintf(f, "%s:%i", name, _hashTable_int(obj));
        else if (type == TYPE_BOOL)
            fprintf(f, "%s:%i", name, _hashTable_bool(obj));
        else if (type == TYPE_STRING)
            fprintf(f, "%s:\"%s\"", name, _hashTable_string(obj));
    } end_foreach(obj)
}

void groups_PrintPropertyToFile(FILE* f, const groups* g, property* prop)
{
    string name = prop->name;
    int propId = prop->propId;
    int type = propId/TYPE_STRIDE;
    char* typeName;
    
    if (type == TYPE_DOUBLE)
        typeName = "double";
    else if (type == TYPE_INT)
        typeName = "int";
    else if (type == TYPE_BOOL)
        typeName = "bool";
    else if (type == TYPE_STRING)
        typeName = "string";
    fprintf(f, "%s:\"%s\"", name, typeName);
}

bool groups_SaveToFile(groups* g, string fileName)
{
    FILE* f = fopen(fileName, "w");
    if (f == NULL)
        return false;
    
    gcstack_item* cursor;
    
    // Print properties.
    sortProperties(g);
    
    cursor = g->properties->root->next;
    property* prop;
    fprintf(f, "properties {\r\n");
    for (; cursor != NULL; cursor = cursor->next)
    {
        prop = (property*)cursor;
        groups_PrintPropertyToFile(f, g, prop);
        if (cursor->next != NULL)
            fprintf(f, ",\r\n");
    }
    fprintf(f, " }\r\n");
    fprintf(f, "\r\n");
    
    // Print members.
    cursor = g->members->root->next;
    hash_table* member;
    int id = 0;
    int length = g->members->length;
    for (; cursor != NULL; cursor = cursor->next)
    {
        ++id;
        member = (hash_table*)cursor;
        if (member->layers != NULL)
        {
            fprintf(f, "member {");
            fprintf(f, "id:%i, ", length-id);
            groups_PrintMemberToFile(f, g, member);
            fprintf(f, "}\r\n");
        }
    }
    
    fclose(f);
    
    return true;
}

bool groups_ReadFromFile(groups* g, string fileName, bool verbose, void(*err)(int line, int column, const char* message))
{
    FILE* f = fopen(fileName, "r");
    if (f == NULL)
        return false;
    
    const int _skip_white_space = 0;
    const int _read_properties = 1;
    const int _read_start_paranthesis = 2;
    const int _error = 3;
    const int _read_name = 4;
    const int _read_colon = 5;
    const int _read_value = 6;
    const int _read_string = 7;
    const int _read_backslash_in_string = 8;
    const int _add = 9;
    const int _read_comma_or_end_paranthesis = 10;
    const int _read_member = 11;
    
    gcstack* gc = gcstack_Init(gcstack_Alloc());
    gcstack_PushInt(gc, _read_member);
    gcstack_PushInt(gc, _read_properties);
    gcstack_PushInt(gc, _skip_white_space);
    
    string propText = "properties";
    int propLength = strlen(propText);
    int propIndex = 0;
    
    string memberText = "member";
    int memberLength = strlen(memberText);
    int memberIndex = 0;
    
    hash_table* hs = hashTable_Init(hashTable_AllocWithGC(NULL));
    
    char* message;
    char* name = NULL;
    char* text = NULL;
    
    char nameBuffer[255];
    int nameBufferIndex = 0;
    
    gcstack* strStack = gcstack_Init(gcstack_Alloc());
    
    char* tag = NULL;
    bool isProperty, isMember, isId, isString;
    
    int ch;
    int line = 0;
    int column = 0;
    int state = gcstack_PopInt(gc);
    for (ch = fgetc(f); !feof(f); ch = fgetc(f)) {
        if (ch == '\n') line++;
        if (ch == '\n') column = 0; else column++;
    NEW_STATE:
        switch (state) {
            case _skip_white_space:
                if (ch == ' ' || ch == '\r' || ch == '\t' || ch == '\n') continue;
                if (verbose) printf("%i,%i: _skip_white_space\r\n", line, column);
                state = gcstack_PopInt(gc);
                goto NEW_STATE;
                break;
            case _error:
                // Error takes one argument from the stack.
                if (err != NULL)
                    err(line, column, message);
                else {
                    printf("%i,%i: %s\r\n", line, column, message);
                }
                goto CLEAN_UP;
                break;
            case _read_properties:
                // Checks character for character against expected keyword.
                if (ch == propText[propIndex++]) continue;
                if (propIndex < propLength) {
                    message = "Expected 'properties'";
                    state = _error;
                    goto NEW_STATE;
                }
                if (verbose) printf("%i,%i: _read_properties\r\n", line, column);
                tag = "properties";
                gcstack_PushInt(gc, _read_name);
                gcstack_PushInt(gc, _read_start_paranthesis);
                state = _skip_white_space;
                goto NEW_STATE;
                break;
            case _read_member:
                // Checks character for character against expected keyword.
                if (ch == memberText[memberIndex++]) continue;
                if (memberIndex < memberLength) {
                    message = "Expected 'member'";
                    state = _error;
                    goto NEW_STATE;
                }
                memberIndex = 0;
                if (verbose) printf("%i,%i: _read_member\r\n", line, column);
                tag = "member";
                gcstack_PushInt(gc, _read_name);
                gcstack_PushInt(gc, _read_start_paranthesis);
                state = _skip_white_space;
                goto NEW_STATE;
                break;
            case _read_start_paranthesis:
                // Reads start paranthesis.
                if (ch == '{') {
                    // Always skip white space after start paranthesis.
                    if (verbose) printf("%i,%i: _read_start_paranthesis\r\n", line, column);
                    state = _skip_white_space;
                    continue;
                }
                else {
                    message = "Expected '{'";
                    state = _error;
                    goto NEW_STATE;
                }
                break;
            case _read_colon:
                if (ch == ':') {
                    // Always skip white space after colon.
                    if (verbose) printf("%i,%i: _read_colon\r\n", line, column);
                    state = _skip_white_space;
                    continue;
                }
                else {
                    message = "Expected ':'";
                    state = _error;
                    goto NEW_STATE;
                }
                break;
            case _read_name:
                // Reads a name that contains only letters and alphanumeric characters.
                if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')) {
                    nameBuffer[nameBufferIndex++] = ch;
                    continue;
                }
                nameBuffer[nameBufferIndex++] = '\0';
                if (verbose) printf("%i,%i: _read_name %s\r\n", line, column, nameBuffer);
                if (name != NULL) free(name);
                name = malloc(nameBufferIndex*sizeof(char));
                strcpy(name, nameBuffer);
                nameBufferIndex = 0;
                gcstack_PushInt(gc, _read_value);
                gcstack_PushInt(gc, _read_colon);
                state = _skip_white_space;
                goto NEW_STATE;
                break;
            case _read_value:
                isString = (ch == '"');
                isProperty = strcmp(tag, "properties") == 0;
                isMember = strcmp(tag, "member") == 0;
                isId = strcmp(name, "id") == 0;
                
                if (isProperty && !isString) {
                    message = "Expected '\"'";
                    state = _error;
                    goto NEW_STATE;
                }
                
                if (isId && isProperty) {
                    message = "The 'id' keyword is reserved";
                    state = _error;
                    goto NEW_STATE;
                }
                
                if (verbose) printf("%i,%i: _read_value %s\r\n", line, column, name);
                
                if (isString && isProperty) {
                    state = _read_string;
                    continue;
                }
                if (isId) {
                    int id = 0;
                    column += fscanf(f, "%i", &id);
                    hashTable_SetInt(hs, 0, id);
                    if (verbose) printf("%i,%i: id %i\r\n", line, column, id);
                }
                else if (isMember) {
                    int propId = groups_GetProperty(g, name);
                    if (propId < 0) {
                        message = "Property not found";
                        state = _error;
                        goto NEW_STATE;
                    }
                    else {
                        // Read the types supported in this format.
                        // Steps one character back to read to get the first character when reading.
                        int type = propId/TYPE_STRIDE;
                        if (type == TYPE_INT) {
                            int val = 0;
                            fseek(f, -1, SEEK_CUR);
                            column += fscanf(f, "%i", &val);
                            hashTable_SetInt(hs, propId, val);
                            if (verbose) printf("%i,%i: %s:%i\r\n", line, column, name, val);
                        }
                        else if (type == TYPE_DOUBLE) {
                            double val = 0.0;
                            fseek(f, -1, SEEK_CUR);
                            column += fscanf(f, "%lg", &val);
                            hashTable_SetDouble(hs, propId, val);
                            if (verbose) printf("%i,%i: %s:%lg\r\n", line, column, name, val);
                        }
                        else if (type == TYPE_BOOL) {
                            bool val = 0;
                            fseek(f, -1, SEEK_CUR);
                            column += fscanf(f, "%i", &val);
                            hashTable_SetBool(hs, propId, val);
                        }
                        else if (type == TYPE_STRING) {
                            state = _read_string;
                            continue;
                        }
                    }
                }
                
                gcstack_PushInt(gc, _read_comma_or_end_paranthesis);
                state = _skip_white_space;
                continue;
            
            case _read_backslash_in_string:
                // Read special characters that are escaped by backspace.
                if (ch == '"')
                    gcstack_PushInt(strStack, ch);
                else if (ch == 'r')
                    gcstack_PushInt(strStack, '\r');
                else if (ch == 'n')
                    gcstack_PushInt(strStack, '\n');
                else if (ch == '\\')
                    gcstack_PushInt(strStack, '\\');
                else if (ch == '/')
                    gcstack_PushInt(strStack, '/');
                else if (ch == 'b')
                    gcstack_PushInt(strStack, '\b');
                else if (ch == 'f')
                    gcstack_PushInt(strStack, '\f');
                else if (ch == 't')
                    gcstack_PushInt(strStack, '\t');
                else if (ch == 'u') {
                    // A unicode letter consists of 4 bytes, but
                    // first we need to convert from hexadecimals to byte form.
                    // The least significant byte should be placed first,
                    // because this is the order the unicode letter is detected.
                    int unicode = 0;
                    fscanf(f, "%x", &unicode);
                    gcstack_PushInt(strStack, (unicode >> 24) & 0xFF);
                    gcstack_PushInt(strStack, (unicode >> 16) & 0xFF);
                    gcstack_PushInt(strStack, (unicode >> 8) & 0xFF);
                    gcstack_PushInt(strStack, unicode & 0xFF);
                    column += 4;
                }
                state = _read_string;
                continue;
                
            case _read_string:
                if (ch == '\\') {
                    state = _read_backslash_in_string;
                    continue;
                }
                if (ch != '"') {
                    gcstack_PushInt(strStack, ch);
                    continue;
                }
                if (text != NULL) free(text);
                text = malloc((strStack->length+1)*sizeof(char));
                text[strStack->length] = '\0';
                while (strStack->length > 0)
                    text[strStack->length] = gcstack_PopInt(strStack);
                if (verbose) printf("%i, %i: _read_string %s\r\n", line, column, text);
                state = _add;
                goto NEW_STATE;
                break;
                
            case _read_comma_or_end_paranthesis:
                if (ch == ',') {
                    if (verbose) printf("%i,%i: _read_comma\r\n", line, column);
                    gcstack_PushInt(gc, _read_name);
                    state = _skip_white_space;
                    continue;
                }
                else if (ch == '}') {
                    // Clear the stack and read a new member.
                    if (verbose) printf("%i,%i: _read_end_paranthesis\r\n", line, column);
                    
                    if (strcmp(tag, "member") == 0) {
                        // Add the member to Groups.
                        if (verbose) groups_PrintMember(g, hs);
                        groups_AddMember(g, hs);
                        if (verbose) printf("%i,%i: added member\r\n", line, column);
                    }
                        
                    while (gc->length > 0) {
                        gcstack_PopInt(gc);
                    }
                    gcstack_PushInt(gc, _read_member);
                    state = _skip_white_space;
                    continue;
                }
                message = "Expected ','";
                state = _error;
                goto NEW_STATE;
                break;
                
            case _add:
                if (strcmp(tag, "properties") == 0) {
                    groups_AddProperty(g, name, text);
                    if (verbose) printf("%i,%i: _add %s:%s\r\n", line, column, name, text);
                    gcstack_PushInt(gc, _read_comma_or_end_paranthesis);
                    state = _skip_white_space;
                    continue;
                }
                else if (strcmp(tag, "member") == 0) {
                    int propId = groups_GetProperty(g, name);
                    hashTable_SetString(hs, propId, text);
                    if (verbose) printf("%i,%i: _add %s:%s\r\n", line, column, name, text);
                    gcstack_PushInt(gc, _read_comma_or_end_paranthesis);
                    state = _skip_white_space;
                    continue;
                }
                break;
                
        }
        break;
    }
    
CLEAN_UP:
    hashTable_Delete(hs);
    free(hs);
    if (text != NULL) free(text);
    if (name != NULL) free(name);
    gcstack_Delete(strStack);
    free(strStack);
    gcstack_Delete(gc);
    free(gc);
    fclose(f);
    
    return true;
}

