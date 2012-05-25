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
}

variable* variable_AllocWithGC(gcstack* gc)
{
	return (variable*)gcstack_malloc(gc, sizeof(variable), variable_Delete);
}

variable* variable_InitWithDouble(variable* var, double val)
{
	double* container = malloc(sizeof(double));
	*container = val;
	var->data = container;
	return var;
}
