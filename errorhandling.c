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

void errorhandling_CrashWithMessage(const char* message) {
	fprintf(stderr, "%s\r\n", message);
	if (m_crashApp)
		exit(1);
	else
		pthread_exit(NULL);
}

void errorhandling_CrashWithFunctionAndMessage(const char* functionName, const char* message) {
	fprintf(stderr, "%s: %s\r\n", functionName, message);
	if (m_crashApp)
		exit(1);
	else
		pthread_exit(NULL);
}

void errorhandling_CrashWithLineAndFunctionAndMessage(int line, const char* functionName, const char* message) {
	fprintf(stderr, "%i, %s: %s\r\n", line, functionName, message);
	if (m_crashApp)
		exit(1);
	else
		pthread_exit(NULL);
}

void errorhandling_CrashExpression
(const char* message, int pos, const char* expr) {
	fprintf(stderr, "%i: ERROR, %s\r\n", pos, message);
	fprintf(stderr, "%s\r\n", expr);
	for (int i = 0; i < pos; i++)
		fprintf(stderr, " ");
	fprintf(stderr, "^");
}

void errorhandling_CrashWithFileAndLineAndFunctionAndMessage(const char* file, int line, const char* functionName, const char* message) {
	fprintf(stderr, "%s %i, %s: %s\r\n", file, line, functionName, message);
	if (m_crashApp)
		exit(1);
	else
		pthread_exit(NULL);
}

