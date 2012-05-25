/*
 *  property.c
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
#include "property.h"


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

