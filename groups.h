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
	
#ifndef memgroups_groups_internal
#undef private
#define private const
#else
#undef private
#define private
#endif
	
	//
	//      PROPERTY STRUCTURE
	//
	//      This is a sub structure of the Groups structure.
	//
	typedef struct property {
		private gcstack_item gc;
		private char* private name;
		private int propId;
	} private property;
	
	void property_Delete
	(void* const p);
	
	property* property_AllocWithGC
	(gcstack* const gc);
	
	property* property_InitWithNameAndId
	(property* const prop, char const* name, const int propId);
	
	//
	//      GROUPS STRUCTURE
	//
	typedef struct groups {
		// Allow struct to be garbage collected by gcstack.
		private gcstack_item gc;
		
		// Bitstream data.
		private gcstack* private bitstreams;
		private int m_bitstreamsReady;
		private bitstream** private m_bitstreamsArray;
		private bitstream* private m_deletedBitstreams;
		
		// Property data.
		private gcstack* private properties;
		private int m_propertiesReady;
		private gcstack_item** private m_sortedPropertyItems;
		
		// Member data.
		private gcstack* private members;
		private int m_membersReady;
		private hash_table** private m_memberArray;
		private bitstream* private m_deletedMembers;
	} groups;
	
	//
	//		Deletes the data in Groups, but not the pointer to it.
	//
	void groups_Delete
	(void* const p);
	
	//
	//		Groups supports stack-based garbage collection.
	//
	groups* groups_AllocWithGC
	(gcstack* const gc);
	
	//
	//		Initialized Groups with empty dataset.
	//
	groups* groups_Init
	(groups* const g);
	
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
	(groups* const g, const void* const name, const void* const propType);
	
	//
	//		Returns a property id by name.
	//		This algorithm has O(log N) worst case.
	//
	int groups_GetProperty
	(groups* const g, const char* const name);
	
	//
	//      Returns an array of property names.
	//
	const char** groups_GetPropertyNames
	(groups* const g);
	
	//
	//	The resulted bitstream represents a selection of objects that
	//	has a property. This is like taking a snapshot of the group
	//	in that moment.
	//
	bitstream* groups_GetBitstream
	(gcstack* const gc, groups* const g, const int propId);
	
	//
	//      This method returns a bitstream containing all members.
	//      It takes the whole range from 0 to the length of member stack,
	//      and subtracts the deleted members with exclude.
	//
	bitstream* groups_GetAll
	(gcstack* const gc, groups* const g);
	
	//
	// Removes the bitstream, but not the data itself from the members.
	// After removing a property, changes to the data will no longer be
	// monitored and kept as bitstream.
	//
	void groups_RemoveProperty
	(groups* const g, const int propId);
	
	//
	// Finds property name by id.
	// This has O(N) worst case, if you need all names, then read from
	// the struct instead.
	//
	const char* groups_PropertyNameById
	(const groups* const g, const int propId);
	
	//
	// IMPORTANT!
	// This method erases the information from the obj parameter after inserting.
	// Groups reuses member objects for stability and minimal memory.
	// You are not supposed to set variables on the member locally after adding.
	// You need only to allocate one member to read from a file or table,
	// because when Groups resets the data, you need no worries about memory leaks.
	//
	int groups_AddMember
	(groups* const g, hash_table* const obj);
	
	//
	// Removes a member from Groups and recycles it for reuse.
	//
	void groups_RemoveMember
	(groups* const g, const int index);
	
	//
	// Removes members by a bitstream. This is way faster than removing each one.
	// Try using bitstreams whenever you can, it makes your code easier to reuse.
	//
	void groups_RemoveMembers
	(groups* const g, const bitstream* const a);
	
	void groups_SetDouble
	(groups* const g, const bitstream* const a, const int propId, 
	 const double val);
	
	void groups_SetString
	(groups* const g, const bitstream* const a, const int propId, 
	 const char* const val);
	
	void groups_SetInt
	(groups* const g, const bitstream* const a, const int propId, 
	 const int val);
	
	void groups_SetBool
	(groups* const g, const bitstream* const a, const int propId, 
	 const int val);
	
	//
	// Sets an array of doubles.
	// The values need to be in the same order as when extracted.
	// Double has no default value, so it is faster than int because
	// it doesn't need to check.
	//
	void groups_SetDoubleArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const double* const values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (NULL), then it will delete the property from member.
	// This is to reduce usage of memory.
	//
	void groups_SetStringArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const char** const values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (-1), then it will delete the property from member.
	// This is to reduce usage of memory.
	//
	void groups_SetIntArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const int* const values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (0), then it will delete the property from member.
	// This is to reduce usage of memory.
	//
	void groups_SetBoolArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const int* const values);
	
	double* groups_GetDoubleArray
	(groups* const g, const bitstream* const a, const int propId);
	
	int* groups_GetIntArray
	(groups* const g, const bitstream* const a, const int propId);
	
	int* groups_GetBoolArray
	(groups* const g, const bitstream* const a, const int propId);
	
	const char** groups_GetStringArray
	(groups* const g, const bitstream* const a, const int propId);
	
	//
	// Prints a member to the console window with property names and values.
	// A member has no clue what the property names are, therefore it
	// has to be a Groups present to print it.
	//
	void groups_PrintMember
	(const groups* const g, const hash_table* const obj);
	
	//
	// Returns true if the variable got default value.
	//
	int groups_IsDefaultVariable
	(const int propId, void* const data);
	
	//
	// Returns true if the property is of an unknown data type.
	//
	int groups_IsUnknown
	(const int propId);
	
	int groups_IsDouble
	(const int propId);
	
	int groups_IsInt
	(const int propId);
	
	int groups_IsString
	(const int propId);
	
	int groups_IsBool
	(const int propId);
	
	// Saves data to file in JSON format.
	int        groups_SaveToFile
	(groups* const g, const char* const fileName);
	
	//
	//	Reads data from a string.
	//
	int groups_ReadFromString
	(groups* const g, const char* const text, const int verbose, 
	 void(* const err)(int line, int column, const char* message));
	
	//
	// 	Reads data from a file in JSON format.
	//
	int groups_ReadFromFile
	(
	 groups* g, 
	 const char* fileName, 
	 const int verbose, 
	 void(* const err)(const int line, const int column, const char* const message)
	 );
	
	//
	//	Run unit tests for groups.
	//
	void groups_RunUnitTests
	(void);
	
#endif
	
#ifdef __cplusplus
}
#endif
