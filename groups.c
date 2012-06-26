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

#include "errorhandling.h"
#include "readability.h"
#include "parsing.h"

#include "groups.h"

#define TMP_ID_PROPID 123

void property_Delete(void* const p)
{
	macro_err(p == NULL);
	
	property* const prop = (property* const)p;
	
	if (prop->name != NULL)
	{
		free(prop->name);
		prop->name = NULL;
	}
}

property* property_AllocWithGC(gcstack* const gc)
{
	return (property*)gcstack_malloc(gc, sizeof(property), property_Delete);
}

property* property_InitWithNameAndId
(property* const prop, const char* const name, const int propId)
{
	macro_err(prop == NULL); macro_err(name == NULL);
	
	const int nameLength = strlen(name);
	char* const newName = malloc(sizeof(char)*nameLength);
	prop->name = strcpy(newName, name);
	prop->propId = propId;
	return prop;
}

void groups_Delete(void* const p)
{
	macro_err(p == NULL);
	
	groups* const g = (groups* const)p;
	
	// Free bitstream stuff.
	gcstack* const bitstreams = g->bitstreams;
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
	gcstack* const properties = g->properties;
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

groups* groups_AllocWithGC(gcstack* const gc)
{
	return (groups*)gcstack_malloc(gc, sizeof(groups), groups_Delete);
}

groups* groups_Init(groups* const g)
{
	macro_err(g == NULL);
	
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
int compareStringVSProperty(const void* const a, const void* const b)
{
	const char* const str = (const char* const)a;
	property const* prop = (property const*)b;
	return strcmp(str, prop->name);
}

int compareProperties
(void const* const a, void const* const b)
{
	const property* const aProp = (const property*)a;
	const property* const bProp = (const property*)b;
	const char* const aName = aProp->name;
	const char* const bName = bProp->name;
	
	if (aName == NULL && bName == NULL) 
		return 0;
	else if (aName == NULL) 
		return -1;
	else if (bName == NULL) 
		return 1;
	return strcmp(aName, bName);
}


void sortProperties(groups* const g)
{
	if (g->m_propertiesReady) 
		return;
	
	const int length = g->properties->length;
	
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
	sorting_Sort((void*)items, 0, length, sizeof(property), t, 
		     compareProperties);
	
	free(t);
	
	if (g->m_sortedPropertyItems != NULL)
		free(g->m_sortedPropertyItems);
	
	g->m_sortedPropertyItems = items;
	
	g->m_propertiesReady = true;
}

void createBitstreamArray(groups* const g)
{
	if (g->m_bitstreamsReady) 
		return;
	
	// Create array of pointers to each bitstream to match the stack.
	if (g->m_bitstreamsArray != NULL)
		free(g->m_bitstreamsArray);
	g->m_bitstreamsArray = (bitstream**)gcstack_CreateItemsArrayBackward(g->bitstreams);
	g->m_bitstreamsReady = true;
}

int groups_AddProperty
(groups* const g, const void* const name, const void* const propType)
{
	macro_err(g == NULL); macro_err(name == NULL); 
	macro_err(propType == NULL);
	
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
		const int length = g->bitstreams->length;
		gcstack_item** const items = g->m_sortedPropertyItems;
		const int index = sorting_SearchBinary
		(length, (void*)items, name, compareStringVSProperty);
		
		if (index < 0) 
			goto IF_BREAK;
		
		const property* const prop = (property*)items[index];
		const int existingPropId = prop->propId;
		
		const int oldType = existingPropId/TYPE_STRIDE;
		
		// We can not be sure how to check for compatibility with
		// unknown data types, so we have to tell it's type collision.
		if (oldType == TYPE_UNKNOWN) 
			return -1;
		
		const int newType = propId/TYPE_STRIDE;
		if (oldType == newType)
			// The same name and same type already exists.
			// Since it is compatible, we can return the existing 
			// id.
			return existingPropId;
		
		// The types are different, so we need to tell it is type 
		// collision here.
		return -1;
	}
IF_BREAK:
	
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


int groups_GetProperty(groups* const g, char const* const name)
{
	macro_err(g == NULL); macro_err(name == NULL);
	
	const int length = g->properties->length;
	if (length == 0) return -1;
	
	sortProperties(g);
	gcstack_item** const items = g->m_sortedPropertyItems;
	
	const int index = sorting_SearchBinary
	(length, (void*)items, name, compareStringVSProperty);
	
	if (index < 0) 
		return -1;
	
	// Return the property id.
	const property* const prop = (property*)items[index];
	return prop->propId;
}

const char** groups_GetPropertyNames(groups* const g)
{
	macro_err(g == NULL);
	
	sortProperties(g);
	
	const int length = g->properties->length;
	const char** arr = malloc(sizeof(char*)*length);
	const gcstack_item* cursor = g->properties->root->next;
	const property* prop;
	int k = 0;
	for (; cursor != NULL; cursor = cursor->next)
	{
		prop = (property*)cursor;
		arr[k++] = prop->name;
	}
	return arr;
}

bitstream* groups_getBitstream(groups* const g, const int propId)
{
	// Filter out the type information.
	const int id = propId % TYPE_STRIDE;
	
	createBitstreamArray(g);
	
	// Even when the bitstream is deleted, we can return this
	// safely because it is not removed from the stack and got length 0.
	// It means it will function as an empty set.
	return g->m_bitstreamsArray[id];
}

//
//      Returns a read-only bitstream for use outside use.
//
const bitstream* groups_GetBitstream
(groups* const g, const int propId)
{
	macro_err(g == NULL); macro_err(propId < 0);
	
	return groups_getBitstream(g, propId);
}

bitstream* groups_GetAll(groups* const g) {
	macro_err(g == NULL);
	
	const int length = g->members->length;
	bitstream* const a = bitstream_InitWithValues(bitstream_AllocWithGC(NULL), 
						2, (int[]){0, length});
	bitstream* const deleted = g->m_deletedMembers;
	
	// If there are no deleted members, then return the whole range.
	if (deleted == NULL) return a;
	
	bitstream* const b = bitstream_Except(NULL, a, deleted);
	bitstream_Delete(a);
	free(a);
	return b;
}

void groups_RemoveProperty(groups* const g, const int propId)
{
	macro_err(g == NULL); macro_err(propId < 0);
	
	const int index = propId % TYPE_STRIDE;
	
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

bool groups_IsDefaultVariable
(const int propId, void* const data)
{
	macro_err(propId < 0); macro_err(data == NULL);
	
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

void createMemberArray(groups* const g)
{
	if (g->m_membersReady) return;
	
	hash_table** const items = (hash_table**)gcstack_CreateItemsArrayBackward(g->members);
	if (g->m_memberArray != NULL)
		free(g->m_memberArray);
	g->m_memberArray = items;
	
	g->m_membersReady = true;
}

int groups_AddMember(groups* const g, hash_table* const obj)
{
	macro_err(g == NULL); macro_err(obj == NULL);
	
	int id = g->members->length;
	hash_table* new;
	
	// When reading from file and id does not match,
	// add 'deleted' members to match up with the id.
	bool hasId = false;
	const int* const oldIdPtr = (int*)hashTable_Get(obj, TMP_ID_PROPID);
	const int oldId = oldIdPtr == NULL ? id : *oldIdPtr;
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
	
	macro_hashTable_foreach(new) {
		propId = macro_hashTable_id(new);
		index = propId%TYPE_STRIDE;
		
		a = g->m_bitstreamsArray[index];
		if (a == NULL) continue;
		
		c = bitstream_Or(gc, a, b);
		// Switch stacks so the new one is kept.
		gcstack_Swap(c, a);
	} macro_end_foreach(new)
	
	gcstack_Delete(gc);
	free(gc); 
	
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	return id;
}

//
// This method sets all variables within a bitstream to a value.
//
void groups_SetDouble
(groups* const g, const bitstream* const a, const int propId, const double val)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetDouble(obj, propId, val);
	} macro_end_foreach(a)
	
	// Double does not have a default value, so we need no condition here.
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	bitstream* c = bitstream_Or(NULL, b, a);
	gcstack_Swap(c, b);
	bitstream_Delete(b);
	free(b);
}

//
// This method sets all variables within a bitstream to a value.
//
void groups_SetString
(groups* const g, const bitstream* const a, const int propId, 
 const char* const val)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetString(obj, propId, val);
	} macro_end_foreach(a)
	
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
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

void groups_SetInt
(groups* const g, const bitstream* const a, const int propId, const int val)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	const bool isDefault = -1 == val;
	int i;
	hash_table* obj;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetInt(obj, propId, val);
	} macro_end_foreach(a)
	
	createBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
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

