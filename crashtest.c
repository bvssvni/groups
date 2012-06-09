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
#include "property.h"

// Some arbitrary number to check if it didn't crash properly.
int DID_NOT_CRASH = 42;

void crash_Test(void*(*f)(void* input), const char* message)
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
        printf("%s DID_NOT_CRASH!!!\r\n", message);
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
    groups_GetBitstream(NULL, 0);
    
    return &DID_NOT_CRASH;
}

void* crash_groups_GetBitstream2(void* input)
{
    gcstack* gc = (gcstack*)input;
    groups* g = groups_Init(groups_AllocWithGC(gc));
    groups_GetBitstream(g, -1);
    
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
    bitstream_InitWithDeltaDouble(bitstream_AllocWithGC(gc), -1, NULL, NULL);
    
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
    bitstream_InitWithDeltaString(bitstream_AllocWithGC(gc), -1, NULL, NULL);
    
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

void* crash_hashLayer_InitWithSize(void* input)
{
    hashLayer_InitWithSize(NULL, -1);
    
    return &DID_NOT_CRASH;
}

void* crash_hashLayer_InitWithSize2(void* input)
{
    gcstack* gc = (gcstack*)input;
    hashLayer_InitWithSize(hashLayer_AllocWithGC(gc), -1);
    
    return &DID_NOT_CRASH;
}

void crashtest_Run() {
    crash_Test(crash_bitstream_Abs, "bitstream_Abs");
    crash_Test(crash_bitstream_ArrayPointer, "bitstream_ArrayPointer");
    crash_Test(crash_bitstream_And, "bitstream_And");
    crash_Test(crash_bitstream_Clone, "bitstream_Clone");
    crash_Test(crash_bitstream_Delete, "bitstream_Delete");
    crash_Test(crash_bitstream_Except, "bitstream_Except");
    crash_Test(crash_bitstream_DirectJoin, "bitstream_DirectJoin");
    crash_Test(crash_bitstream_DirectJoin2, "bitstream_DirectJoin2");
    crash_Test(crash_bitstream_InitWithDeltaBool, "bitstream_InitWithDeltaBool");
    crash_Test(crash_bitstream_InitWithDeltaBool2, "bitstream_InitWithDeltaBool2");
    crash_Test(crash_bitstream_InitWithDeltaBool3, "bitstream_InitWithDeltaBool3");
    crash_Test(crash_bitstream_InitWithDeltaDouble, "bitstream_InitWithDeltaDouble");
    crash_Test(crash_bitstream_InitWithDeltaDouble2, "bitstream_InitWithDeltaDouble2");
    crash_Test(crash_bitstream_InitWithDeltaDouble3, "bitstream_InitWithDeltaDouble3");
    crash_Test(crash_bitstream_InitWithDeltaInt, "bitstream_InitWithDeltaInt");
    crash_Test(crash_bitstream_InitWithDeltaInt2, "bitstream_InitWithDeltaInt2");
    crash_Test(crash_bitstream_InitWithDeltaInt3, "bitstream_InitWithDeltaInt3");
    crash_Test(crash_bitstream_InitWithDeltaString, "bitstream_InitWithDeltaString");
    crash_Test(crash_bitstream_InitWithDeltaString2, "bitstream_InitWithDeltaString2");
    crash_Test(crash_bitstream_InitWithDeltaString3, "bitstream_InitWithDeltaString");
    crash_Test(crash_bitstream_InitWithIndices, "bitstream_InitWithIndices");
    crash_Test(crash_bitstream_InitWithIndices2, "bitstream_InitWithIndices2");
    crash_Test(crash_bitstream_InitWithIndices3, "bitstream_InitWithIndices3");
    crash_Test(crash_bitstream_InitWithSize, "bitstream_InitWithSize");
    crash_Test(crash_bitstream_InitWithSize2, "bitstream_InitWithSize2");
    crash_Test(crash_bitstream_InitWithValues, "bitstream_InitWithValues");
    crash_Test(crash_bitstream_InitWithValues2, "bitstream_InitWithValues2");
    crash_Test(crash_bitstream_InitWithValues3, "bitstream_InitWithValues3");
    crash_Test(crash_bitstream_Invert, "bitstream_Invert");
    crash_Test(crash_bitstream_NumberOfBlocks, "bitstream_NumberOfBlocks");
    crash_Test(crash_bitstream_Or, "bitstream_Or");
    crash_Test(crash_bitstream_PopEnd, "bitstream_PopEnd");
    crash_Test(crash_bitstream_PopStart, "bitstream_PopStart");
    crash_Test(crash_bitstream_Print, "crash_bitstream_Print");
    crash_Test(crash_bitstream_Size, "bitstream_Size");
    crash_Test(crash_groups_AddMember, "groups_AddMember");
    crash_Test(crash_groups_AddMember2, "groups_AddMember2");
    crash_Test(crash_groups_AddProperty, "groups_AddProperty");
    crash_Test(crash_groups_AddProperty2, "groups_AddProperty2");
    crash_Test(crash_groups_AddProperty3, "groups_AddProperty3");
    crash_Test(crash_groups_Delete, "groups_Delete");
    crash_Test(crash_groups_GetBitstream, "groups_GetBitstream");
    crash_Test(crash_groups_GetBitstream2, "groups_GetBitstream2");
    crash_Test(crash_groups_Init, "groups_Init");
    crash_Test(crash_groups_IsDefaultVariable, "groups_IsDefaultVariable");
    crash_Test(crash_groups_IsDefaultVariable2, "groups_IsDefaultVariable2");
    crash_Test(crash_gcstack_Delete, "gcstack_Delete");
    crash_Test(crash_groups_RemoveProperty, "groups_RemoveProperty");
    crash_Test(crash_groups_RemoveProperty2, "groups_RemovePoperty2");
    crash_Test(crash_groups_SetDouble, "groups_SetDouble");
    crash_Test(crash_groups_SetDouble2, "groups_SetDouble2");
    crash_Test(crash_groups_SetDouble3, "groups_SetDouble3");
    crash_Test(crash_hashTable_ContainsStringHash, "hashTable_ContainsStringHash");
    crash_Test(crash_hashTable_ContainsStringHash2, "hashTable_ContainsStringHash2");
    crash_Test(crash_hashTable_Delete, "hashTable_Delete");
    crash_Test(crash_hashTable_Get, "hashTable_Get");
    crash_Test(crash_hashTable_Get2, "hashTable_Get2");
    crash_Test(crash_groups_GetProperty, "groups_GetProperty");
    crash_Test(crash_groups_GetProperty2, "groups_GetProperty2");
    crash_Test(crash_hashTable_Init, "hashTable_Init");
    crash_Test(crash_hashTable_InitWithMember, "hashTable_InitWithMember");
    crash_Test(crash_hashTable_InitWithMember2, "hashTable_InitWithMember2");
    crash_Test(crash_hashTable_Set, "hashTable_Set");
    crash_Test(crash_hashTable_Set2, "hashTable_Set2");
    crash_Test(crash_hashTable_SetBool, "hashTable_SetBool");
    crash_Test(crash_hashTable_SetBool2, "hashTable_SetBool2");
    crash_Test(crash_hashTable_SetDouble, "hashTable_SetDouble");
    crash_Test(crash_hashTable_SetDouble2, "hashTable_SetDouble2");
    crash_Test(crash_hashTable_SetInt, "crash_hashTable_SetInt");
    crash_Test(crash_hashTable_SetInt2, "crash_hashTable_SetInt2");
    crash_Test(crash_hashTable_SetString, "hashTable_SetString");
    crash_Test(crash_hashTable_SetString2, "hashTable_SetString2");
    crash_Test(crash_hashTable_SetStringHash, "hashTable_SetStringHash");
    crash_Test(crash_hashTable_SetStringHash2, "hashTable_SetStringHash2");
    crash_Test(crash_property_Delete, "property_Delete");
    crash_Test(crash_hashLayer_InitWithSize, "hashLayer_InitWithSize");
    crash_Test(crash_hashLayer_InitWithSize2, "hashLayer_InitWithSize2");
 
}




