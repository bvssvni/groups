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
#include "hashtable.h"
	
	typedef struct groups {
		// Allow struct to be garbage collected by gcstack.
		gcstack_item gc;
		
		// Bitstream data.
		gcstack* bitstreams;
		int m_bitstreamsReady;
		bitstream** m_bitstreamsArray;
		bitstream* m_deletedBitstreams;
		
		// Property data.
		gcstack* properties;
		int m_propertiesReady;
		gcstack_item** m_sortedPropertyItems;
		
		// Member data.
		gcstack* members;
		int m_membersReady;
		hash_table** m_memberArray;
		bitstream* m_deletedMembers;
	} groups;
	
	//
	//		Deletes the data in Groups, but not the pointer to it.
	//
	void groups_Delete
	(void* p);
	
	//
	//		Groups supports stack-based garbage collection.
	//
	groups* groups_AllocWithGC
	(gcstack* gc);
	
	//
	//		Initialized Groups with empty dataset.
	//
	groups* groups_Init
	(groups* g);
	
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
	//			TYPE		DEFAULT (FALSE)		NOTES
	//			bool		0					int as native type
	//			int			-1					Used for relations
	//			double		has no default		Used for numbers
	//			string		NULL				const char*
	//
	int groups_AddProperty
	(groups* g, const void* name, const void* propType);
	
	//
	//		Returns a property id by name.
	//		This algorithm has O(log N) worst case.
	//
	int groups_GetProperty
	(groups* g, char const* name);
    
    //
    //      Returns an array of property names.
    //
    char** groups_GetPropertyNames
    (groups* g);
	
	//
	//		Bitstream is the object you use to group objects fast.
	//		Each property has it's own bitstream, but it changes
	//		each time you change a variable or remove members.
	//		The bitstream you get is guaranteed not to change,
	//		but you need to make sure that when you update
	//		that Groups is in a state compatible with your update.
	//
	//		For example, if you delete an object and then try to update it,
	//		that is no good.
	//
	bitstream* groups_GetBitstream
	(groups* g, int propId);
	
	//
	// Removes the bitstream, but not the data itself from the members.
	// After removing a property, changes to the data will no longer be
	// monitored and kept as bitstream.
	//
	void groups_RemoveProperty
	(groups* g, int propId);
	
	//
	// Finds property name by id.
	// This has O(N) worst case, if you need all names, then read from
	// the struct instead.
	//
	const char* groups_PropertyNameById
	(const groups* g, int propId);
	
	//
	// IMPORTANT!
	// This method erases the information from the obj parameter after inserting.
	// Groups reuses member objects for stability and minimal memory.
	// You are not supposed to set variables on the member locally after adding.
	// You need only to allocate one member to read from a file or table,
	// because when Groups resets the data, you need no worries about memory leaks.
	//
	int groups_AddMember
	(groups* g, hash_table* obj);
	
	//
	// Removes a member from Groups and recycles it for reuse.
	//
	void groups_RemoveMember
	(groups* g, int index);
	
	//
	// Removes members by a bitstream. This is way faster than removing each one.
	// Try using bitstreams whenever you can, it makes your code easier to reuse.
	//
	void groups_RemoveMembers
	(groups* g, bitstream const* a);
	
	void groups_SetDouble
	(groups* g, const bitstream* a, int propId, double val);
	
	void groups_SetString
	(groups* g, const bitstream* a, int propId, const char* val);
	
	void groups_SetInt
	(groups* g, const bitstream* a, int propId, int val);
	
	void groups_SetBool
	(groups* g, const bitstream* a, int propId, int val);
	
	//
	// Sets an array of doubles.
	// The values need to be in the same order as when extracted.
	// Double has no default value, so it is faster than int because
	// it doesn't need to check.
	//
	void groups_SetDoubleArray
	(groups* g, const bitstream* a, int propId, int n, const double* values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (NULL), then it will delete the property from member.
	// This is to reduce usage of memory.
	//
	void groups_SetStringArray
	(groups* g, const bitstream* a, int propId, int n, const char** values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (-1), then it will delete the property from member.
	// This is to reduce usage of memory.
	//
	void groups_SetIntArray
	(groups* g, const bitstream* a, int propId, int n, const int* values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (0), then it will delete the property from member.
	// This is to reduce usage of memory.
	//
	void groups_SetBoolArray
	(groups* g, const bitstream* a, int propId, int n, const int* values);
	
	double* groups_GetDoubleArray
	(groups* g, const bitstream* a, int propId);
	
	int* groups_GetIntArray
	(groups* g, const bitstream* a, int propId);
	
	int* groups_GetBoolArray
	(groups* g, const bitstream* a, int propId);
	
	const char** groups_GetStringArray
	(groups* g, const bitstream* a, int propId);
	
	//
	// Prints a member to the console window with property names and values.
	// A member has no clue what the property names are, therefore it
	// has to be a Groups present to print it.
	//
	void groups_PrintMember
	(const groups* g, const hash_table* obj);
	
	//
	// Returns true if the variable got default value.
	//
	int groups_IsDefaultVariable
    (int propId, void* data);
	
	//
	// Returns true if the property is of an unknown data type.
	//
	int groups_IsUnknown
	(int propId);
	
	int groups_IsDouble
	(int propId);
	
	int groups_IsInt
	(int propId);
	
	int groups_IsString
	(int propId);
	
	int groups_IsBool
	(int propId);
	
    // Saves data to file in JSON format.
    int        groups_SaveToFile
    (groups* g, const char* fileName);
    
    // Reads data from a file in JSON format.
    int groups_ReadFromFile
    (
     groups* g, 
     const char* fileName, 
     int verbose, 
     void(*err)(int line, int column, const char* message)
    );
    
#endif
	
#ifdef __cplusplus
}
#endif