void groups_SetBool
(groups* const g, const bitstream* const a, const int propId, const bool val)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	bool isDefault = 0 == val;
	int i;
	hash_table* obj;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetBool(obj, propId, val);
	} macro_end_foreach(a)
	
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
void groups_SetDoubleArray
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const double* const values)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0); 
	macro_err(n < 0); macro_err(values == NULL);
	
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// We need a counter to read the right value from the array.
	int k = 0;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetDouble(obj, propId, values[k++]);
	} macro_end_foreach(a)
	
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
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const char** const values)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0); 
	macro_err(n < 0); macro_err(values == NULL);
	
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
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetString(obj, propId, values[k++]);
	} macro_end_foreach(a)
	
	createBitstreamArray(g);
	
	gcstack* const gc = gcstack_Init(gcstack_Alloc());
	
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
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const int* values)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0); 
	macro_err(n < 0); macro_err(values == NULL);
	
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* const notDefaultIndices = malloc(n*sizeof(int));
	const int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetInt(obj, propId, values[k++]);
	} macro_end_foreach(a)
	
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
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const bool* const values)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0); 
	macro_err(n < 0); macro_err(values == NULL);
	
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* const notDefaultIndices = malloc(n*sizeof(int));
	const int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetBool(obj, propId, values[k++]);
	} macro_end_foreach(a)
	
	createBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
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
(groups* const g, const bitstream* const a, const int propId)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	double* arr = malloc(sizeof(double)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		arr[k++] = *((double*)hashTable_Get(obj, propId));
	} macro_end_foreach(a)
	
	return arr;
}


