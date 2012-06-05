/*
 *  variable.c
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

#ifndef MEMGROUPS_READABILITY
#define MEMGROUPS_READABILITY
#include "readability.h"
#endif

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

variable* variable_InitWithBool(variable* var, int propId, bool val)
{
	bool* container = malloc(sizeof(bool));
	*container = val;
	var->propId = propId;
	var->data = container;
	return var;
}
