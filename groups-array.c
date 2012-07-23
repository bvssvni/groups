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
#include "bitstream.h"
#include "groups.h"
#include "errorhandling.h"
#include "readability.h"

#include "groups-array.h"

//
// This method sets variables to an array of double values.
// It is assumed that members are in the order you got them through the bitstream.
//
void groups_array_SetDoubleArray
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const double* const values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	groups_CreateMemberArray(g);
	
	int i;
	hash_table* obj;
	
	// We need a counter to read the right value from the array.
	int k = 0;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		hashTable_SetDouble(obj, propId, values[k++]);
	} macro_bitstream_end_foreach(a)
	
	groups_CreateBitstreamArray(g);
	
	// Update the bitstream, cleaning up manually for saving one malloc call.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	bitstream* c = bitstream_GcOr(NULL, b, a);
	gcstack_Swap(c, b);
	bitstream_Delete(b);
	free(b);
}

void groups_array_SetStringArray
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const char** const values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	groups_CreateMemberArray(g);
	
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
	
	groups_CreateBitstreamArray(g);
	
	gcstack* const gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	bitstream* notDef = bitstream_InitWithIndices
	(bitstream_GcAlloc(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	bitstream* isDef = bitstream_GcExcept(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	bitstream* c = bitstream_GcExcept(gc, bitstream_GcOr(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}



void groups_array_SetIntArray
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const int* values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	groups_CreateMemberArray(g);
	
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
	
	groups_CreateBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	bitstream* notDef = bitstream_InitWithIndices
	(bitstream_GcAlloc(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	bitstream* isDef = bitstream_GcExcept(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	bitstream* c = bitstream_GcExcept(gc, bitstream_GcOr(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}

void groups_array_SetBoolArray
(groups* const g, const bitstream* const a, const int propId, const int n, 
 const bool* const values)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0); 
	macro_err_return(n < 0);
	macro_err_return(values == NULL);
	
	// Create member array so we can access members directly.
	groups_CreateMemberArray(g);
	
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
	
	groups_CreateBitstreamArray(g);
	
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	
	// Update the bitstream, this time is is a bit messier
	// so we use the gcstack for safety.
	// It takes only one operation to update all.
	const int propIndex = propId%TYPE_STRIDE;
	bitstream* b = g->m_bitstreamsArray[propIndex];
	
	bitstream* notDef = bitstream_InitWithIndices
	(bitstream_GcAlloc(gc), notDefaultIndicesSize, notDefaultIndices);
	
	// These are those who are default.
	bitstream* isDef = bitstream_GcExcept(gc, a, notDef);
	
	// existing + notDef - (input - notDef)
	bitstream* c = bitstream_GcExcept(gc, bitstream_GcOr(gc, b, notDef), isDef);
	
	gcstack_Swap(c, b);
	
	gcstack_Delete(gc);
	free(gc);
	
	// Free the buffer that stored the indices that was not default.
	free(notDefaultIndices);
}

void groups_array_FillDoubleArray
(groups* const g, const bitstream* const a, const int propId, 
 const int arrc, double* const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!groups_IsPropertyType(propId, TYPE_DOUBLE));
	
	// Make sure we have a table with pointers to members.
	groups_CreateMemberArray(g);
	
	const int size = bitstream_Size(a);
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
(groups* const g, const bitstream* const a, const int propId,
 const int arrc, int* const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!groups_IsPropertyType(propId, TYPE_INT));
	
	// Make sure we have a table with pointers to members.
	groups_CreateMemberArray(g);
	
	const int size = bitstream_Size(a);
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
(groups* const g, const bitstream* const a, const int propId,
 const int arrc, bool* const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!groups_IsPropertyType(propId, TYPE_BOOL));
	
	// Make sure we have a table with pointers to members.
	groups_CreateMemberArray(g);
	
	const int size = bitstream_Size(a);
	macro_err(size > arrc);
	
	int i;
	const hash_table* obj;
	int k = 0;
	const bool* ptr;
	macro_bitstream_foreach (a) {
		i = macro_bitstream_pos(a);
		obj = g->m_memberArray[i];
		ptr = (const bool*)hashTable_Get(obj, propId);
		if (ptr == NULL)
			arr[k++] = false;
		else
			arr[k++] = *ptr;
	} macro_bitstream_end_foreach(a)
}

void groups_array_FillStringArray
(groups* const g, const bitstream* const a, const int propId,
 const int arrc, const char** const arr)
{
	macro_err_return(g == NULL);
	macro_err_return(a == NULL);
	macro_err_return(propId < 0);
	macro_err_return(arr == NULL);
	macro_err_return(arrc < 0);
	macro_err_return(!groups_IsPropertyType(propId, TYPE_STRING));
	
	// Make sure we have a table with pointers to members.
	groups_CreateMemberArray(g);
	
	const int size = bitstream_Size(a);
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

