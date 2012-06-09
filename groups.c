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
#include <sys/stat.h>
#include <pthread.h>

#include "gcstack.h"
#include "bitstream.h"
#include "sorting.h"
#include "property.h"

#include "readability.h"

#include "groups.h"

#define TMP_ID_PROPID 123

void groups_Delete(void* p)
{
	groups* g = (groups*)p;
    
    if (g == NULL) {
        fprintf(stderr, "groups_Delete: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_Init: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_AddProperty: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (name == NULL) {
        fprintf(stderr, "groups_AddProperty: name == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propType == NULL) {
        fprintf(stderr, "groups_AddPropety: propType == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_GetProperty: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (name == NULL) {
        fprintf(stderr, "groups_GetProperty: name == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_GetBitstream: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_GetBitstream: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_RemoveProperty: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_RemoveProperty: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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

bool groups_IsDefaultVariable(int propId, void* data)
{
    if (propId < 0) {
        fprintf(stderr, "groups_IsDefaultVariable: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (data == NULL) {
        fprintf(stderr, "groups_IsDefaultVariable: data == NULL\r\n");
        pthread_exit(NULL);
    }
    
	int type = propId/TYPE_STRIDE;
	if (type == TYPE_DOUBLE) return false;
	else if (type == TYPE_INT)
	{
		int* val = (int*)data;
		if (*val == -1) return true;
	}
	else if (type == TYPE_BOOL)
	{
		bool* val = (bool*)data;
		if (*val == 0) return true;
	}
	else if (type == TYPE_STRING)
	{
		char* val = (char*)data;
		if (val == NULL) return true;
	}
	
	if (data == NULL) return true;
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
    if (g == NULL) {
        fprintf(stderr, "groups_AddMember: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (obj == NULL) {
        fprintf(stderr, "groups_AddMember: obj == NULL\r\n");
        pthread_exit(NULL);
    }
    
	int id = g->members->length;
	hash_table* new;
    
    // When reading from file and id does not match,
    // add 'deleted' members to match up with the id.
    bool hasId = false;
    int* oldIdPtr = (int*)hashTable_Get(obj, TMP_ID_PROPID);
    int oldId = oldIdPtr == NULL ? id : *oldIdPtr;
    int newId = id;
    while (oldId > newId) {
        hash_table* hs = hashTable_AllocWithGC(g->members);
        hs->layers = NULL;
        hs->m_lastPrime = 0;
        newId++;
    }
    if (newId - id > 0)
    {
        // Update the indices of deleted members to include the 'fakes' that has been added.
        bitstream* addedIds = bitstream_InitWithValues(bitstream_AllocWithGC(NULL), 2, (int[]){id, newId});
        bitstream* newDeleted = bitstream_Or(NULL, g->m_deletedMembers, addedIds);
        bitstream* oldDeleted = g->m_deletedMembers;
        g->m_deletedMembers = newDeleted;
        bitstream_Delete(addedIds);
        free(addedIds);
        bitstream_Delete(oldDeleted);
        free(oldDeleted);
        hasId = true;
    }
    
    id = newId;
    
    // Make sure it contains no id.
    hashTable_Set(obj, TMP_ID_PROPID, NULL);
    
    // If the member came with fixed id, then don't reuse an old one.
	if (!hasId && g->m_deletedMembers->length > 0)
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetDouble: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetDouble: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetDouble: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetString: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetString: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetString: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetInt: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetInt: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetInt: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetBool: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetBool: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetBool: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetDoubleArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetDoubleArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetDoubleArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (n < 0) {
        fprintf(stderr, "groups_SetDoubleArray: n < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (values == NULL) {
        fprintf(stderr, "groups_SetDoubleArray: values == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetStringArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetStringArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetStringArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (n < 0) {
        fprintf(stderr, "groups_SetStringArray: n < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (values == NULL) {
        fprintf(stderr, "groups_SetStringArray: values == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetIntArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetIntArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetIntArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (n < 0) {
        fprintf(stderr, "groups_SetIntArray: n < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (values == NULL) {
        fprintf(stderr, "groups_SetIntArray: values == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SetBoolArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetBoolArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetBoolArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (n < 0) {
        fprintf(stderr, "groups_SetBoolArray: n < 0\r\n");
        pthread_exit(NULL);
    }
    
    if (values == NULL) {
        fprintf(stderr, "groups_SetBoolArray: values == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_GetDoubleArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_GetDoubleArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_GetDoubleArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_GetIntArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_SetIntArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_SetIntArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_GetBoolArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_GetBoolArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_GetBoolArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_GetStringArray: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (a == NULL) {
        fprintf(stderr, "groups_GetStringArray: a == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_GetStringArray: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_PropertyNameById: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (propId < 0) {
        fprintf(stderr, "groups_PropertyNameById: propId < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_PrintMember: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (obj == NULL) {
        fprintf(stderr, "groups_PrintMember: obj == NULL");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_RemoveMember: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (index < 0) {
        fprintf(stderr, "groups_RemoveMember: index < 0\r\n");
        pthread_exit(NULL);
    }
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_RemoveMembers: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (prop == NULL) {
        fprintf(stderr, "groups_RemoveMembers: prop == NULL\r\n");
        pthread_exit(NULL);
    }
    
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
		hashTable_Delete(obj);
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
    char* typeName = NULL;
    
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
    if (g == NULL) {
        fprintf(stderr, "groups_SaveToFile: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (fileName == NULL) {
        fprintf(stderr, "groups_SaveToFile: fileName == NULL\r\n");
        pthread_exit(NULL);
    }
    
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

void groups_AppendMembers(groups* g, gcstack* newMembers)
{
    if (g == NULL) {
        fprintf(stderr, "groups_AppendMembers: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (newMembers == NULL) {
        fprintf(stderr, "groups_AppendMembers: newMembers == NULL\r\n");
        pthread_exit(NULL);
    }
    
    gcstack_item* cursor = newMembers->root->next;
    for (; cursor != NULL; cursor = cursor->next)
    {
        groups_AddMember(g, (hash_table*)cursor);
    }
}

// The sscanf function does allow tracking of how many characters are read.
// This function computes the number of characters.
int sscanfSizeOf(const char* text, int type) {
    bool acceptNumeric = true;
    bool acceptDot = type == TYPE_DOUBLE || type == TYPE_STRING;
    bool acceptDoubleQuote = false;
    bool acceptWhiteSpace = true;
    bool acceptComma = false;
    bool acceptParanthesis = type == TYPE_STRING;
    
    bool isNumeric, isDot, isDoubleQuote, isWhiteSpace, isComma, isParanthesis;
    
    int i = 0;
    for (i = 0; text[i] != '\0'; i++) {
        
        isNumeric = text[i] >= '0' && text[i] <= '9';
        isDot = text[i] == '.';
        isDoubleQuote = text[i] == '"';
        isWhiteSpace = text[i] == ' ' || text[i] == '\r' || text[i] == '\n' || text[i] == '\t';
        isComma = text[i] == ',';
        isParanthesis = text[i] == '}' || text[i] == '{' || text[i] == ']' || text[i] == '[' ||
            text[i] == ')' || text[i] == '(';
        
        if (isNumeric && !acceptNumeric) break;
        if (isDot && !acceptDot) break;
        if (isDoubleQuote && !acceptDoubleQuote) break;
        if (isWhiteSpace && !acceptWhiteSpace) break;
        if (isComma && !acceptComma) break;
        if (isParanthesis && !acceptParanthesis) break;
    }
    
    return i;
}

// Reads a double from text and returns the number of characters read.
int sscanDouble(const char* text, double* output)
{
    int s = sscanfSizeOf(text, TYPE_DOUBLE);
    if (s == 0) return 0;
    int n = sscanf(text, "%lg", output);
    return n*s;
}

// Reads an int from text and returns the number of characters read.
int sscanInt(const char* text, int* output)
{
    int s = sscanfSizeOf(text, TYPE_INT);
    if (s == 0) return 0;
    int n = sscanf(text, "%i", output);
    return n*s;
}

bool groups_ReadFromFile(groups* g, string fileName, bool verbose, void(*err)(int line, int column, const char* message))
{
    if (g == NULL) {
        fprintf(stderr, "groups_ReadFromFile: g == NULL\r\n");
        pthread_exit(NULL);
    }
    
    if (fileName == NULL) {
        fprintf(stderr, "groups_ReadFromFile: fileName == NULL\r\n");
        pthread_exit(NULL);
    }
    
    // Get file size.
    struct stat s;
    if (stat(fileName, &s) != 0) {
        return false;
    }
    int size = s.st_size;
    
    FILE* f = fopen(fileName, "r");
    if (f == NULL)
        return false;
    
    char* buff = malloc(sizeof(byte)*size);
    fread(buff, sizeof(byte), size, f);
    fclose(f);
    
    const int _skip_white_space = 0;
    const int _read_properties = 1;
    const int _read_start_paranthesis = 2;
    const int _error = 3;
    const int _read_name = 4;
    const int _read_colon = 5;
    const int _read_value = 6;
    const int _read_string = 7;
    const int _read_backslash_in_string = 8;
    const int _add_after_reading_string = 9;
    const int _read_comma_or_end_paranthesis = 10;
    const int _read_member = 11;
    
    gcstack* memberStack = gcstack_Init(gcstack_Alloc());
    
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
    
    const char* message = NULL;
    char* name = NULL;
    char* text = NULL;
    
    char nameBuffer[255];
    int nameBufferIndex = 0;
    
    gcstack* strStack = gcstack_Init(gcstack_Alloc());
    
    const int tag_properties = 1;
    const int tag_member = 2;
    int tag = 0;
    bool isProperty, isMember, isId, isString;
    
    int valInt;
    double valDouble;
    int unicode, success;
    
    int buffPos = 0;
    int ch;
    int delta;
    int line = 0;
    int column = 0;
    int state = gcstack_PopInt(gc);
    for (ch = buff[buffPos++]; buffPos < size; ch = buff[buffPos++]) {
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
                tag = tag_properties;
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
                tag = tag_member;
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
                isProperty = tag == tag_properties;
                isMember = tag == tag_member;
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
                    // Read the id, take a step back to include last read character.
                    buffPos--; column--;
                    delta = sscanInt(buff+buffPos, &valInt);
                    buffPos += delta;
                    column += delta;
                    hashTable_SetInt(hs, TMP_ID_PROPID, valInt);
                    if (verbose) printf("%i,%i: id %i\r\n", line, column, valInt);
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
                            buffPos--; column--;
                            delta = sscanInt(buff+buffPos, &valInt);
                            buffPos += delta;
                            column += delta;
                            hashTable_SetInt(hs, propId, valInt);
                            if (verbose) printf("%i,%i: %s:%i\r\n", line, column, name, valInt);
                        }
                        else if (type == TYPE_DOUBLE) {
                            buffPos--; column--;
                            delta = sscanDouble(buff+buffPos, &valDouble);
                            
                            buffPos += delta;
                            column += delta;
                            hashTable_SetDouble(hs, propId, valDouble);
                            if (verbose) printf("%i,%i: %s:%lg\r\n", line, column, name, valDouble);
                        }
                        else if (type == TYPE_BOOL) {
                            buffPos--; column--;
                            delta = sscanInt(buff+buffPos-1, &valInt)-1;
                            buffPos += delta;
                            column += delta;
                            hashTable_SetBool(hs, propId, valInt);
                            if (verbose) printf("%i,%i: %s:%i\r\n", line, column, name, valInt);
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
                    success = sscanf(buff+buffPos, "%x", &unicode);
                    if (success) {
                        gcstack_PushInt(strStack, (unicode >> 24) & 0xFF);
                        gcstack_PushInt(strStack, (unicode >> 16) & 0xFF);
                        gcstack_PushInt(strStack, (unicode >> 8) & 0xFF);
                        gcstack_PushInt(strStack, unicode & 0xFF);
                        column += 4;
                        buffPos += 4;
                    }
                    else {
                        message = "Unknown unicode format";
                        state = _error;
                        goto NEW_STATE;
                    }
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
                state = _add_after_reading_string;
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
                    
                    if (tag == tag_member) {
                        // Put the member on the stack, because it is in reverse order.
                        if (verbose) groups_PrintMember(g, hs);
                        hashTable_InitWithMember(hashTable_AllocWithGC(memberStack), hs);
                        hashTable_Init(hs);
                        if (verbose) printf("%i,%i: added member\r\n", line, column);
                    }
                    
                    while (gc->length > 0) {
                        gcstack_PopInt(gc);
                    }
                    gcstack_PushInt(gc, _read_member);
                    state = _skip_white_space;
                    continue;
                }
                message = "Expected ',' or '}'";
                state = _error;
                goto NEW_STATE;
                break;
                
            case _add_after_reading_string:
                if (tag == tag_properties) {
                    groups_AddProperty(g, name, text);
                    if (verbose) printf("%i,%i: _add %s:%s\r\n", line, column, name, text);
                    gcstack_PushInt(gc, _read_comma_or_end_paranthesis);
                    state = _skip_white_space;
                    continue;
                }
                else if (tag == tag_member) {
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
    
    // Add members to group.
    groups_AppendMembers(g, memberStack);
    
CLEAN_UP:
    hashTable_Delete(hs);
    free(hs);
    if (text != NULL) free(text);
    if (name != NULL) free(name);
    
    gcstack_Delete(strStack);
    free(strStack);
    
    gcstack_Delete(gc);
    free(gc);
    
    gcstack_Delete(memberStack);
    free(memberStack);
    
    free(buff);
    
    return true;
}

