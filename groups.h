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
		member** m_memberArray;
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
	
	void groups_SetDouble
	(groups* g, const bitstream* a, int propId, double val);
	
	void groups_SetString
	(groups* g, const bitstream* a, int propId, const char* val);
	
	void groups_SetInt
	(groups* g, const bitstream* a, int propId, int val);
	
	void groups_SetBool
	(groups* g, const bitstream* a, int propId, bool val);
	
	void groups_SetDoubleArray
	(groups* g, const bitstream* a, int propId, int n, const double* values);
	
	void groups_SetStringArray
	(groups* g, const bitstream* a, int propId, int n, const char** values);
	
	void groups_SetIntArray
	(groups* g, const bitstream* a, int propId, int n, const int* values);
	
	void groups_SetBoolArray
	(groups* g, const bitstream* a, int propId, int n, const bool* values);
	
	double* groups_GetDoubleArray
	(groups* g, const bitstream* a, int propId);
	
	int* groups_GetIntArray
	(groups* g, const bitstream* a, int propId);
	
	bool* groups_GetBoolArray
	(groups* g, const bitstream* a, int propId);
	
	const char** groups_GetStringArray
	(groups* g, const bitstream* a, int propId);
	
	const char* groups_PropertyNameById
	(const groups* g, int propId);
	
	void groups_PrintMember
	(const groups* g, const member* obj);
	
	bool groups_IsDefaultVariable
	(const variable* var);
	
	//
	// Returns true if the property is of an unknown data type.
	//
	bool groups_IsUnknown
	(int propId);
	
	bool groups_IsDouble
	(int propId);
	
	bool groups_IsInt
	(int propId);
	
	bool groups_IsString
	(int propId);
	
	bool groups_IsBool
	(int propId);
	
#endif
	
#ifdef __cplusplus
}
#endif
