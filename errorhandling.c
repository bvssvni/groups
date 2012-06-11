//
//  errorhandling.c
//  MemGroups
//
//  Created by Sven Nilsen on 11.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>


void crash(const char* message) {
    fprintf(stderr, "%s\r\n", message);
    pthread_exit(NULL);
}