int* groups_GetIntArray
(groups* const g, const bitstream* const a, const int propId)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	int* arr = malloc(sizeof(int)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		arr[k++] = *((int*)hashTable_Get(obj, propId));
	} macro_end_foreach(a)
	
	return arr;
}

bool* groups_GetBoolArray
(groups* const g, const bitstream* const a, const int propId)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	bool* arr = malloc(sizeof(bool)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		arr[k++] = *((bool*)hashTable_Get(obj, propId));
	} macro_end_foreach(a)
	
	return arr;
}

const char** groups_GetStringArray
(groups* const g, const bitstream* const a, const int propId)
{
	macro_err(g == NULL); macro_err(a == NULL); macro_err(propId < 0);
	
	// Make sure we have a table with pointers to members.
	createMemberArray(g);
	
	int size = bitstream_Size(a);
	const char** arr = malloc(sizeof(string)*size);
	
	int i;
	hash_table* obj;
	
	int k = 0;
	macro_foreach (a) {
		i = macro_pos(a);
		obj = g->m_memberArray[i];
		arr[k++] = (const char*)hashTable_Get(obj, propId);
	} macro_end_foreach(a)
	
	return arr;
}

const char* groups_PropertyNameById
(const groups* const g, const int propId)
{
	macro_err(g == NULL); macro_err(propId < 0);
	
	property* prop;
	gcstack_item* cursor = g->properties->root->next;
	for (; cursor != NULL; cursor = cursor->next)
	{
		prop = (property*)cursor;
		if (prop->propId == propId) return prop->name;
	}
	return NULL;
}

void groups_PrintMember
(const groups* const g, const hash_table* const obj)
{
	macro_err(g == NULL); macro_err(obj == NULL);
	
	int propId, type;
	
	macro_hashTable_foreach(obj) {
		propId = macro_hashTable_id(obj);
		type = propId/TYPE_STRIDE;
		const char* name = groups_PropertyNameById(g, propId);
		if (type == TYPE_DOUBLE)
			printf("%s:%lg ", name, macro_hashTable_double(obj));
		else if (type == TYPE_INT)
			printf("%s:%i ", name, macro_hashTable_int(obj));
		else if (type == TYPE_BOOL)
			printf("%s:%i ", name, macro_hashTable_bool(obj));
		else if (type == TYPE_STRING)
			printf("%s:%s ", name, macro_hashTable_string(obj));
	} macro_end_foreach(obj)
	printf("\r\n");
}

