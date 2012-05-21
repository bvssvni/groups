/*
 *  string.c
 *  MemGroups
 *
 *  Created by Sven Nilsen on 15.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

#include "memgroups.h"

int string_references = 0;

int string_Length(string str)
{
	int i = 0;
	while (str[i] != '\0')
		i++;
	return i;
}

string string_Copy(string str)
{
	string_references++;
	
	int length = string_Length(str);
	string s = malloc(length*sizeof(char));
	for (int i = 0; i < length; i++)
		s[i] = str[i];
	return s;
}

bool string_Equals(string a, string b)
{
	return strcmp(a, b) == 0;
}

void string_Delete(string str)
{
	string_references--;
	// if (string_references < 0 || string_references > 1000)
	// 	printf("%i\r\n", string_references);
	
	free(str);
}