/*
 *  groups.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcstack.h"
#include "bitstream.h"
#include "sorting.h"
#include "readability.h"
#include "property.h"

#include "groups.h"

#define TYPE_STRIDE 1000000
#define TYPE_UNKNOWN 0
#define TYPE_DOUBLE 1
#define TYPE_STRING 2
#define TYPE_INT 3
#define TYPE_BOOL 4

////////////////// Macro to iterate through bitstream //////////////

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

#define end_foreach }}__BREAK_BITSTREAM_prop:;

#define _break(a)     goto __BREAK_BITSTREAM_##a

#define _pos(a)    __j##a

//////////////////////////////////////////////////////////////////

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
	bitstream_Delete(g->m_deletedMembers);
	free(g->m_deletedMembers);
}

groups* groups_AllocWithGC(gcstack* gc)
{
	return (groups*)gcstack_malloc(gc, sizeof(groups), groups_Delete);
}

groups* groups_Init(groups* g)
{
	g->bitstreams = gcstack_Init(gcstack_Alloc());
	g->properties = gcstack_Init(gcstack_Alloc());
	g->m_sorted = false;
	g->m_sortedPropertyItems = NULL;
	g->m_ready = false;
	g->m_bitstreamsArray = NULL;
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
	return strcmp(aName, bName);
}


void sortProperties(groups* g)
{
	if (g->m_sorted) return;
	
	int length = g->properties->length;
	gcstack_item** items;
	items = gcstack_CreateItemsArray(g->properties);
	property* t = malloc(sizeof(property));
	
	// Use QuickSort to sort the properties in right order.
	sorting_Sort((void*)items, 0, length, sizeof(property), t, (void*)compareProperties);
	
	free(t);
	if (g->m_sortedPropertyItems != NULL)
		free(g->m_sortedPropertyItems);
	g->m_sortedPropertyItems = items;
	
	g->m_sorted = true;
}

int groups_AddProperty(groups* g, const void* name, const void* propType)
{
	// We use the length of the bitstream stack to generate ids,
	// because those bitstreams are set to empty instead of deleted.
	int propId = g->bitstreams->length;
	
	// Here we do a binary search trick to avoid some calls to strcmp.
	// You just build the code like a search tree and it will run faster.
	//
	// bool (node
	// double (node)
	// int <- root
	// string (node)
	//
	int intCheck = strcmp(propType, "int");
	if (intCheck == 0) {
		propId += TYPE_INT*TYPE_STRIDE;
	}
	if (intCheck < 0) {
		if (strcmp(propType, "bool") == 0)
			propId += TYPE_BOOL*TYPE_STRIDE;
		else if (strcmp(propType, "double") == 0)
			propId += TYPE_DOUBLE*TYPE_STRIDE;
		else {
			propId += TYPE_UNKNOWN*TYPE_STRIDE;
		}
	}
	else {
		if (strcmp(propType, "string") == 0)
			propId += TYPE_STRING*TYPE_STRIDE;
		else {
			propId += TYPE_UNKNOWN*TYPE_STRIDE;
		}
	}
	
	// It might seem like a waste to sort before inserting,
	// but we need to check if the property already exists.
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
	
	
	// Create new property that links name to id.
	property_InitWithNameAndId
	(property_AllocWithGC(g->properties), name, propId);
	
	// Create a new empty bitstream for that property.
	bitstream_InitWithSize(bitstream_AllocWithGC(g->bitstreams), 0);
	
	g->m_sorted = false;
	g->m_ready = false;
	
	return propId;
}


int groups_GetProperty(groups* g, char const* name)
{
	sortProperties(g);
	
	int length = g->properties->length;
	gcstack_item** items = g->m_sortedPropertyItems;
	
	int index = sorting_SearchBinary(length, (void*)items, name, compareStringVSProperty);
	
	if (index < 0) return -1;
	
	// Return the property id.
	property* prop = (property*)items[index];
	return prop->propId;
}

void createBitstreamArray(groups* g)
{
	if (g->m_ready) return;
	
	// Create array of pointers to each bitstream to match the stack.
	if (g->m_bitstreamsArray != NULL)
		free(g->m_bitstreamsArray);
	g->m_bitstreamsArray = (bitstream**)gcstack_CreateItemsArray(g->bitstreams);
	g->m_ready = true;
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
	
	createBitstreamArray(g);
	
	bitstream* a = g->m_bitstreamsArray[index];
	bitstream_Delete(a);
	
	// Loop through the stack to find the property to delete.
	gcstack_item* cursor = g->properties->root->next;
	property* prop = NULL;
	while (cursor != NULL)
	{
		prop = (property*)cursor;
		if (prop->propId == propId) break;
		cursor = cursor->next;
	}
	
	if (prop == NULL) return;
	
	// Delete it, including freeing the pointer.
	gcstack_free(g->properties, prop);
}

bool groups_IsDefaultVariable(variable* var)
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

int groups_AddMember(groups* g, member* obj)
{
	int id = g->members->length;
	gcstack_Push(g->members, obj);
	
	// If the member contains no variables, skip the advanced stuff.
	if (obj->variables->length == 0)
		return id;
	
	// Prepare bitstreams to be searched.
	createBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update bitstreams.
	gcstack_item* cursor = obj->variables->root->next;
	bitstream* a;
	bitstream* b;
	bitstream* c;
	b = bitstream_InitWithValues
	(bitstream_AllocWithGC(gc), 2, (const int[]){id, id+1});
	int propId = 0;
	int index = 0;
	
	// Go through the bitstreams and update those who is contained
	for (; cursor != NULL; cursor = cursor->next)
	{
		variable* var = (variable*)cursor;
		
		// If the variable is default value, we don't add it to the bitstream.
		if (groups_IsDefaultVariable(var)) continue;
		
		propId = var->propId;
		
		index = propId%TYPE_STRIDE;
		a = g->m_bitstreamsArray[index];
		
		c = bitstream_Or(gc, a, b);
		
		// Switch stacks so the new one is kept.
		gcstack_Swap(c, a);
	}
	
	gcstack_Delete(gc);
	free(gc); 
	
	return id;
}

void createMemberArray(groups* g)
{
	if (g->m_membersReady) return;
	
	member** items = (member**)gcstack_CreateItemsArray(g->members);
	if (g->m_memberArray != NULL)
		free(g->m_memberArray);
	g->m_memberArray = items;
	
	g->m_membersReady = true;
}

//
// This method sets all variables within a bitstream to a value.
//
void groups_SetDouble(groups* g, bitstream* a, int propId, double val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	int index;
	member* obj;
	variable* var;
	double* p;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		
		index = member_IndexOf(obj, propId);
		if (index < 0) {
			// The property does not exist, so we need to add it the default way.
			member_AddDouble(obj, propId, val);
			continue;
		}
		if (index >= 0)
		{
			// Since 'member_IndexOf' creates variableArray if not created,
			// we don't have to make an extra call to do this.
			var = obj->m_variableArray[index];
			
			// We don't have to allocate new memory for double,
			// because it fits perfectly inside the existing allocated memory.
			p = var->data;
			*p = val;
		}
	} end_foreach
	
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
void groups_SetString(groups* g, bitstream* a, int propId, const char* val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	bool isDefault = NULL == val;
	int i;
	int index;
	member* obj;
	variable* var;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		
		index = member_IndexOf(obj, propId);
		if (index < 0) {
			// If the string is NULL and the member does not contain it,
			// then we don't have to do anything.
			if (!isDefault)
				// The property does not exist, so we need to add it the default way.
				member_AddString(obj, propId, val);
			continue;
		}
		if (index >= 0)
		{
			// Since 'member_IndexOf' creates variableArray if not created,
			// we don't have to make an extra call to do this.
			var = obj->m_variableArray[index];
			
			if (isDefault)
			{
				// Remove the item and free the pointer.
				// Since we are not iterating through the member properties,
				// but along the members in a group, it is safe.
				gcstack_free(obj->variables, var);
				
				// Tell the member to update the m_variableArray.
				obj->m_ready = false;
				continue;
			}
				
			// Free the old string and init with a new one.
			variable_Delete(var);
			var = variable_InitWithString(var, propId, val);
		}
	} end_foreach
	
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

void groups_SetInt(groups* g, bitstream* a, int propId, int val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	bool isDefault = -1 == val;
	int i;
	int index;
	member* obj;
	variable* var;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		
		index = member_IndexOf(obj, propId);
		if (index < 0) {
			// If the string is NULL and the member does not contain it,
			// then we don't have to do anything.
			if (!isDefault)
				// The property does not exist, so we need to add it the default way.
				member_AddInt(obj, propId, val);
			continue;
		}
		if (index >= 0)
		{
			// Since 'member_IndexOf' creates variableArray if not created,
			// we don't have to make an extra call to do this.
			var = obj->m_variableArray[index];
			
			if (isDefault)
			{
				// Remove the item and free the pointer.
				// Since we are not iterating through the member properties,
				// but along the members in a group, it is safe.
				gcstack_free(obj->variables, var);
				
				// Tell the member to update the m_variableArray.
				obj->m_ready = false;
				continue;
			}
			
			// We can just switch the value.
			int* data = var->data;
			*data = val;
		}
	} end_foreach
	
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

void groups_SetBool(groups* g, bitstream* a, int propId, bool val)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	bool isDefault = 0 == val;
	int i;
	int index;
	member* obj;
	variable* var;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		
		index = member_IndexOf(obj, propId);
		if (index < 0) {
			// If the string is NULL and the member does not contain it,
			// then we don't have to do anything.
			if (!isDefault)
				// The property does not exist, so we need to add it the default way.
				member_AddInt(obj, propId, val);
			continue;
		}
		if (index >= 0)
		{
			// Since 'member_IndexOf' creates variableArray if not created,
			// we don't have to make an extra call to do this.
			var = obj->m_variableArray[index];
			
			if (isDefault)
			{
				// Remove the item and free the pointer.
				// Since we are not iterating through the member properties,
				// but along the members in a group, it is safe.
				gcstack_free(obj->variables, var);
				
				// Tell the member to update the m_variableArray.
				obj->m_ready = false;
				continue;
			}
			
			// We can just switch the value.
			bool* data = var->data;
			*data = val;
		}
	} end_foreach
	
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
void groups_SetDoubleArray(groups* g, bitstream* a, int propId, int n, const double* values)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	int i;
	int index;
	member* obj;
	variable* var;
	double* p;
	
	// We need a counter to read the right value from the array.
	int k = 0;
	foreach (a) {
		i = _pos(a);
		obj = g->m_memberArray[i];
		
		index = member_IndexOf(obj, propId);
		if (index < 0) {
			// The property does not exist, so we need to add it the default way.
			member_AddDouble(obj, propId, values[k++]);
			continue;
		}
		if (index >= 0)
		{
			// Since 'member_IndexOf' creates variableArray if not created,
			// we don't have to make an extra call to do this.
			var = obj->m_variableArray[index];
			
			// We don't have to allocate new memory for double,
			// because it fits perfectly inside the existing allocated memory.
			p = var->data;
			*p = values[k++];
		}
	} end_foreach
	
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
(groups* g, bitstream* a, int propId, int n, const char** values)
{
	// Create member array so we can access members directly.
	createMemberArray(g);
	
	bool isDefault;
	int i;
	int index;
	member* obj;
	variable* var;
	const char* val;
	
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
		
		val = values[k++];
		isDefault = NULL == val;
		
		// Remember the indices who are not default.
		if (!isDefault)
			notDefaultIndices[notDefaultIndicesSize++] = i;
		
		index = member_IndexOf(obj, propId);
		if (index < 0) {
			// If the string is NULL and the member does not contain it,
			// then we don't have to do anything.
			if (!isDefault)
				// The property does not exist, so we need to add it the default way.
				member_AddString(obj, propId, val);
			continue;
		}
		if (index >= 0)
		{
			// Since 'member_IndexOf' creates variableArray if not created,
			// we don't have to make an extra call to do this.
			var = obj->m_variableArray[index];
			
			if (isDefault)
			{
				// Remove the item and free the pointer.
				// Since we are not iterating through the member properties,
				// but along the members in a group, it is safe.
				gcstack_free(obj->variables, var);
				
				// Tell the member to update the m_variableArray.
				obj->m_ready = false;
				continue;
			}
			
			// Free the old string and init with a new one.
			variable_Delete(var);
			var = variable_InitWithString(var, propId, val);
		}
	} end_foreach
	
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

