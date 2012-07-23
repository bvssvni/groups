//
//  parsing.h
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

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef MemGroups_parsing_h
#define MemGroups_parsing_h
	
	//
	//	The sscanf function does allow tracking of how many characters 
	//	are read. This function computes the number of characters.
	//
	int parsing_ScanfSizeOf
	(const char* text, int type);
	
	//
	//      Reads a double from text and returns the number of characters 
	//	read.
	//
	int parsing_ScanDouble
	(const char* text, double* output);
	
	//
	//      Reads an int from text and returns the number of characters 
	//	read.
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
	//	Read one character of many options.
	//
	char parsing_ReadOneCharacterOf
	(const char* const text, const char* const chs, int* const delta);
	
	//
	//      Remember to release the pointer when done.
	//
	char* parsing_ReadVariableName
	(const char* text, const char* breakAtCharacters, int* delta);
	
	char* parsing_ReadEscapedString
	(const char* text, int* delta);
	
	//
	//	Reads an expected keyword from text.
	//	Returns 0 if the text does not start with the keyword.
	//
	int parsing_ReadKeyword
	(const char* const text, const char* const keyword);
	
#endif
	
#ifdef __cplusplus
}
#endif
