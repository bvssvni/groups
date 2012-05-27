/*
 *  groups.h
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
		bool m_bitstreamsReady;
		bitstream** m_bitstreamsArray;
		bitstream* m_deletedBitstreams;
		
		// Property data.
		gcstack* properties;
		bool m_propertiesReady;
		gcstack_item** m_sortedPropertyItems;
		
		// Member data.
		gcstack* members;
		bool m_membersReady;
		member** m_memberArray;
		bitstream* m_deletedMembers;
	} groups;
	
	void groups_Delete
	(void* p);
	
	groups* groups_AllocWithGC
	(gcstack* gc);
	
	groups* groups_Init
	(groups* g);
	
	int groups_AddProperty
	(groups* g, const void* name, const void* propType);
	
	int groups_GetProperty
	(groups* g, char const* name);
	
	bitstream* groups_GetBitstream
	(groups* g, int propId);
	
	//
	// Removes the bitstream, but not the data itself from the members.
	// After removing a property, changes to the data will no longer be
	// monitored and kept as bitstream.
	//
	void groups_RemoveProperty
	(groups* g, int propId);
	
	void groups_RemoveMember
	(groups* g, int index);
	
	void groups_RemoveMembers
	(groups* g, bitstream const* a);
	
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
