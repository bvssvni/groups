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
#include "group.h"
#include "member.h"
#include "sorting.h"

#include "errorhandling.h"
#include "readability.h"
#include "parsing.h"

#include "gop.h"
#include "groups-array.h"

#include "boolean.h"

#define TMP_ID_PROPID 123

void property_Delete(void* const p)
{
	macro_err_return(p == NULL);
	
	property* const prop = (property* const)p;
	
	if (prop->name != NULL)
	{
		free(prop->name);
		prop->name = NULL;
	}
}

property* property_GcAlloc(gcstack* const gc)
{
	return (property*)gcstack_malloc(gc, sizeof(property), property_Delete);
}

property* property_InitWithNameAndId
(property* const prop, const char* const name, const int propId)
{
	macro_err_return_null(prop == NULL);
	macro_err_return_null(name == NULL);
	
	const size_t nameLength = strlen(name);
	char* const newName = malloc(sizeof(char)*nameLength);
	prop->name = strcpy(newName, name);
	prop->propId = propId;
	return prop;
}

void gop_Delete(void* const p)
{
	macro_err_return(p == NULL);
	
	gop* const g = (gop* const)p;
	
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
		group_Delete(g->m_deletedMembers);
		
		free(g->m_deletedMembers);
	}
}

gop* gop_GcAlloc(gcstack* const gc)
{
	return (gop*)gcstack_malloc(gc, sizeof(gop), gop_Delete);
}

gop* gop_Init(gop* const g)
{
	macro_err_return_null(g == NULL);
	
	g->bitstreams = gcstack_Init(gcstack_Alloc());
	g->m_bitstreamsReady = false;
	g->m_bitstreamsArray = NULL;
	g->m_deletedBitstreams = group_InitWithSize(group_GcAlloc(NULL), 0);
	
	g->properties = gcstack_Init(gcstack_Alloc());
	g->m_propertiesReady = false;
	g->m_sortedPropertyItems = NULL;
	
	g->members = gcstack_Init(gcstack_Alloc());
	g->m_membersReady = false;
	g->m_memberArray = NULL;
	g->m_deletedMembers = group_InitWithSize(group_GcAlloc(NULL), 0);
	return g;
}

int compareStringVSProperty(const void* const a, const void* const b);

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
(void const* const a, void const* const b);

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

void sortProperties(gop* const g);

void sortProperties(gop* const g)
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

void gop_CreateBitstreamArray(gop* const g)
{
	if (g->m_bitstreamsReady) 
		return;
	
	// Create array of pointers to each bitstream to match the stack.
	if (g->m_bitstreamsArray != NULL)
		free(g->m_bitstreamsArray);
	g->m_bitstreamsArray = (group**)gcstack_CreateItemsArrayBackward
	(g->bitstreams);
	
	g->m_bitstreamsReady = true;
}

int gop_AddProperty
(gop* const g, const void* const name, const void* const propType)
{
	macro_err_return_zero(g == NULL);
	macro_err_return_zero(name == NULL); 
	macro_err_return_zero(propType == NULL);
	
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
		gop_CreateBitstreamArray(g);
		propIndex = group_PopEnd(g->m_deletedBitstreams);
		propId = (propId-propId%TYPE_STRIDE) + propIndex;
		group_InitWithSize(g->m_bitstreamsArray[propIndex], 0);
	}
	else
		// Create a new empty bitstream for that property.
		group_InitWithSize(group_GcAlloc(g->bitstreams), 0);
	
	// Create new property that links name to id.
	property_InitWithNameAndId
	(property_GcAlloc(g->properties), name, propId);
	
	
	g->m_propertiesReady = false;
	g->m_bitstreamsReady = false;
	
	return propId;
}


