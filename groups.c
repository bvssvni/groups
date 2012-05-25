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

#include "groups.h"

void property_Delete(void* p)
{
	property* prop = (property*)p;
	if (prop->name != NULL)
		free(prop->name);
}

property* property_AllocWithGC(gcstack* gc)
{
	return (property*)gcstack_malloc(gc, sizeof(property), property_Delete);
}

property* property_InitWithNameAndId(property* prop, char const* name, int propId)
{
	int nameLength = strlen(name);
	char* newName = malloc(sizeof(char)*nameLength);
	prop->name = strcpy(newName, name);
	prop->propId = propId;
	return prop;
}

void groups_Delete(void* p)
{
	groups* g = (groups*)p;
	gcstack* bitstreams = g->bitstreams;
	if (bitstreams != NULL)
	{
		gcstack_Delete(bitstreams);
		free(bitstreams);
		g->bitstreams = NULL;
	}
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
	return g;
}

int groups_AddProperty(groups* g, char const* name)
{
	// Use the id of the last added property.
	int propId = 0;
	property* lastAdded = (property*)g->properties->root->next;
	if (lastAdded != NULL)
		propId = lastAdded->propId+1;
	
	// Create new property that links name to id.
	property_InitWithNameAndId
	(property_AllocWithGC(g->properties), name, propId);
	
	// Create a new empty bitstream for that property.
	bitstream_InitWithSize(bitstream_AllocWithGC(g->bitstreams), 0);
	
	g->m_sorted = false;
	
	return propId;
}

//
// The first argument is string.
// The second argument is property.
//
int compareStringVSProperty(void const* a, void const* b)
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
}


int groups_GetProperty(groups* g, char const* name)
{
	if (!g->m_sorted) sortProperties(g);
	g->m_sorted = true;
	
	int length = g->properties->length;
	gcstack_item** items = g->m_sortedPropertyItems;
	
	int index = sorting_SearchBinary(length, (void*)items, name, compareStringVSProperty);
	
	if (index < 0) return -1;
	
	// Return the property id.
	property* prop = (property*)items[index];
	return prop->propId;
}

