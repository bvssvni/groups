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
	
	typedef struct groups groups;
	struct groups {
		gcstack_item gc;
		gcstack* bitstreams;
		gcstack* properties;
		bool m_sorted;
		gcstack_item** m_sortedPropertyItems;
	};
	
	typedef struct property property;
	struct property {
		gcstack_item gc;
		char* name;
		int propId;
	};
	
	void property_Delete
	(void* p);
	
	property* property_AllocWithGC
	(gcstack* gc);
	
	property* property_InitWithNameAndId
	(property* prop, char const* name, int propId);
	
	void groups_Delete
	(void* p);
	
	groups* groups_AllocWithGC
	(gcstack* gc);
	
	groups* groups_Init
	(groups* g);
	
	int groups_AddProperty
	(groups* g, char const* name);
	
	int groups_GetProperty
	(groups* g, char const* name);
	
#endif
	
#ifdef __cplusplus
}
#endif