int gop_GetProperty(gop* const g, char const* const name)
{
	macro_err_return_zero(g == NULL);
	macro_err_return_zero(name == NULL);
	
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

const char** gop_GetPropertyNames(gop* const g)
{
	macro_err_return_null(g == NULL);
	
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

group* getBitstream(gop* const g, const int propId);

group* getBitstream(gop* const g, const int propId)
{
	// Filter out the type information.
	const int id = propId % TYPE_STRIDE;
	
	gop_CreateBitstreamArray(g);
	
	// Even when the bitstream is deleted, we can return this
	// safely because it is not removed from the stack and got length 0.
	// It means it will function as an empty set.
	return g->m_bitstreamsArray[id];
}

//
//      Returns a read-only bitstream for use outside use.
//
group* gop_GcGetBitstream
(gcstack* const gc, gop* const g, const int propId)
{
	macro_err(g == NULL); macro_err(propId < 0);
	
	group* a = getBitstream(g, propId);
	return group_GcClone(gc, a);
}

group* gop_GcGetAll(gcstack* const gc, gop* const g) 
{
	macro_err_return_null(g == NULL);
	
	const int length = g->members->length;
	group* const a = group_InitWithValues
	(group_GcAlloc(NULL), 2, (int[]){0, length});
	group* const deleted = g->m_deletedMembers;
	
	// If there are no deleted members, then return the whole range.
	if (deleted == NULL) return a;
	
	group* const b = group_GcExcept(gc, a, deleted);
	group_Delete(a);
	free(a);
	return b;
}

void gop_RemoveProperty(gop* const g, const int propId)
{
	macro_err_return(g == NULL);
	macro_err_return(propId < 0);
	
	const int index = propId % TYPE_STRIDE;
	
	// Delete content, but do not move from stack of bitstreams.
	gop_CreateBitstreamArray(g);
	group* a = g->m_bitstreamsArray[index];
	group_Delete(a);
	
	// Add the property id to deleted bitstream for reuse.
	group* b = group_InitWithValues
	(group_GcAlloc(NULL), 2, (int[]){index, index+1});
	group* c = g->m_deletedBitstreams;
	group* d = group_GcOr(NULL, c, b);
	gcstack_Swap(d, c);
	g->m_deletedBitstreams = d;
	group_Delete(c);
	free(c);
	group_Delete(b);
	free(b);
	
	// Loop through the stack to find the property to delete.
	const gcstack_item* cursor = g->properties->root->next;
	property* prop = NULL;
	for (; cursor != NULL; cursor = cursor->next)
	{
		prop = (property*)cursor;
		if (prop->propId == propId) 
			break;
	}
	
	if (prop == NULL) 
		return;
	
	// Delete it, including freeing the pointer.
	gcstack_free(g->properties, (gcstack_item*)prop);
	
	g->m_propertiesReady = false;
}

int gop_IsDefaultVariable
(const int propId, void* const data)
{
	macro_err_return_zero(propId < 0);
	macro_err_return_zero(data == NULL);
	
	int type = propId/TYPE_STRIDE;
	if (type == TYPE_DOUBLE) return false;
	else if (type == TYPE_INT)
	{
		int* val = (int*)data;
		if (*val == -1) return true;
	}
	else if (type == TYPE_BOOL)
	{
		int* val = (int*)data;
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

void gop_CreateMemberArray(gop* const g)
{
	if (g->m_membersReady) return;
	
	hash_table** const items = (hash_table**)gcstack_CreateItemsArrayBackward(g->members);
	if (g->m_memberArray != NULL)
		free(g->m_memberArray);
	g->m_memberArray = items;
	
	g->m_membersReady = true;
}

int gop_AddMember(gop* const g, hash_table* const obj)
{
	macro_err(g == NULL); macro_err(obj == NULL);
	
	int id = g->members->length;
	hash_table* new;
	
	// When reading from file and id does not match,
	// add 'deleted' members to match up with the id.
	int hasId = false;
	const int* const oldIdPtr = (int*)member_Get(obj, TMP_ID_PROPID);
	const int oldId = oldIdPtr == NULL ? id : *oldIdPtr;
	int newId = id;
	while (oldId > newId) {
		// The gcstack malloc sets everything to 0 so we do not need
		// to set the members here.
		member_GcAlloc(g->members);
		newId++;
	}
	if (newId - id > 0)
	{
		// Update the indices of deleted members to include the 'fakes' that has been added.
		group* addedIds = group_InitWithValues(group_GcAlloc(NULL), 2, (int[]){id, newId});
		group* newDeleted = group_GcOr(NULL, g->m_deletedMembers, addedIds);
		group* oldDeleted = g->m_deletedMembers;
		g->m_deletedMembers = newDeleted;
		group_Delete(addedIds);
		free(addedIds);
		group_Delete(oldDeleted);
		free(oldDeleted);
		hasId = true;
	}
	
	id = newId;
	
	// Make sure it contains no id.
	member_Set(obj, TMP_ID_PROPID, NULL);
	
	// If the member came with fixed id, then don't reuse an old one.
	if (!hasId && g->m_deletedMembers->length > 0)
	{
		// Reuse an existing position.
		gop_CreateMemberArray(g);
		id = group_PopEnd(g->m_deletedMembers);
		new = member_InitWithMember(g->m_memberArray[id], obj);
	}
	else
	{
		// There is no free positions, so we allocate new.
		new = member_InitWithMember(member_GcAlloc(g->members), obj);
	}
	
	// Reinitialize the input so one can continue using same object to insert data.
	member_Init(obj);
	
	// Prepare bitstreams to be searched.
	gop_CreateBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	int propId;
	int index;
	
	group* a;
	group* b;
	group* c;
	b = group_InitWithValues
	(group_GcAlloc(gc), 2, (const int[]){id, id+1});
	
	macro_hashTable_foreach(new) {
		propId = macro_hashTable_id(new);
		index = propId%TYPE_STRIDE;
		
		a = g->m_bitstreamsArray[index];
		if (a == NULL) continue;
		
		c = group_GcOr(gc, a, b);
		// Switch stacks so the new one is kept.
		gcstack_Swap(c, a);
	} macro_bitstream_end_foreach(new)
	
	gcstack_Delete(gc);
	free(gc); 
	
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	return id;
}

//
// This method sets all variables within a bitstream to a value.
//
void gop_SetDouble
(gop* const g, const group* const a, const int propId, const double val)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_DOUBLE));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	int i;
	hash_table* obj;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		member_SetDouble(obj, propId, val);
	} macro_bitstream_end_foreach(a)
	
	// Double does not have a default value, so we need no condition here.
	gop_CreateBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	group* c = group_GcOr(NULL, b, a);
	gcstack_Swap(c, b);
	group_Delete(b);
	free(b);
}

