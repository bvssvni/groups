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

void crash_Test(void*(*f)(void* input))
{
    pthread_t thread;
    pthread_create(&thread, NULL, f, NULL);
    void* res;
    pthread_join(thread, &res);
    if (res == &DID_NOT_CRASH)
    {
        printf("DID_NOT_CRASH!!!\r\n");
        pthread_exit(NULL);
    }
}

void* crash_groups_Delete(void* input)
{
    groups_Delete(NULL);
    
    return &DID_NOT_CRASH;
}

void* crash_bitstream_Delete(void* input)
{
    bitstream_Delete(NULL);
    
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

void crashtest_Run() {
    crash_Test(crash_groups_Delete);
    crash_Test(crash_bitstream_Delete);
    crash_Test(crash_gcstack_Delete);
    crash_Test(crash_property_Delete);
    crash_Test(crash_hashTable_Delete);
}
