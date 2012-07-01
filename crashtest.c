//
//  crashtest.c
//  MemGroups
//
//  Created by Sven Nilsen on 08.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>

#include "memgroups.h"

// Some arbitrary number to check if it didn't crash properly.
int DID_NOT_CRASH = 42;

#define macro_crash(a) \
crash_Test(a, #a)

void crash_Test(void*(*f)(void* input), const char const* message)
{
	pthread_t thread;
	
	// Use garbage collector stack to clean up after crash.
	gcstack* gc = gcstack_Init(gcstack_Alloc());
	pthread_create(&thread, NULL, f, gc);
	
	void* res;
	pthread_join(thread, &res);
	
	// Clean up memory allocated by crash method.
	gcstack_Delete(gc);
	free(gc);
	
	if (res == &DID_NOT_CRASH)
	{
		printf("\r\nDID NOT CRASH: %s\r\n", message);
        	pthread_exit(NULL);
	}
}

void* crash_groups_Delete(void* input)
{
	groups_Delete(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_Init(void* input)
{
	groups_Init(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_AddProperty(void* input)
{
	groups_AddProperty(NULL, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_AddProperty2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_AddProperty(g, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_AddProperty3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_AddProperty(g, "name", NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetProperty(void* input)
{
	groups_GetProperty(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetProperty2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_GetProperty(g, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetBitstream(void* input)
{
	groups_GetBitstream(NULL, NULL, 0);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetBitstream2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_GetBitstream(NULL, g, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_RemoveProperty(void* input)
{
	groups_RemoveProperty(NULL, 0);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_RemoveProperty2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_RemoveProperty(g, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_IsDefaultVariable(void* input)
{
	groups_IsDefaultVariable(-1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_IsDefaultVariable2(void* input)
{
	groups_IsDefaultVariable(0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_AddMember(void* input)
{
	groups_AddMember(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_AddMember2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_AddMember(g, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDouble(void* input)
{
	groups_SetDouble(NULL, NULL, -1, 0);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDouble2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetDouble(g, NULL, -1, 0);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDouble3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetDouble(g, b, -1, 0);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetString(void* input)
{
	groups_SetString(NULL, NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetString2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetString(g, NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetString3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetString(g, b, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetInt(void* input)
{
	groups_SetInt(NULL, NULL, 0, 20);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetInt2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetInt(g, NULL, 0, 20);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetInt3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetInt(g, b, -1, 20);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBool(void* input)
{
	groups_SetBool(NULL, NULL, -1, 1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBool2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetBool(g, NULL, 0, 1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBool3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetBool(g, b, -1, 1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDoubleArray(void* input)
{
	groups_SetDoubleArray(NULL, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDoubleArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetDoubleArray(g, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDoubleArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetDoubleArray(g, b, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDoubleArray4(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetDoubleArray(g, b, 0, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetDoubleArray5(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetDoubleArray(g, b, 0, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetStringArray(void* input)
{
	groups_SetStringArray(NULL, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetStringArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetStringArray(g, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetStringArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetStringArray(g, b, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetStringArray4(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetStringArray(g, b, 0, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetStringArray5(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetStringArray(g, b, 0, 20, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetIntArray(void* input)
{
	groups_SetIntArray(NULL, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetIntArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetIntArray(g, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetIntArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetIntArray(g, b, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetIntArray4(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetIntArray(g, b, 0, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetIntArray5(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetIntArray(g, b, 0, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBoolArray(void* input)
{
	groups_SetBoolArray(NULL, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBoolArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SetBoolArray(g, NULL, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBoolArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetBoolArray(g, b, -1, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBoolArray4(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetBoolArray(g, b, 0, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SetBoolArray5(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_SetBoolArray(g, b, 0, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetDoubleArray(void* input)
{
	groups_GetDoubleArray(NULL, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetDoubleArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_GetDoubleArray(g, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetDoubleArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_GetDoubleArray(g, b, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetIntArray(void* input)
{
	groups_GetIntArray(NULL, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetIntArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_GetIntArray(g, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetIntArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_GetIntArray(g, b, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetBoolArray(void* input)
{
	groups_GetBoolArray(NULL, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetBoolArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_GetBoolArray(g, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetBoolArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_GetBoolArray(g, b, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetStringArray(void* input)
{
	groups_GetStringArray(NULL, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetStringArray2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_GetStringArray(g, NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_GetStringArray3(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	bitstream* b = bitstream_InitWithSize(bitstream_AllocWithGC(gc), 0);
	groups_GetStringArray(g, b, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_PropertyNameById(void* input)
{
	groups_PropertyNameById(NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_PropertyNameById2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_PropertyNameById(g, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_PrintMember(void* input)
{
	groups_PrintMember(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_PrintMember2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_PrintMember(g, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_RemoveMember(void* input)
{
	groups_RemoveMember(NULL, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_RemoveMember2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_RemoveMember(g, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_RemoveMembers(void* input)
{
	groups_RemoveMembers(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_RemoveMembers2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_RemoveMembers(g, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SaveToFile(void* input)
{
	groups_SaveToFile(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_SaveToFile2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_SaveToFile(g, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_ReadFromFile(void* input)
{
	groups_ReadFromFile(NULL, NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_groups_ReadFromFile2(void* input)
{
	gcstack* gc = (gcstack*)input;
	groups* g = groups_Init(groups_AllocWithGC(gc));
	groups_ReadFromFile(g, NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Delete(void* input)
{
	bitstream_Delete(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithSize(void* input)
{
	bitstream_InitWithSize(NULL, 2);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithSize2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithSize(bitstream_AllocWithGC(gc), -1);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithValues(void* input)
{
	bitstream_InitWithValues(NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithValues2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithValues(bitstream_AllocWithGC(gc), -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithValues3(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithValues(bitstream_AllocWithGC(gc), 1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithIndices(void* input)
{
	bitstream_InitWithIndices(NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithIndices2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithIndices(bitstream_AllocWithGC(gc), -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithIndices3(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithIndices(bitstream_AllocWithGC(gc), 1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaDouble(void* input)
{
	bitstream_InitWithDeltaDouble(NULL, -1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaDouble2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaDouble(bitstream_AllocWithGC(gc), -1, NULL, 
				      NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaDouble3(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaDouble(bitstream_AllocWithGC(gc), 1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaInt(void* input)
{
	bitstream_InitWithDeltaInt(NULL, -1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaInt2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaInt(bitstream_AllocWithGC(gc), -1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaInt3(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaInt(bitstream_AllocWithGC(gc), 1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaBool(void* input)
{
	bitstream_InitWithDeltaBool(NULL, -1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaBool2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaBool(bitstream_AllocWithGC(gc), -1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaBool3(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaBool(bitstream_AllocWithGC(gc), 1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaString(void* input)
{
	bitstream_InitWithDeltaString(NULL, -1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaString2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaString(bitstream_AllocWithGC(gc), -1, NULL, 
				      NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_InitWithDeltaString3(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_InitWithDeltaString(bitstream_AllocWithGC(gc), 1, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_DirectJoin(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream_DirectJoin(gc, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_DirectJoin2(void* input)
{
	gcstack* gc = (gcstack*)input;
	bitstream* a = bitstream_InitWithSize(a, 0);
	bitstream_DirectJoin(gc, a, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Print(void* input)
{
	bitstream_Print(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Clone(void* input)
{
	bitstream_Clone(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_And(void* input)
{
	bitstream_And(NULL, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Or(void* input)
{
	bitstream_Or(NULL, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Invert(void* input)
{
	bitstream_Invert(NULL, NULL, 0);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Except(void* input)
{
	bitstream_Except(NULL, NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Size(void* input)
{
	bitstream_Size(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_Abs(void* input)
{
	bitstream_Abs(NULL, 10);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_ArrayPointer(void* input)
{
	bitstream_ArrayPointer(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_NumberOfBlocks(void* input)
{
	bitstream_NumberOfBlocks(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_PopStart(void* input)
{
	bitstream_PopStart(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_bitstream_PopEnd(void* input)
{
	bitstream_PopEnd(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_gcstack_Delete(void* input)
{
	gcstack_Delete(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_gcstack_PopStringWithItem(void* input)
{
	gcstack_PopStringWithItem(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_gcstack_PopStringWithItem2(void* input)
{
	gcstack* gc = (gcstack*)input;
	
	// Create a stack and push it on the stack for cleaning up.
	gcstack* st = gcstack_Init(gcstack_Alloc());
	gcstack_PushPointer(gc, st, gcstack_Delete);
	
	gcstack_PopStringWithItem(st, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_property_Delete(void* input)
{
	property_Delete(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_Delete(void* input)
{
	hashTable_Delete(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_Init(void* input)
{
	hashTable_Init(NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_InitWithMember(void* input)
{
	hashTable_InitWithMember(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_InitWithMember2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hashTable_InitWithMember(hashTable_AllocWithGC(gc), NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_Set(void* input)
{
	hashTable_Set(NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_Set2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_Set(hs, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetStringHash(void* input)
{
	hashTable_SetStringHash(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetStringHash2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_SetStringHash(hs, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_Get(void* input)
{
	hashTable_Get(NULL, 0);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_Get2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_Get(hs, -1);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_ContainsStringHash(void* input)
{
	hashTable_ContainsStringHash(NULL, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_ContainsStringHash2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_ContainsStringHash(hs, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetDouble(void* input)
{
	hashTable_SetDouble(NULL, 0, 20);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetDouble2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_SetDouble(hs, -1, 20);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetString(void* input)
{
	hashTable_SetString(NULL, 0, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetString2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_SetString(hs, -1, NULL);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetInt(void* input)
{
	hashTable_SetInt(NULL, 0, 20);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetInt2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_SetInt(hs, -1, 20);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetBool(void* input)
{
	hashTable_SetBool(NULL, -1, 1);
	
	return &DID_NOT_CRASH;
}

void* crash_hashTable_SetBool2(void* input)
{
	gcstack* gc = (gcstack*)input;
	hash_table* hs = hashTable_Init(hashTable_AllocWithGC(gc));
	hashTable_SetBool(hs, -1, 1);
	
	return &DID_NOT_CRASH;
}

void crashtest_Run() {
	macro_crash(crash_bitstream_Abs);
	macro_crash(crash_bitstream_ArrayPointer);
	macro_crash(crash_bitstream_And);
	macro_crash(crash_bitstream_Clone);
	macro_crash(crash_bitstream_Delete);
	macro_crash(crash_bitstream_Except);
	macro_crash(crash_bitstream_DirectJoin);
	macro_crash(crash_bitstream_DirectJoin2);
	macro_crash(crash_bitstream_InitWithDeltaBool);
	macro_crash(crash_bitstream_InitWithDeltaBool2);
	macro_crash(crash_bitstream_InitWithDeltaBool3);
	macro_crash(crash_bitstream_InitWithDeltaDouble);
	macro_crash(crash_bitstream_InitWithDeltaDouble2);
	macro_crash(crash_bitstream_InitWithDeltaDouble3);
	macro_crash(crash_bitstream_InitWithDeltaInt);
	macro_crash(crash_bitstream_InitWithDeltaInt2);
	macro_crash(crash_bitstream_InitWithDeltaInt3);
	macro_crash(crash_bitstream_InitWithDeltaString);
	macro_crash(crash_bitstream_InitWithDeltaString2);
	macro_crash(crash_bitstream_InitWithDeltaString3);
	macro_crash(crash_bitstream_InitWithIndices);
	macro_crash(crash_bitstream_InitWithIndices2);
	macro_crash(crash_bitstream_InitWithIndices3);
	macro_crash(crash_bitstream_InitWithSize);
	macro_crash(crash_bitstream_InitWithSize2);
	macro_crash(crash_bitstream_InitWithValues);
	macro_crash(crash_bitstream_InitWithValues2);
	macro_crash(crash_bitstream_InitWithValues3);
	macro_crash(crash_bitstream_Invert);
	macro_crash(crash_bitstream_NumberOfBlocks);
	macro_crash(crash_bitstream_Or);
	macro_crash(crash_bitstream_PopEnd);
	macro_crash(crash_bitstream_PopStart);
	macro_crash(crash_bitstream_Print);
	macro_crash(crash_bitstream_Size);
	macro_crash(crash_groups_AddMember);
	macro_crash(crash_groups_AddMember2);
	macro_crash(crash_groups_AddProperty);
	macro_crash(crash_groups_AddProperty2);
	macro_crash(crash_groups_AddProperty3);
	macro_crash(crash_groups_Delete);
	macro_crash(crash_groups_GetBitstream);
	macro_crash(crash_groups_GetBitstream2);
	macro_crash(crash_groups_Init);
	macro_crash(crash_groups_IsDefaultVariable);
	macro_crash(crash_groups_IsDefaultVariable2);
	macro_crash(crash_gcstack_Delete);
	macro_crash(crash_groups_RemoveProperty);
	macro_crash(crash_groups_RemoveProperty2);
	macro_crash(crash_groups_SetBool);
	macro_crash(crash_groups_SetBool2);
	macro_crash(crash_groups_SetBool3);
	macro_crash(crash_groups_SetDouble);
	macro_crash(crash_groups_SetDouble2);
	macro_crash(crash_groups_SetDouble3);
	macro_crash(crash_groups_SetInt);
	macro_crash(crash_groups_SetInt2);
	macro_crash(crash_groups_SetInt3);
	macro_crash(crash_hashTable_ContainsStringHash);
	macro_crash(crash_hashTable_ContainsStringHash2);
	macro_crash(crash_hashTable_Delete);
	macro_crash(crash_hashTable_Get);
	macro_crash(crash_hashTable_Get2);
	macro_crash(crash_groups_GetProperty);
	macro_crash(crash_groups_GetProperty2);
	macro_crash(crash_hashTable_Init);
	macro_crash(crash_hashTable_InitWithMember);
	macro_crash(crash_hashTable_InitWithMember2);
	macro_crash(crash_hashTable_Set);
	macro_crash(crash_hashTable_Set2);
	macro_crash(crash_hashTable_SetBool);
	macro_crash(crash_hashTable_SetBool2);
	macro_crash(crash_hashTable_SetDouble);
	macro_crash(crash_hashTable_SetDouble2);
	macro_crash(crash_groups_GetBoolArray);
	macro_crash(crash_groups_GetBoolArray2);
	macro_crash(crash_groups_GetBoolArray3);
	macro_crash(crash_groups_GetDoubleArray);
	macro_crash(crash_groups_GetDoubleArray2);
	macro_crash(crash_groups_GetDoubleArray3);
	macro_crash(crash_groups_GetIntArray);
	macro_crash(crash_groups_GetIntArray2);
	macro_crash(crash_groups_GetIntArray3);
	macro_crash(crash_groups_GetStringArray);
	macro_crash(crash_groups_GetStringArray2);
	macro_crash(crash_groups_GetStringArray3);
	macro_crash(crash_groups_PrintMember);
	macro_crash(crash_groups_PrintMember2);
	macro_crash(crash_groups_PropertyNameById);
	macro_crash(crash_groups_PropertyNameById2);
	macro_crash(crash_groups_ReadFromFile);
	macro_crash(crash_groups_ReadFromFile2);
	macro_crash(crash_groups_RemoveMember);
	macro_crash(crash_groups_RemoveMember2);
	macro_crash(crash_groups_RemoveMembers);
	macro_crash(crash_groups_RemoveMembers2);
	macro_crash(crash_groups_SaveToFile);
	macro_crash(crash_groups_SaveToFile2);
	macro_crash(crash_groups_SetBoolArray);
	macro_crash(crash_groups_SetBoolArray2);
	macro_crash(crash_groups_SetBoolArray3);
	macro_crash(crash_groups_SetBoolArray4);
	macro_crash(crash_groups_SetBoolArray5);
	macro_crash(crash_groups_SetDoubleArray);
	macro_crash(crash_groups_SetDoubleArray2);
	macro_crash(crash_groups_SetDoubleArray3);
	macro_crash(crash_groups_SetDoubleArray4);
	macro_crash(crash_groups_SetDoubleArray5);
	macro_crash(crash_groups_SetIntArray);
	macro_crash(crash_groups_SetIntArray2);
	macro_crash(crash_groups_SetIntArray3);
	macro_crash(crash_groups_SetIntArray4);
	macro_crash(crash_groups_SetIntArray5);
	macro_crash(crash_groups_SetStringArray);
	macro_crash(crash_groups_SetStringArray2);
	macro_crash(crash_groups_SetStringArray3);
	macro_crash(crash_groups_SetStringArray4);
	macro_crash(crash_groups_SetStringArray5);
	macro_crash(crash_groups_SetString);
	macro_crash(crash_groups_SetString2);
	macro_crash(crash_groups_SetString3);
	macro_crash(crash_hashTable_SetInt);
	macro_crash(crash_hashTable_SetInt2);
	macro_crash(crash_hashTable_SetString);
	macro_crash(crash_hashTable_SetString2);
	macro_crash(crash_hashTable_SetStringHash);
	macro_crash(crash_hashTable_SetStringHash2);
	macro_crash(crash_property_Delete);
	
	////////////////////////////////////////////////////////////////////////
	macro_crash(crash_gcstack_PopStringWithItem);
	macro_crash(crash_gcstack_PopStringWithItem2);
}