double gop_GetDouble(gop* const g, const int propId, const int id)
{
	macro_err_return_zero(g == NULL);
	macro_err_return_zero(propId < 0);
	macro_err_return_zero(id >= g->members->length);
	macro_err_return_zero(id < 0);
	macro_err_return_zero(!gop_IsPropertyType(propId, TYPE_DOUBLE));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	hash_table* hs = g->m_memberArray[id];
	const double* const ptr = member_Get(hs, propId);
	if (ptr == NULL) return 0.0;
	
	return *ptr;
}

//
// This method sets all variables within a bitstream to a value.
//
void gop_SetString
(gop* const g, const group* const a, const int propId, 
 const char* const val)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_STRING));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	int i;
	hash_table* obj;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		member_SetString(obj, propId, val);
	} macro_bitstream_end_foreach(a)
	
	gop_CreateBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	
	// If the string is NULL, then it is a default value
	// and we subtract from the bitstream instead of adding.
	group* c = val == NULL ?
	group_GcExcept(NULL, b, a) :
	group_GcOr(NULL, b, a);
	
	gcstack_Swap(c, b);
	group_Delete(b);
	free(b);
}

const char* gop_GetString(gop* const g, const int propId, const int id)
{
	macro_err_return_zero(g == NULL);
	macro_err_return_zero(propId < 0);
	macro_err_return_zero(id >= g->members->length);
	macro_err_return_zero(id < 0);
	macro_err_return_zero(!gop_IsPropertyType(propId, TYPE_STRING));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	hash_table* hs = g->m_memberArray[id];
	return member_Get(hs, propId);
}

