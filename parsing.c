//
//  parsing.c
//  MemGroups
//
//  Created by Sven Nilsen on 15.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#include <stdio.h>

#include "memgroups.h"
#include "readability.h"

#include "parsing.h"

int parsing_ScanfSizeOf(const char* text, int type) {
    _err(text == NULL);
    
    bool acceptNumeric = true;
    bool acceptDot = type == TYPE_DOUBLE || type == TYPE_STRING;
    bool acceptDoubleQuote = false;
    bool acceptWhiteSpace = type == TYPE_STRING;
    bool acceptComma = false;
    bool acceptParanthesis = type == TYPE_STRING;
    
    bool isNumeric, isDot, isDoubleQuote, isWhiteSpace, isComma, isParanthesis;
    
    int i = 0;
    for (i = 0; text[i] != '\0'; i++) {
        
        isNumeric = text[i] >= '0' && text[i] <= '9';
        isDot = text[i] == '.';
        isDoubleQuote = text[i] == '"';
        isWhiteSpace = text[i] == ' ' || text[i] == '\r' || text[i] == '\n' || text[i] == '\t';
        isComma = text[i] == ',';
        isParanthesis = text[i] == '}' || text[i] == '{' || text[i] == ']' || text[i] == '[' ||
        text[i] == ')' || text[i] == '(';
        
        if (isNumeric && !acceptNumeric) break;
        if (isDot && !acceptDot) break;
        if (isDoubleQuote && !acceptDoubleQuote) break;
        if (isWhiteSpace && !acceptWhiteSpace) break;
        if (isComma && !acceptComma) break;
        if (isParanthesis && !acceptParanthesis) break;
    }
    
    return i;
}

int parsing_ScanDouble(const char* text, double* output)
{
    _err(text == NULL); _err(output == NULL);
    
    int s = parsing_ScanfSizeOf(text, TYPE_DOUBLE);
    if (s == 0) return 0;
    int n = sscanf(text, "%lg", output);
    return n*s;
}

int parsing_ScanInt(const char* text, int* output)
{
    _err(text == NULL); _err(output == NULL);
    
    int s = parsing_ScanfSizeOf(text, TYPE_INT);
    if (s == 0) return 0;
    int n = sscanf(text, "%i", output);
    return n*s;
}

#define _isWhiteSpace(a) (a == ' ' || a == '\r' || a == '\n' || a == '\t')
#define _isLetter(a) ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z'))
#define _isNumeric(a) (a >= '0' && a <= '9')

int parsing_SkipWhiteSpace(const char* text)
{
    _err(text == NULL);
    
    int i;
    char ch;
    for (i = 0; text[i] != '\0'; i++)
    {
        ch = text[i];
        if (_isWhiteSpace(ch)) continue;
        break;
    }
    return i;
}

int parsing_ReadCharacter(const char* text, char ch) {
    _err(text == NULL);
    return text[0] == ch ? 1 : 0;
}

char* parsing_ReadVariableName
(const char* text, const char* breakAtCharacters, int* delta) {
    
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
    for (int j = 0; j < i; j++) {
        output[j] = text[j];
    }
    output[i] = '\0';
    
    *delta = i;
    return output;
}

char* parsing_ReadEscapedString
(const char* text, int* delta) {
    if (text[0] != '\"') {
        *delta = 0; return NULL;
    }
    
    char ch;
    int i;
    for (i = 1; text[i] != '\0'; i++) {
        ch = text[i];
        if (ch == '\"') break;
    }
    
    char* output = malloc(sizeof(char)*(i+1));
    for (int j = 0; j < i; j++) {
        output[j] = text[j];
    }
    output[i] = '\0';
    
    *delta = i+1;
    return output;
}

