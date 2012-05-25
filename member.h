/*
 *  member.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef memgroups_member
#define memgroups_member
	
#include "gcstack.h"
#include "variable.h"
#include "readability.h"
	
typedef struct member {
	gcstack_item gc;
	gcstack* variables;
	bool m_ready;
	variable** m_variableArray;
} member;

	void member_Delete
	(void* p);
	
	member* member_AllocWithGC
	(gcstack* gc);
	
	member* member_Init
	(member* obj);
	
	void member_AddDouble
	(member* obj, int propId, double val);
	
	void member_AddString
	(member* obj, int propId, char const* val);
	
	void member_AddInt
	(member* obj, int propId, int val);
	
	void member_AddBool
	(member* obj, int propId, bool val);
	
	//
	// Uses binary search to find the index of a property id.
	//
	int member_IndexOf
	(member* obj, int propId);
	
#endif

#ifdef __cplusplus
}
#endif
