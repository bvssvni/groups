//
//  groups-array.c
//  MemGroups
//
//  Created by Sven Nilsen on 24.07.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "gcstack.h"
#include "hashtable.h"
#include "group.h"
#include "gop.h"
#include "errorhandling.h"
#include "readability.h"

#include "groups-array.h"

//
// This method sets variables to an array of double values.
// It is assumed that members are in the order you got them through the bitstream.
//
void groups_array_SetDoubleArray
(gop* const g, const group* const a, const int propId, const int n, 
 const double* const values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// We need a counter to read the right value from the array.
	int k = 0;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetDouble(obj, propId, values[k++]);
	} macro_bitstream_end_foreach(a)
	
	gop_CreateBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	group* c = group_GcOr(NULL, b, a);
	gcstack_Swap(c, b);
	group_Delete(b);
	free(b);
}

void groups_array_SetStringArray
(gop* const g, const group* const a, const int propId, const int n, 
 const char** const values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* notDefaultIndices = malloc(n*sizeof(int));
	int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetString(obj, propId, values[k++]);
	} macro_bitstream_end_foreach(a)
	
	gop_CreateBitstreamArray(g);
	
	gcstack* const gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	
	group* notDef = group_InitWithIndices
	(group_GcAlloc(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	group* isDef = group_GcExcept(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	group* c = group_GcExcept(gc, group_GcOr(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}



void groups_array_SetIntArray
(gop* const g, const group* const a, const int propId, const int n, 
 const int* values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* const notDefaultIndices = malloc(n*sizeof(int));
	const int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetInt(obj, propId, values[k++]);
	} macro_bitstream_end_foreach(a)
	
	gop_CreateBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	
	group* notDef = group_InitWithIndices
	(group_GcAlloc(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	group* isDef = group_GcExcept(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	group* c = group_GcExcept(gc, group_GcOr(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}

void groups_array_SetBoolArray
(gop* const g, const group* const a, const int propId, const int n, 
 const int* const values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	gop_CreateMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// String has a default value in a bitstream,
	// so we must create a buffer that takes only those who are not default.
	// Later we convert it to a bitstream and use it for updating.
	// The maximum size equals the array of values.
	int* const notDefaultIndices = malloc(n*sizeof(int));
	const int notDefaultIndicesSize = 0;
	
	// We need an index to read properly from the values.
	int k = 0;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetBool(obj, propId, values[k++]);
	} macro_bitstream_end_foreach(a)
	
	gop_CreateBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
	group* b = g->m_bitstreamsArray[propIndex];
	
	group* notDef = group_InitWithIndices
	(group_GcAlloc(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	group* isDef = group_GcExcept(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	group* c = group_GcExcept(gc, group_GcOr(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}

void groups_array_FillDoubleArray
(gop* const g, const group* const a, const int propId, 
 const int arrc, double* const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_DOUBLE));
	
	// Make sure we have a table with pointers to members.
	gop_CreateMemberArray(g);
	
	const int size = group_Size(a);
	macro_err(size > arrc);
	
	int i;
	const hash_table* obj;
	int k = 0;
	const double* ptr;
	
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		ptr = (const double*)hashTable_Get(obj, propId);
		if (ptr == NULL)
			arr[k++] = 0.0;
		else
			arr[k++] = *ptr;
	} macro_bitstream_end_foreach(a)
}

void groups_array_FillIntArray
(gop* const g, const group* const a, const int propId,
 const int arrc, int* const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_INT));
	
	// Make sure we have a table with pointers to members.
	gop_CreateMemberArray(g);
	
	const int size = group_Size(a);
	macro_err(size > arrc);
	
	int i;
	const hash_table* obj;
	int k = 0;
	const int* ptr;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		ptr = (const int*)hashTable_Get(obj, propId);
		if (ptr == NULL)
			arr[k++] = -1;
		else
			arr[k++] = *ptr;
	} macro_bitstream_end_foreach(a)
}

void groups_array_FillBoolArray
(gop* const g, const group* const a, const int propId,
 const int arrc, int* const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_BOOL));
	
	// Make sure we have a table with pointers to members.
	gop_CreateMemberArray(g);
	
	const int size = group_Size(a);
	macro_err(size > arrc);
	
	int i;
	const hash_table* obj;
	int k = 0;
	const int* ptr;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		ptr = (const int*)hashTable_Get(obj, propId);
		if (ptr == NULL)
			arr[k++] = false;
		else
			arr[k++] = *ptr;
	} macro_bitstream_end_foreach(a)
}

void groups_array_FillStringArray
(gop* const g, const group* const a, const int propId,
 const int arrc, const char** const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!gop_IsPropertyType(propId, TYPE_STRING));
	
	// Make sure we have a table with pointers to members.
	gop_CreateMemberArray(g);
	
	const int size = group_Size(a);
	macro_err(size > arrc);
	
	int i;
	const hash_table* obj;
	
	int k = 0;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		arr[k++] = (const char*)hashTable_Get(obj, propId);
	} macro_bitstream_end_foreach(a)
}

