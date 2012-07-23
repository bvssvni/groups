//
//  groups-array.h
//  MemGroups
//
//  Created by Sven Nilsen on 24.07.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef MemGroups_groups_array_h
#define MemGroups_groups_array_h

	
	//
	// Sets an array of doubles.
	// The values need to be in the same order as when extracted.
	// Double has no default value, so it is faster than int because
	// it doesn't need to check.
	//
	void groups_array_SetDoubleArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const double* const values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (NULL), then it will delete the property 
	// from member.
	// This is to reduce usage of memory.
	//
	void groups_array_SetStringArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const char** const values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (-1), then it will delete the property from 
	// member.
	// This is to reduce usage of memory.
	//
	void groups_array_SetIntArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const int* const values);
	
	//
	// The values need to be in the same order as when extracted.
	// If the value is default (0), then it will delete the property from 
	// member.
	// This is to reduce usage of memory.
	//
	void groups_array_SetBoolArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int n, const int* const values);
	
	void groups_array_FillDoubleArray
	(groups* const g, const bitstream* const a, const int propId, 
	 const int arrc, double* const arr);
	
	void groups_array_FillIntArray
	(groups* const g, const bitstream* const a, const int propId,
	 const int arrc, int* const arr);
	
	void groups_array_FillBoolArray
	(groups* const g, const bitstream* const a, const int propId,
	 const int arrc, int* const arr);
	
	void groups_array_FillStringArray
	(groups* const g, const bitstream* const a, const int propId,
	 const int arrc, const char** const arr);

#endif

#ifdef __cplusplus
}
#endif