void gop_SetInt
(gop* const g, const group* const a, const int propId, const int val)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_INT));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	const int isDefault = -1 == val;
	int i;
	hash_table* obj;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		member_SetInt(obj, propId, val);
	} macro_bitstream_end_foreach(a)
	
	gop_CreateBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	
	// If the string is NULL, then it is a default value
	// and we subtract from the bitstream instead of adding.
	group* c = isDefault ?
	group_GcExcept(NULL, b, a) :
	group_GcOr(NULL, b, a);
	
	gcstack_Swap(c, b);
	group_Delete(b);
	free(b);
}

int gop_GetInt(gop* const g, const int propId, const int id)
{
	macro_err_return_zero(g == NULL);
	macro_err_return_zero(propId < 0);
	macro_err_return_zero(id >= g->members->length);
	macro_err_return_zero(id < 0);
	macro_err_return_zero(!gop_IsPropertyType(propId, TYPE_INT));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	hash_table* hs = g->m_memberArray[id];
	const int* const ptr = member_Get(hs, propId);
	if (ptr == NULL) return -1;
	
	return *ptr;
}

void gop_SetBool
(gop* const g, const group* const a, const int propId, const int val)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_BOOL));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	int isDefault = 0 == val;
	int i;
	hash_table* obj;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		member_SetBool(obj, propId, val);
	} macro_bitstream_end_foreach(a)
	
	gop_CreateBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	
	// If the string is NULL, then it is a default value
	// and we subtract from the bitstream instead of adding.
	group* c = isDefault ?
	group_GcExcept(NULL, b, a) :
	group_GcOr(NULL, b, a);
	
	gcstack_Swap(c, b);
	group_Delete(b);
	free(b);
}

int gop_GetBool(gop* const g, const int propId, const int id)
{
	macro_err_return_zero(g == NULL);
	macro_err_return_zero(propId < 0);
	macro_err_return_zero(id >= g->members->length);
	macro_err_return_zero(id < 0);
	macro_err_return_zero(!gop_IsPropertyType(propId, TYPE_BOOL));
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	hash_table* hs = g->m_memberArray[id];
	const int* const ptr = member_Get(hs, propId);
	if (ptr == NULL) return false;
	
	return *ptr;
}

void gop_SetArray
(gop* const g, const group* const a, const int propId, 
 const int n, const void* const values)
{
	if (gop_IsPropertyType(propId, TYPE_BOOL))
	    groups_array_SetBoolArray(g, a, propId, n, values);
	else if (gop_IsPropertyType(propId, TYPE_DOUBLE))
		groups_array_SetDoubleArray(g, a, propId, n, values);
	else if (gop_IsPropertyType(propId, TYPE_INT))
		groups_array_SetIntArray(g, a, propId, n, values);
	else if (gop_IsPropertyType(propId, TYPE_STRING))
		groups_array_SetStringArray
		(g, a, propId, n, (const char**)values);
}

void gop_FillArray
(gop* const g, const group* const a, const int propId, 
 const int arrc, void* const arr)
{
	if (gop_IsPropertyType(propId, TYPE_BOOL))
		groups_array_FillBoolArray(g, a, propId, arrc, arr);
	else if (gop_IsPropertyType(propId, TYPE_INT))
		groups_array_FillIntArray(g, a, propId, arrc, arr);
	else if (gop_IsPropertyType(propId, TYPE_DOUBLE))
		groups_array_FillDoubleArray(g, a, propId, arrc, arr);
	else if (gop_IsPropertyType(propId, TYPE_STRING))
		groups_array_FillStringArray(g, a, propId, arrc, 
					     (const char**)arr);
}

