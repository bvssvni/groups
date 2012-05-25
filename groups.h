/*
 *  groups.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef memgroups_groups
#define memgroups_groups
	
#include "gcstack.h"
#include "bitstream.h"
#include "member.h"
#include "readability.h"
	
	typedef struct groups {
		gcstack_item gc;
		
		// Bitstream data.
		gcstack* bitstreams;
		bool m_ready;
		bitstream** m_bitstreamsArray;
		
		// Property data.
		gcstack* properties;
		bool m_sorted;
		gcstack_item** m_sortedPropertyItems;
		
		// Member data.
		gcstack* members;
		bool m_membersReady;
		gcstack** m_memberArray;
		bitstream* m_deletedMembers;
	} groups;
	
	int groups_AddProperty
	(groups* g, const void* name, const void* propType);
	
	void groups_Delete
	(void* p);
	
	groups* groups_AllocWithGC
	(gcstack* gc);
	
	groups* groups_Init
	(groups* g);
	
	int groups_GetProperty
	(groups* g, char const* name);
	
	bitstream* groups_GetBitstream
	(groups* g, int propId);
	
	void groups_RemoveProperty
	(groups* g, int propId);
	
	int compareStringVSProperty
	(const void* a, const void* b);
	
	int groups_AddMember
	(groups* g, member* obj);
	
	bool groups_IsDefaultVariable
	(variable* var);
	
	bool groups_IsDouble
	(int propId);
	
	bool groups_IsInt
	(int propId);
	
	bool groups_IsString
	(int propId);
	
#endif
	
#ifdef __cplusplus
}
#endif
