//
//  errorhandling.c
//  MemGroups
//
//  Created by Sven Nilsen on 11.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int m_crashApp = 0;

void errorhandling_OnCrashExitApp()
{
    m_crashApp = 1;
}

void errorhandling_OnCrashExitThread()
{
    m_crashApp = 0;
}

void errorhandling_Crash(const char* message) {
    fprintf(stderr, "%s\r\n", message);
    if (m_crashApp)
        exit(1);
    else
        pthread_exit(NULL);
}
