//
//  errorhandling.c
//  MemGroups
//
//  Created by Sven Nilsen on 11.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "errorhandling.h"

int m_crashApp = 0;

void errorhandling_OnCrashExitApp(void)
{
	m_crashApp = 1;
}

void errorhandling_OnCrashExitThread(void)
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
	int i;
	for (i = 0; i < pos; i++)
		fprintf(stderr, " ");
	fprintf(stderr, "^");
}

void errorhandling_CrashWithFileAndLineAndFunctionAndMessage
(const char* file, int line, const char* functionName, const char* message) 
{
	fprintf(stderr, "%s %i, %s: %s\r\n", file, line, functionName, message);
	if (m_crashApp)
		exit(1);
	else
		pthread_exit(NULL);
}

int errorhandling_CheckMargin
(const double a, const double b, const double errorMargin)
{
	const double maxB = b * (1+errorMargin);
	const double minB = b * (1-errorMargin);
	if (a < minB || b > maxB)
		return 0;
	return 1;
}
