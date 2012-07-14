//
//  parsing.c
//  MemGroups
//
//  Created by Sven Nilsen on 15.06.12.
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

#include "memgroups.h"
#include "readability.h"

#include "parsing.h"

int parsing_ScanfSizeOf(const char* text, const int type) {
	macro_err(text == NULL);
	
	const bool acceptNumeric = true;
	const bool acceptDot = type == TYPE_DOUBLE || type == TYPE_STRING;
	const bool acceptDoubleQuote = false;
	const bool acceptWhiteSpace = type == TYPE_STRING;
	const bool acceptComma = false;
	const bool acceptParanthesis = type == TYPE_STRING;
	
	bool isNumeric, isDot, isDoubleQuote, isWhiteSpace, isComma, 
	isParanthesis;
	
	int i = 0;
	for (i = 0; text[i] != '\0'; i++) {
		
		isNumeric = text[i] >= '0' && text[i] <= '9';
		isDot = text[i] == '.';
		isDoubleQuote = text[i] == '"';
		
		isWhiteSpace = text[i] == ' ' || text[i] == '\r' || 
		text[i] == '\n' || text[i] == '\t';
		
		isComma = text[i] == ',';
		
		isParanthesis = text[i] == '}' || text[i] == '{' || 
		text[i] == ']' || text[i] == '[' ||
		text[i] == ')' || text[i] == '(';
		
		// Return 0 if there is an int data type ending with '.'
		// In such cases, a double should be read instead.
		if (isDot && type == TYPE_INT) 
			return 0;
		
		if (isNumeric && !acceptNumeric) 
			break;
		
		if (isDot && !acceptDot) 
			break;
		
		if (isDoubleQuote && !acceptDoubleQuote) 
			break;
		
		if (isWhiteSpace && !acceptWhiteSpace) 
			break;
		
		if (isComma && !acceptComma) 
			break;
		
		if (isParanthesis && !acceptParanthesis) 
			break;
	}
	
	return i;
}

int parsing_ScanDouble(const char* const text, double* const output)
{
	macro_err(text == NULL); macro_err(output == NULL);
	
	const int s = parsing_ScanfSizeOf(text, TYPE_DOUBLE);
	
	if (s == 0) 
		return 0;
	
	const int n = sscanf(text, "%lg", output);
	return n*s;
}

int parsing_ScanInt(const char* const text, int* const output)
{
	macro_err(text == NULL); macro_err(output == NULL);
	
	const int s = parsing_ScanfSizeOf(text, TYPE_INT);
	if (s == 0) 
		return 0;
	
	// The sscanf function does not return number of read characters,
	// but only 1 if one or more character is read.
	const int n = sscanf(text, "%i", output);
	return n *s ;
}

#define _isWhiteSpace(a) (a == ' ' || a == '\r' || a == '\n' || a == '\t')
#define _isLetter(a) ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z'))
#define _isNumeric(a) (a >= '0' && a <= '9')

int parsing_SkipWhiteSpace(const char* const text)
{
	macro_err(text == NULL);
	
	int i;
	char ch;
	for (i = 0; text[i] != '\0'; i++)
	{
		ch = text[i];
		if (_isWhiteSpace(ch)) 
			continue;
		
		break;
	}
	
	return i;
}

int parsing_ReadCharacter(const char* const text, const char ch) 
{
	macro_err(text == NULL);
	
	return text[0] == ch ? 1 : 0;
}

char parsing_ReadOneCharacterOf
(const char* const text, const char* const chs, int* const delta)
{
	macro_err(text == NULL); macro_err(chs == NULL);
	
	int i;
	for (i = 0; chs[i] != '\0'; i++) {
		if (text[0] == chs[i]) {
			*delta = 1;
			return chs[i];
		}
	}
	
	*delta = 0;
	return '\0';
}

char* parsing_ReadVariableName
(const char* const text, const char* const breakAtCharacters, int* const delta) 
{
	
	if (_isNumeric(text[0])) {
		*delta = 0;
		return NULL;
	}
	if (text[0] == '\"') {
		*delta = 0;
		return NULL;
	}
	
	// Loop until finding empty space or a character to break at.
	char ch;
	int i;
	for (i = 0; text[i] != '\0'; i++) {
		ch = text[i];
		if (_isWhiteSpace(ch)) break;
		if (strchr(breakAtCharacters, ch) != NULL) break;
	}
	
	char* output = malloc(sizeof(char)*(i+1));
	memcpy(output, text, i*sizeof(char));
	output[i] = '\0';
	
	*delta = i;
	return output;
}

char* parsing_ReadEscapedString
(const char* const text, int* const delta) {
	// If the first letter is not starting with '"', return NULL.
	if (text[0] != '\"') {
		*delta = 0; return NULL;
	}
	
	char ch;
	int i;
	
	// Count the number of characters in text.
	for (i = 1; text[i] != '\0'; i++) {
		ch = text[i];
		if (ch == '\"') 
			break;
	}
	
	// Copy characters from text.
	char* const output = malloc(sizeof(char)*i);
	memcpy(output, text+1, (i-1)*sizeof(char));
	
	// Add end character at end of string.
	output[i-1] = '\0';
	
	*delta = i+1;
	return output;
}

void parsing_RunUnitTests(void)
{
	printf("Parsing unit tests - "); 
	
	{
		int delta = 0;
		char* res = parsing_ReadEscapedString("\"foo\"", &delta);
		macro_test_string(res, "foo");
		free(res);
	}
	
	{
		int delta = 0;
		char* res = parsing_ReadVariableName("foo bar", "", &delta);
		macro_test_string(res, "foo");
		free(res);
	}
	
	{
		int delta = parsing_ReadCharacter("foo bar", 'f');
		macro_test_int(delta, 1);
	}
	
	{
		int delta = parsing_SkipWhiteSpace("   foo");
		macro_test_int(delta, 3);
	}
	
	{
		int num = 0;
		int delta = parsing_ScanInt("1248", &num);
		macro_test_int(delta, 4);
	}
	
	{
		int num = 0;
		int delta = parsing_ScanInt("20.8", &num);
		macro_test_int(delta, 0);
	}
	
	{
		double num = 0;
		int delta = parsing_ScanDouble("30.7", &num);
		macro_test_int(delta, 4);
	}
	
	{
		double num = 0;
		int delta = parsing_ScanDouble("30", &num);
		macro_test_int(delta, 2);
	}
	
	{
		double num = 0;
		int delta = parsing_ScanDouble("3e4", &num);
		macro_test_int(delta, 3);
	}
	
	{
		double num = 0;
		int delta = parsing_ScanDouble("-10e20", &num);
		macro_test_int(delta, 6);
	}
	
	printf("OK\r\n");
}

