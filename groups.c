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
	
	// Add an extra value to the property id for type checking.
	if (strcmp(propType, "double") == 0)
		propId += TYPE_DOUBLE*TYPE_STRIDE;
	else if (strcmp(propType, "string") == 0)
		propId += TYPE_STRING*TYPE_STRIDE;
	else if (strcmp(propType, "int") == 0)
		propId += TYPE_INT*TYPE_STRIDE;
	else
		propId += TYPE_UNKNOWN*TYPE_STRIDE;
	
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
	while (cursor != NULL)
	{
		variable* var = (variable*)cursor;
		propId = var->propId;
		
		index = propId%TYPE_STRIDE;
		a = g->m_bitstreamsArray[index];
		
		c = bitstream_Or(gc, a, b);
		
		// Switch stacks so the new one is kept.
		gcstack_Swap(c, a);
		
		cursor = cursor->next;
	}
	
	gcstack_Delete(gc);
	free(gc);
	
	/*
	// TEST
	if (c->gc.free != NULL)
	{
		printf("c->gc.free != NULL\r\n");
		exit(1);
	}
	 */
	
	return id;
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
