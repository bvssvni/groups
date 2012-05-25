/*
 *  variable.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef memgroups_variable
#define memgroups_variable
	
#include "gcstack.h"

	typedef struct variable {
		gcstack_item gc;
		int propId;
		void* data;
	} variable;
	
	void variable_Delete
	(void* p);
	
	variable* variable_AllocWithGC
	(gcstack* gc);
	
	variable* variable_InitWithDouble
	(variable* var, int propId, double val);
	
#endif
	
#ifdef __cplusplus
}
#endif
