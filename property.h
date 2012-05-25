/*
 *  property.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef memgroups_property
#define memgroups_property
	
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

#endif
	
#ifdef __cplusplus
}
#endif
