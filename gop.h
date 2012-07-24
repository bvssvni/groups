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
	
	//
	//      PROPERTY STRUCTURE
	//
	//      This is a sub structure of the Groups structure.
	//
	typedef struct property {
		gcstack_item gc;
		char* name;
		int propId;
	} property;
	
	void property_Delete
	(void* const p);
	
	property* property_GcAlloc
	(gcstack* const gc);
	
	property* property_InitWithNameAndId
	(property* const prop, char const* name, const int propId);
	
	//
	//      GROUPS STRUCTURE
	//
	typedef struct gop {
		// Allow struct to be garbage collected by gcstack.
		gcstack_item gc;
		
		// Bitstream data.
		gcstack* bitstreams;
		int m_bitstreamsReady;
		group** m_bitstreamsArray;
		group* m_deletedBitstreams;
		
		// Property data.
		gcstack* properties;
		int m_propertiesReady;
		gcstack_item** m_sortedPropertyItems;
		
		// Member data.
		gcstack* members;
		int m_membersReady;
		hash_table** m_memberArray;
		group* m_deletedMembers;
	} gop;
	
	//
	//		Deletes the data in Groups, but not the pointer to it.
	//
	void groups_Delete
	(void* const p);
	
	//
	//		Groups supports stack-based garbage collection.
	//
	gop* groups_GcAlloc
	(gcstack* const gc);
	
	//
	//		Initialized Groups with empty dataset.
	//
	gop* groups_Init
	(gop* const g);
	
	//
	//		Use this method to add properties to Groups.
	//		This is usually done at the startup of the program.
	//		All objects that has a property found in the collection
	//		will be monitored when you change the value.
	//		All objects that has a property can be fast accessed with bitstreams.
	//
	//		You need to specify name and type.
	//		Supported types:
	//
	//	TYPE		DEFAULT (FALSE)		NOTES
	//	bool		0			int as native type
	//	int		-1			Used for relations
	//	double		has no default		Used for numbers
	//	string		NULL			const char*
	//
	int groups_AddProperty
	(gop* const g, const void* const name, const void* const propType);
	
	//
	//		Returns a property id by name.
	//		This algorithm has O(log N) worst case.
	//
	int groups_GetProperty
	(gop* const g, const char* const name);
	
	//
	//      Returns an array of property names.
	//
	const char** groups_GetPropertyNames
	(gop* const g);
	
	//
	//	The resulted bitstream represents a selection of objects that
	//	has a property. This is like taking a snapshot of the group
	//	in that moment.
	//
	group* groups_GcGetBitstream
	(gcstack* const gc, gop* const g, const int propId);
	
	//
	//      This method returns a bitstream containing all members.
	//      It takes the whole range from 0 to the length of member stack,
	//      and subtracts the deleted members with exclude.
	//
	group* groups_GcGetAll
	(gcstack* const gc, gop* const g);
	
	//
	// Removes the bitstream, but not the data itself from the members.
	// After removing a property, changes to the data will no longer be
	// monitored and kept as bitstream.
	//
	void groups_RemoveProperty
	(gop* const g, const int propId);
	
	//
	// Finds property name by id.
	// This has O(N) worst case, if you need all names, then read from
	// the struct instead.
	//
	const char* groups_PropertyNameById
	(const gop* const g, const int propId);
	
	//
	// IMPORTANT!
	// This method erases the information from the obj parameter after inserting.
	// Groups reuses member objects for stability and minimal memory.
	// You are not supposed to set variables on the member locally after adding.
	// You need only to allocate one member to read from a file or table,
	// because when Groups resets the data, you need no worries about memory leaks.
	//
	int groups_AddMember
	(gop* const g, hash_table* const obj);
	
	//
	// Removes a member from Groups and recycles it for reuse.
	//
	void groups_RemoveMember
	(gop* const g, const int index);
	
	//
	// Removes members by a bitstream. This is way faster than removing each one.
	// Try using bitstreams whenever you can, it makes your code easier to reuse.
	//
	void groups_RemoveMembers
	(gop* const g, const group* const a);
	
	void groups_SetDouble
	(gop* const g, const group* const a, const int propId, 
	 const double val);
	
	void groups_SetString
	(gop* const g, const group* const a, const int propId, 
	 const char* const val);
	
	const char* groups_GetString
	(gop* const g, const int propId, const int id);
	
	void groups_SetInt
	(gop* const g, const group* const a, const int propId, 
	 const int val);
	
	int groups_GetInt(gop* const g, const int propId, const int id);
	
	void groups_SetBool
	(gop* const g, const group* const a, const int propId, 
	 const int val);
	
	int groups_GetBool(gop* const g, const int propId, const int id);
	
	double groups_GetDouble
	(gop* const g, const int propId, const int id);
	
	void groups_PrintMember
	(const gop* const g, const hash_table* const obj);
	
	int groups_IsDefaultVariable
	(const int propId, void* const data);
	
	int groups_IsPropertyType(const int propId, const int type);
	
	void groups_CreateBitstreamArray(gop* const g);
	void groups_CreateMemberArray(gop* g);
	
	void groups_SetArray
	(gop* const g, const group* const a, const int propId, 
	 const int n, const void* const values);
	
	void groups_FillArray
	(gop* const g, const group* const a, const int propId, 
	 const int arrc, void* const arr);
	
	group* groups_GcEval
	(gcstack* const gc, gop* const g, const char* const expr, 
	 void (* const err)(int pos, const char* message));
#endif
	
#ifdef __cplusplus
}
#endif
