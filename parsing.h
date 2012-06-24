//
//  parsing.h
//  MemGroups
//
//  Created by Sven Nilsen on 15.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef MemGroups_parsing_h
#define MemGroups_parsing_h
	
	//
	//      The sscanf function does allow tracking of how many characters are read.
	//      This function computes the number of characters.
	//
	int parsing_ScanfSizeOf
	(const char* text, int type);
	
	//
	//      Reads a double from text and returns the number of characters read.
	//
	int parsing_ScanDouble
	(const char* text, double* output);
	
	//
	//      Reads an int from text and returns the number of characters read.
	//
	int parsing_ScanInt
	(const char* text, int* output);
	
	//
	//      Skipts empty space.
	//
	int parsing_SkipWhiteSpace
	(const char* text);
	
	int parsing_ReadCharacter
	(const char* text, char ch);
	
	//
	//      Remember to release the pointer when done.
	//
	char* parsing_ReadVariableName
	(const char* text, const char* breakAtCharacters, int* delta);
	
	char* parsing_ReadEscapedString
	(const char* text, int* delta);
	
#endif
	
#ifdef __cplusplus
}
#endif