const char* gop_PropertyNameById
(const gop* const g, const int propId)
{
	macro_err_return_null(g == NULL);
	macro_err_return_null(propId < 0);
	
	property* prop;
	const gcstack_item* cursor = g->properties->root->next;
	for (; cursor != NULL; cursor = cursor->next)
	{
		prop = (property*)cursor;
		if (prop->propId == propId) 
			return prop->name;
	}
	return NULL;
}

void gop_PrintMember
(const gop* const g, const hash_table* const obj)
{
	macro_err_return(g == NULL);
	macro_err_return(obj == NULL);
	
	int propId, type;
	
	macro_hashTable_foreach(obj) {
		propId = macro_hashTable_id(obj);
		type = propId/TYPE_STRIDE;
		const char* name = gop_PropertyNameById(g, propId);
		if (type == TYPE_DOUBLE)
			printf("%s:%lg ", name, macro_hashTable_double(obj));
		else if (type == TYPE_INT)
			printf("%s:%i ", name, macro_hashTable_int(obj));
		else if (type == TYPE_BOOL)
			printf("%s:%i ", name, macro_hashTable_bool(obj));
		else if (type == TYPE_STRING)
			printf("%s:%s ", name, macro_hashTable_string(obj));
	} macro_bitstream_end_foreach(obj)
	printf("\r\n");
}

void gop_RemoveMember(gop* const g, const int index)
{
	macro_err_return(g == NULL);
	macro_err_return(index < 0);
	
	gop_CreateMemberArray(g);
	
	hash_table* const obj = g->m_memberArray[index];
	gcstack* const gc = gcstack_Init(gcstack_Alloc());
	
	int propId;
	group* a;
	group* b = group_InitWithValues
	(group_GcAlloc(gc), 2, (int[]){index,index+1});
	group* c;
	macro_hashTable_foreach(obj) {
		propId = macro_hashTable_id(obj);
		a = getBitstream(g, propId);
		if (a == NULL) continue;
		
		c = group_GcExcept(gc, a, b);
		gcstack_Swap(c, a);
	} macro_bitstream_end_foreach(obj)
	
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	// Free the member but don't delete it, in order to maintain index.
	member_Delete(obj);
	
	// Add the member to bitstream of deleted members for reuse of index.
	group* d = g->m_deletedMembers;
	group* e = group_GcOr(gc, d, b);
	gcstack_Swap(d, e);
	g->m_deletedMembers = e;
	
	gcstack_Delete(gc);
	
	free(gc);
}

void gop_RemoveMembers(gop* const g, const group* const prop)
{
	macro_err_return(g == NULL);
	macro_err_return(prop == NULL);
	
	gop_CreateMemberArray(g);
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Remove the group from all bitstream properties.
	const gcstack_item* cursor = g->bitstreams->root->next;
	group* exProp;
	group* tmpProp;
	for (; cursor != NULL; cursor = cursor->next)
	{
		exProp = (group*)cursor;
		tmpProp = group_GcExcept(gc, exProp, prop);
		gcstack_Swap(tmpProp, exProp);
	}
	
	int index;
	hash_table* obj;
	macro_bitstream_foreach (prop) {
		// Free the member but don't delete it, in order to maintain index.
		index = macro_bitstream_pos(prop);
		obj = g->m_memberArray[index];
		member_Delete(obj);
	} macro_bitstream_end_foreach (prop)
	
	g->m_bitstreamsReady = false;
	g->m_membersReady = false;
	
	// Add the member to bitstream of deleted members for reuse of index.
	group* const d = g->m_deletedMembers;
	group* const e = group_GcOr(gc, d, prop);
	gcstack_Swap(d, e);
	g->m_deletedMembers = e;
	
	gcstack_Delete(gc);
	
	free(gc);
}

int gop_IsPropertyType(const int propId, const int type)
{
	return propId/TYPE_STRIDE == type;
}

group* gop_GcEval
(gcstack* const gc, gop* const g, const char* const expr, 
 void (* const err)(int pos, const char* message)) {
	return boolean_GcEval(gc, g, expr, err);
}
