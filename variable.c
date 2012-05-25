/*
 *  variable.c
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

#include "variable.h"

void variable_Delete(void* p)
{
	variable* var = (variable*)p;
	if (var->data != NULL)
	{
		free(var->data);
		var->data = NULL;
	}
	var->propId = -1;
}

variable* variable_AllocWithGC(gcstack* gc)
{
	return (variable*)gcstack_malloc(gc, sizeof(variable), variable_Delete);
}

variable* variable_InitWithDouble(variable* var, int propId, double val)
{
	double* container = malloc(sizeof(double));
	*container = val;
	var->propId = propId;
	var->data = container;
	return var;
}

variable* variable_InitWithString(variable* var, int propId, const char* val)
{
	char* str = malloc(sizeof(char)*strlen(val));
	strcpy(str, val);
	var->propId = propId;
	var->data = str;
	return var;
}

variable* variable_InitWithInt(variable* var, int propId, int val)
{
	int* container = malloc(sizeof(int));
	*container = val;
	var->propId = propId;
	var->data = container;
	return var;
}