void groups_RemoveMember(groups* const g, const int index)
{
	macro_err(g == NULL); macro_err(index < 0);
	
	createMemberArray(g);
	
	hash_table* const obj = g->m_memberArray[index];
	gcstack* const gc = gcstack_Init(gcstack_Alloc());
	
	int propId;
	bitstream* a;
	bitstream* b = bitstream_InitWithValues
	(bitstream_AllocWithGC(gc), 2, (int[]){index,index+1});
	bitstream* c;
	macro_hashTable_foreach(obj) {
		propId = macro_hashTable_id(obj);
		a = groups_getBitstream(g, propId);
		if (a == NULL) continue;
		
		c = bitstream_Except(gc, a, b);
		gcstack_Swap(c, a);
	} macro_end_foreach(obj)
	
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

void groups_RemoveMembers(groups* const g, bitstream const* prop)
{
	macro_err(g == NULL); macro_err(prop == NULL);
	
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
	macro_foreach (prop) {
		// Free the member but don't delete it, in order to maintain index.
		index = macro_pos(prop);
		obj = g->m_memberArray[index];
		hashTable_Delete(obj);
	} macro_end_foreach (prop)
	
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	// Add the member to bitstream of deleted members for reuse of index.
	bitstream* const d = g->m_deletedMembers;
	bitstream* const e = bitstream_Or(gc, d, prop);
	gcstack_Swap(d, e);
	g->m_deletedMembers = e;
	
	gcstack_Delete(gc);
	
	free(gc);
}

bool groups_IsUnknown(const int propId)
{
	return propId/TYPE_STRIDE == TYPE_UNKNOWN;
}

bool groups_IsDouble(const int propId)
{
	return propId/TYPE_STRIDE == TYPE_DOUBLE;
}

bool groups_IsInt(const int propId)
{
	return propId/TYPE_STRIDE == TYPE_INT;
}

bool groups_IsString(const int propId)
{
	return propId/TYPE_STRIDE == TYPE_STRING;
}

bool groups_IsBool(const int propId)
{
	return propId/TYPE_STRIDE == TYPE_BOOL;
}

void groups_PrintStringToFile(FILE* const f, const char* const text) {
	// Put quotation mark at the beginning.
	fputc('"', f);
	char ch;
	for (const char* cursor = text; *cursor != '\0'; cursor++) {
		ch = *cursor;
		
		// Handle escaped characters.
		if (ch == '"') {
			fputc('\\', f);
			fputc('"', f);
		}
		else if (ch == '\t') {
			fputc('\\', f);
			fputc('t', f);
		}
		else if (ch == '\r') {
			fputc('\\', f);
			fputc('r', f);
		}
		else if (ch == '\n') {
			fputc('\\', f);
			fputc('n', f);
		}
		else if (ch == '\\') {
			fputc('\\', f);
			fputc('\\', f);
		}
		else if (ch == '/') {
			fputc('\\', f);
			fputc('/', f);
		}
		else if (ch == '\f') {
			fputc('\\', f);
			fputc('f', f);
		}
		else {
			fputc(ch, f);
		}
	}
	// Put quotation mark at the end.
	fputc('"', f);
}

void groups_PrintMemberToFile
(FILE* const f, const groups* const g, const hash_table* const obj)
{
	int propId, type;
	bool first = true;
	
	macro_hashTable_foreach(obj) {
		propId = macro_hashTable_id(obj);
		type = propId/TYPE_STRIDE;
		
		// Write comma to separate the items in the document.
		if (!first) fprintf(f, ", ");
		first = false;
		
		const char* name = groups_PropertyNameById(g, propId);
		if (type == TYPE_DOUBLE)
			fprintf(f, "%s:%lg", name, macro_hashTable_double(obj));
		else if (type == TYPE_INT)
			fprintf(f, "%s:%i", name, macro_hashTable_int(obj));
		else if (type == TYPE_BOOL)
			fprintf(f, "%s:%i", name, macro_hashTable_bool(obj));
		else if (type == TYPE_STRING) {
			fprintf(f, "%s:", name);
			groups_PrintStringToFile(f, macro_hashTable_string(obj));
		}
	} macro_end_foreach(obj)
}

void groups_PrintPropertyToFile
(FILE* const f, const groups* const g, property* const prop)
{
	const char* const name = prop->name;
	const int propId = prop->propId;
	const int type = propId/TYPE_STRIDE;
	const char* typeName = NULL;
	
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

bool groups_SaveToFile(groups* const g, const char* const fileName)
{
	macro_err(g == NULL); macro_err(fileName == NULL);
	
	FILE* f = fopen(fileName, "w");
	if (f == NULL)
		return false;
	
	const gcstack_item* cursor;
	
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
	const hash_table* member;
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

void groups_AppendMembers(groups* const g, gcstack* const newMembers)
{
	macro_err(g == NULL); macro_err(newMembers == NULL);
	
	gcstack_item* cursor = newMembers->root->next;
	for (; cursor != NULL; cursor = cursor->next)
	{
		groups_AddMember(g, (hash_table*)cursor);
	}
}

typedef struct read_from_file_settings {
	gcstack* memberStack;
	gcstack* stateStack;
	gcstack* strStack;
	char* buff;
	string propText;
	int propLength;
	int propIndex;
	string memberText;
	int memberLength;
	int memberIndex;
	hash_table* hs;
	string message;
	char* name;
	char* text;
	char nameBuffer[255];
	int nameBufferIndex;
	int tag;
	bool isProperty, isMember, isId, isString;
	int valInt;
	double valDouble;
	int unicode, success;
	int buffPos;
	int ch;
	int delta;
	int line;
	int column;
	int state;
	int propId;
} read_from_file_settings;

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

const int tag_properties = 1;
const int tag_member = 2;

const int GROUPS_RFF_CONTINUE = 0;
const int GROUPS_RFF_CLEAN_UP = 1;
const int GROUPS_RFF_NEW_STATE = 2;

const char* const GROUPS_ERROR_EXPECTED_PROPERTIES = "Expected 'properties'";
const char* const GROUPS_ERROR_EXPECTED_MEMBER = "Expected 'member'";
const char* const GROUPS_ERROR_EXPECTED_START_CURLY_PARANTHESIS = "Expected '{'";
const char* const GROUPS_ERROR_EXPECTED_COLON = "Expected ':'";
const char* const GROUPS_ERROR_EXPECTED_DOUBLE_QUOTE = "Expected '\"'";
const char* const GROUPS_ERROR_ID_KEYWORD_RESERVED = "The 'id' keyword is reserved";
const char* const GROUPS_ERROR_PROPERTY_NOT_FOUND = "Property not found";
const char* const GROUPS_ERROR_UNKNOWN_UNICODE_FORMAT = "Unknown unicode format";
const char* const GROUPS_ERROR_EXPECTED_COMMA_OR_END_CURLY_PARANTHESIS =
"Expected ',' or '}'";
const char* const GROUPS_ERROR_INVALID_TAG = 
"Invalid tag, expected 'member' or property";

//
// This macro does the proper action by the return value from function.
// I can not be handled differently since goto and continue are native
// keywords in C and does not allow conditions. Normally I avoid macros
// when possible, but in this case it will make the code easier to maintain.
//
#define macro_rff_action(a) \
switch (a) { case GROUPS_RFF_CONTINUE: continue; \
case GROUPS_RFF_CLEAN_UP: goto CLEAN_UP; \
case GROUPS_RFF_NEW_STATE: goto NEW_STATE; }

inline void groups_readFromFile_initSettings
(read_from_file_settings* const s, FILE* const f, const int fileSize);

void groups_readFromFile_initSettings
(read_from_file_settings* const s, FILE* const f, 
 const int fileSize) 
{
	s->buff = malloc(sizeof(byte)*fileSize);
	fread(s->buff, sizeof(byte), fileSize, f);
	fclose(f);
	
	s->memberStack = gcstack_Init(gcstack_Alloc());
	
	s->stateStack = gcstack_Init(gcstack_Alloc());
	gcstack_PushInt(s->stateStack, _read_member);
	gcstack_PushInt(s->stateStack, _read_properties);
	gcstack_PushInt(s->stateStack, _skip_white_space);
	
	s->propText = "properties";
	s->propLength = strlen(s->propText);
	s->propIndex = 0;
	
	s->memberText = "member";
	s->memberLength = strlen(s->memberText);
	s->memberIndex = 0;
	
	s->hs = hashTable_Init(hashTable_AllocWithGC(NULL));
	
	s->message = NULL;
	s->name = NULL;
	s->text = NULL;
	
	s->nameBufferIndex = 0;
	
	s->strStack = gcstack_Init(gcstack_Alloc());
	
	s->tag = 0;
	s->buffPos = 0;
	s->line = 0;
	s->column = 0;
	s->state = gcstack_PopInt(s->stateStack);
}

inline void groups_readFromFile_deleteSettings(read_from_file_settings* const s);

void groups_readFromFile_deleteSettings(read_from_file_settings* const s)
{
	hashTable_Delete(s->hs);
	free(s->hs);
	if (s->text != NULL) free(s->text);
	if (s->name != NULL) free(s->name);
	
	gcstack_Delete(s->strStack);
	free(s->strStack);
	
	gcstack_Delete(s->stateStack);
	free(s->stateStack);
	
	gcstack_Delete(s->memberStack);
	free(s->memberStack);
	
	free(s->buff);
}

inline int groups_readFromFile_skipWhiteSpace
(read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_skipWhiteSpace
(read_from_file_settings* const s, const bool verbose)
{
	if (s->ch == ' ' || s->ch == '\r' || s->ch == '\t' || s->ch == '\n') 
		return GROUPS_RFF_CONTINUE;
	if (verbose) 
		printf("%i,%i: _skip_white_space\r\n", s->line, s->column);
	
	s->state = gcstack_PopInt(s->stateStack);
	
	return GROUPS_RFF_NEW_STATE;
}

inline int groups_readFromFile_error
(const read_from_file_settings* const s, 
 void(*const err)(int line, int column, const char* message));

int groups_readFromFile_error
(const read_from_file_settings* const s, 
 void(* const err)(int line, int column, const char* message))
{
	// Error takes one argument from the stack.
	if (err != NULL) {
		err(s->line, s->column, s->message);
	} else {
		printf("%i,%i: %s\r\n", s->line, s->column, s->message);
	}
	return GROUPS_RFF_CLEAN_UP;
}

inline int groups_readFromFile_readProperties
(read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_readProperties
(read_from_file_settings* const s, const bool verbose)
{
	// Checks character for character against 
	// expected keyword.
	if (s->ch == s->propText[s->propIndex++]) 
		return GROUPS_RFF_CONTINUE;
	
	if (s->propIndex < s->propLength) {
		s->message = GROUPS_ERROR_EXPECTED_PROPERTIES;
		s->state = _error;
		return GROUPS_RFF_NEW_STATE;
	}
	
	if (verbose) 
		printf("%i,%i: _read_properties\r\n", s->line, s->column);
	
	s->tag = tag_properties;
	gcstack_PushInt(s->stateStack, _read_name);
	gcstack_PushInt(s->stateStack, _read_start_paranthesis);
	s->state = _skip_white_space;
	return GROUPS_RFF_NEW_STATE;
}

inline int groups_readFromFile_readMember
(read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_readMember
(read_from_file_settings* const s, const bool verbose)
{
	// Checks character for character against 
	// expected keyword.
	if (s->ch == s->memberText[s->memberIndex++]) 
		return GROUPS_RFF_CONTINUE;
	if (s->memberIndex < s->memberLength) {
		s->message = GROUPS_ERROR_EXPECTED_MEMBER;
		s->state = _error;
		return GROUPS_RFF_NEW_STATE;
	}
	s->memberIndex = 0;
	if (verbose) 
		printf("%i,%i: _read_member\r\n", s->line, s->column);
	
	s->tag = tag_member;
	gcstack_PushInt(s->stateStack, _read_name);
	gcstack_PushInt(s->stateStack, _read_start_paranthesis);
	s->state = _skip_white_space;
	return GROUPS_RFF_NEW_STATE;
}

inline int groups_readFromFile_readStartParanthesis
(read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_readStartParanthesis
(read_from_file_settings* const s, const bool verbose)
{
	// Reads start paranthesis.
	if (s->ch == '{') {
		// Always skip white space after start 
		// paranthesis.
		if (verbose) 
			printf("%i,%i: _read_start_para"
			       "nthesis\r\n", s->line, s->column);
		
		s->state = _skip_white_space;
		return GROUPS_RFF_CONTINUE;
	}
	
	s->message = GROUPS_ERROR_EXPECTED_START_CURLY_PARANTHESIS;
	s->state = _error;
	return GROUPS_RFF_NEW_STATE;
}

inline int groups_readFromFile_readColon
(read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_readColon
(read_from_file_settings* const s, const bool verbose)
{
	if (s->ch == ':') {
		// Always skip white space after colon.
		if (verbose) 
			printf("%i,%i: _read_colon\r\n",
			       s->line, s->column);
		
		s->state = _skip_white_space;
		return GROUPS_RFF_CONTINUE;
	}
	
	s->message = GROUPS_ERROR_EXPECTED_COLON;
	s->state = _error;
	return GROUPS_RFF_NEW_STATE;
}

inline int groups_readFromFile_readName
(read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_readName
(read_from_file_settings* const s, const bool verbose)
{
	// Reads a name that contains only letters and 
	// alphanumeric characters.
	if ((s->ch >= 'a' && s->ch <= 'z') || 
	    (s->ch >= 'A' && s->ch <= 'Z') || 
	    (s->ch >= '0' && s->ch <= '9')) {
		s->nameBuffer[s->nameBufferIndex++] = s->ch;
		return GROUPS_RFF_CONTINUE;
	}
	s->nameBuffer[s->nameBufferIndex++] = '\0';
	if (verbose) 
		printf("%i,%i: _read_name %s\r\n", 
		       s->line, s->column, s->nameBuffer);
	
	if (s->name != NULL) 
		free(s->name);
	
	s->name = malloc(s->nameBufferIndex*sizeof(char));
	strcpy(s->name, s->nameBuffer);
	s->nameBufferIndex = 0;
	gcstack_PushInt(s->stateStack, _read_value);
	gcstack_PushInt(s->stateStack, _read_colon);
	s->state = _skip_white_space;
	return GROUPS_RFF_NEW_STATE;
}

inline int groups_readFromFile_readValue
(groups* const g, read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_readValue
(groups* const g, read_from_file_settings* const s, const bool verbose)
{
	s->isString = (s->ch == '"');
	s->isProperty = s->tag == tag_properties;
	s->isMember = s->tag == tag_member;
	s->isId = strcmp(s->name, "id") == 0;
	
	if (s->isProperty && !s->isString) {
		s->message = GROUPS_ERROR_EXPECTED_DOUBLE_QUOTE;
		s->state = _error;
		return GROUPS_RFF_NEW_STATE;
	}
	
	if (s->isId && s->isProperty) {
		s->message = GROUPS_ERROR_ID_KEYWORD_RESERVED;
		s->state = _error;
		return GROUPS_RFF_NEW_STATE;
	}
	
	if (verbose) 
		printf("%i,%i: _read_value %s\r\n", s->line, s->column, 
		       s->name);
	
	if (s->isString && s->isProperty) {
		s->state = _read_string;
		return GROUPS_RFF_CONTINUE;
	}
	if (s->isId) {
		// Read the id, take a step back to 
		// include last read character.
		s->buffPos--; s->column--;
		s->delta = parsing_ScanInt(s->buff+s->buffPos, 
					  &s->valInt);
		s->buffPos += s->delta;
		s->column += s->delta;
		hashTable_SetInt(s->hs, TMP_ID_PROPID, 
				 s->valInt);
		if (verbose) 
			printf("%i,%i: id %i\r\n", s->line, s->column, 
			       s->valInt);
	}
	else if (s->isMember) {
		s->propId = groups_GetProperty(g, s->name);
		if (s->propId < 0) {
			s->message = GROUPS_ERROR_PROPERTY_NOT_FOUND;
			s->state = _error;
			return GROUPS_RFF_NEW_STATE;
		} else {
			// Read the types supported in 
			// this format.
			// Steps one character back to 
			// read to get the first 
			// character when reading.
			const int type = s->propId/TYPE_STRIDE;
			if (type == TYPE_INT) {
				s->buffPos--; s->column--;
				s->delta = parsing_ScanInt
				(s->buff+s->buffPos, &s->valInt);
				s->buffPos += s->delta;
				s->column += s->delta;
				hashTable_SetInt(s->hs, s->propId, s->valInt);
				if (verbose) 
					printf
					("%i,%i: %s:%i\r\n", s->line, s->column,
					 s->name, s->valInt);
			}
			else if (type == TYPE_DOUBLE) {
				s->buffPos--; s->column--;
				s->delta = parsing_ScanDouble
				(s->buff+s->buffPos, &s->valDouble);
				
				s->buffPos += s->delta;
				s->column += s->delta;
				hashTable_SetDouble
				(s->hs, s->propId, s->valDouble);
				if (verbose) 
					printf
					("%i,%i: %s:%lg\r\n", s->line, 
					 s->column, s->name, s->valDouble);
			}
			else if (type == TYPE_BOOL) {
				s->buffPos--; s->column--;
				s->delta = parsing_ScanInt
				(s->buff+s->buffPos-1, &s->valInt)-1;
				s->buffPos += s->delta;
				s->column += s->delta;
				hashTable_SetBool(s->hs, s->propId, s->valInt);
				if (verbose) 
					printf("%i,%i: %s:%i\r\n", s->line, 
					       s->column, s->name, s->valInt);
			}
			else if (type == TYPE_STRING) {
				s->state = _read_string;
				return GROUPS_RFF_CONTINUE;
			}
		}
	}
	
	gcstack_PushInt(s->stateStack, _read_comma_or_end_paranthesis);
	s->state = _skip_white_space;
	return GROUPS_RFF_CONTINUE;
}

inline int groups_readFromFile_ReadBackSlashInString
(read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_ReadBackSlashInString
(read_from_file_settings* const s, const bool verbose)
{
	// Read special characters that are escaped by backspace.
	if (s->ch == '"')
		gcstack_PushInt(s->strStack, s->ch);
	else if (s->ch == 'r')
		gcstack_PushInt(s->strStack, '\r');
	else if (s->ch == 'n')
		gcstack_PushInt(s->strStack, '\n');
	else if (s->ch == '\\')
		gcstack_PushInt(s->strStack, '\\');
	else if (s->ch == '/')
		gcstack_PushInt(s->strStack, '/');
	else if (s->ch == 'b')
		gcstack_PushInt(s->strStack, '\b');
	else if (s->ch == 'f')
		gcstack_PushInt(s->strStack, '\f');
	else if (s->ch == 't')
		gcstack_PushInt(s->strStack, '\t');
	else if (s->ch == 'u') {
		// A unicode letter consists of 4 bytes, but
		// first we need to convert from hexadecimals to byte form.
		// The least significant byte should be placed first,
		// because this is the order the unicode letter is detected.
		s->success = sscanf(s->buff+s->buffPos, "%x", &s->unicode);
		if (s->success) {
			gcstack_PushInt(s->strStack, (s->unicode >> 24) & 0xFF);
			gcstack_PushInt(s->strStack, (s->unicode >> 16) & 0xFF);
			gcstack_PushInt(s->strStack, (s->unicode >> 8) & 0xFF);
			gcstack_PushInt(s->strStack, s->unicode & 0xFF);
			s->column += 4;
			s->buffPos += 4;
		}
		else {
			s->message = GROUPS_ERROR_UNKNOWN_UNICODE_FORMAT;
			s->state = _error;
			return GROUPS_RFF_NEW_STATE;
		}
	}
	s->state = _read_string;
	return GROUPS_RFF_CONTINUE;
}

inline int groups_readFromFile_readString
(read_from_file_settings* const s, bool const verbose);

int groups_readFromFile_readString
(read_from_file_settings* const s, bool const verbose)
{
	if (s->ch == '\\') {
		s->state = _read_backslash_in_string;
		return GROUPS_RFF_CONTINUE;
	}
	if (s->ch != '"') {
		gcstack_PushInt(s->strStack, s->ch);
		return GROUPS_RFF_CONTINUE;
	}
	
	if (s->text != NULL) 
		free(s->text);
	
	s->text = malloc((s->strStack->length+1)*sizeof(char));
	s->text[s->strStack->length] = '\0';
	while (s->strStack->length > 0)
		s->text[s->strStack->length] = gcstack_PopInt(s->strStack);
	
	if (verbose) 
		printf("%i, %i: _read_string %s\r\n", s->line, s->column, 
		       s->text);
	
	s->state = _add_after_reading_string;
	return GROUPS_RFF_NEW_STATE;	
}

inline int groups_readFromFile_readCommadOrEndParanthesis
(groups* const g, read_from_file_settings* const s, bool const verbose);

int groups_readFromFile_readCommadOrEndParanthesis
(groups* const g, read_from_file_settings* const s, const bool verbose)
{
	if (s->ch == ',') {
		if (verbose) 
			printf("%i,%i: _read_comma\r\n", s->line, s->column);
		
		gcstack_PushInt(s->stateStack, _read_name);
		s->state = _skip_white_space;
		return GROUPS_RFF_CONTINUE;
	}
	else if (s->ch == '}') {
		// Clear the stack and read a new member.
		if (verbose) 
			printf("%i,%i: _read_end_paranthesis\r\n", s->line, 
			       s->column);
		
		if (s->tag == tag_member) {
			// Put the member on the stack, because it is in 
			// reverse order.
			if (verbose) 
				groups_PrintMember(g, s->hs);
			
			hashTable_InitWithMember
			(hashTable_AllocWithGC(s->memberStack), s->hs);
			hashTable_Init(s->hs);
			if (verbose) 
				printf
				("%i,%i: added member\r\n", s->line, s->column);
		}
		
		while (s->stateStack->length > 0) {
			gcstack_PopInt(s->stateStack);
		}
		gcstack_PushInt(s->stateStack, _read_member);
		s->state = _skip_white_space;
		return GROUPS_RFF_CONTINUE;
	}
	s->message = GROUPS_ERROR_EXPECTED_COMMA_OR_END_CURLY_PARANTHESIS;
	s->state = _error;
	return GROUPS_RFF_NEW_STATE;
}

inline int groups_readFromFile_afterReadingString
(groups* const g, read_from_file_settings* const s, const bool verbose);

int groups_readFromFile_afterReadingString
(groups* const g, read_from_file_settings* const s, const bool verbose)
{
	if (s->tag == tag_properties) {
		groups_AddProperty(g, s->name, s->text);
		if (verbose) 
			printf
			("%i,%i: _add %s:%s\r\n", s->line, s->column, s->name, 
			 s->text);
		gcstack_PushInt(s->stateStack, _read_comma_or_end_paranthesis);
		s->state = _skip_white_space;
		return GROUPS_RFF_CONTINUE;
	} else if (s->tag == tag_member) {
		int propId = groups_GetProperty(g, s->name);
		hashTable_SetString(s->hs, propId, s->text);
		if (verbose) 
			printf("%i,%i: _add %s:%s\r\n", 
			       s->line, s->column, s->name, s->text);
		gcstack_PushInt(s->stateStack, _read_comma_or_end_paranthesis);
		s->state = _skip_white_space;
		return GROUPS_RFF_CONTINUE;
	}
	s->state = _error;
	s->message = GROUPS_ERROR_INVALID_TAG;
	return GROUPS_RFF_NEW_STATE;
}

bool groups_ReadFromFile(groups* const g, const char* const fileName, const bool verbose, 
			 void(* const err)(int line, int column, const char* message))
{
	macro_err(g == NULL); macro_err(fileName == NULL);
	
	// Get file size.
	struct stat fileState;
	if (stat(fileName, &fileState) != 0) {
		return false;
	}
	int size = fileState.st_size;
	
	FILE* const f = fopen(fileName, "r");
	if (f == NULL)
		return false;
	
	// Make settings ready for reading.
	read_from_file_settings s;
	groups_readFromFile_initSettings(&s, f, size);
	
	for (s.ch = s.buff[s.buffPos++]; 
	     s.buffPos < size; 
	     s.ch = s.buff[s.buffPos++]) 
	{
		if (s.ch == '\n') 
			s.line++;
		
		if (s.ch == '\n') 
			s.column = 0; 
		else 
			s.column++;
		
	NEW_STATE:
		switch (s.state) {
			case _skip_white_space:
				macro_rff_action
				(groups_readFromFile_skipWhiteSpace
				 (&s, verbose));
				break;
			case _error:
				macro_rff_action
				(groups_readFromFile_error
				 (&s, err));
				break;
			case _read_properties:
				macro_rff_action
				(groups_readFromFile_readProperties
				 (&s, verbose));
				break;
			case _read_member:
				macro_rff_action
				(groups_readFromFile_readMember
				 (&s, verbose));
				break;
			case _read_start_paranthesis:
				macro_rff_action
				(groups_readFromFile_readStartParanthesis
				 (&s, verbose));
				break;
			case _read_colon:
				macro_rff_action
				(groups_readFromFile_readColon
				 (&s, verbose));
				break;
			case _read_name:
				macro_rff_action
				(groups_readFromFile_readName
				 (&s, verbose));
				break;
			case _read_value:
				macro_rff_action
				(groups_readFromFile_readValue
				 (g, &s, verbose));
				break;
			case _read_backslash_in_string:
				macro_rff_action
				(groups_readFromFile_ReadBackSlashInString
				 (&s, verbose));
				break;
			case _read_string:
				macro_rff_action
				(groups_readFromFile_readString
				 (&s, verbose));
				break;
			case _read_comma_or_end_paranthesis:
				macro_rff_action
				(groups_readFromFile_readCommadOrEndParanthesis
				 (g, &s, verbose));
				break;
			case _add_after_reading_string:
				macro_rff_action
				(groups_readFromFile_afterReadingString
				 (g, &s, verbose));
				break;
				
		}
		break;
	}
	
	// Add members to group.
	groups_AppendMembers(g, s.memberStack);
	
CLEAN_UP:
	groups_readFromFile_deleteSettings(&s);
	return true;
}

