/*
 *  readability.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 22.05.12.
 *  Copyright 2012 Cutout Pro. All rights reserved.
 *
 */

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
	
#ifndef MEMGROUPS_READABILITY
#define MEMGROUPS_READABILITY
	
	// This is to make the code easier to read and to import from C#.
#define bool int
#define true 1
#define false 0
#define string char const*
#define byte unsigned char
	
#define TYPE_STRIDE 1000000
#define TYPE_DOUBLE 1
#define TYPE_STRING 2
#define TYPE_INT 3
#define TYPE_BOOL 4
#define TYPE_UNKNOWN 5
#define TYPE_VARIABLE 100
#define TYPE_ARRAY 200
	
	//
	//		FOREACH MACROES
	//
	//		These need to be copied to your source file in order to work.
	//		They simplify the task of dealing with nested bitstreams.
	//
	//		foreach (bitstream) {
	//			if (_pos(bitstream) > 200)
	//				_break(bitstream);
	//      } end_foreach
	//
	//		// Iterating reverse:
	//		foreach_reverse (bitstream) {
	//		} end_foreach
	//
	//		// Nested loops:
	//		foreach (a) {
	//			foreach (b) {
	//				_break(a); // Jumps outside the loop.
	//			} end_foreach
	//		} end_foreach
	//
	//		To access the index of the array which the bitstream
	//		is referring to, use
	//
	//		int i = _pos(prop);
	//
	//		Tip: Declaring variables before the loop will increase performance.
	//
	
#define macro_foreach_reverse(a) \
int _macro_start##a, _macro_end##a, _macro_i##a, _macro_j##a; \
for (_macro_i##a = a->length-2; _macro_i##a >= 0; _macro_i##a -= 2) { \
_macro_start##a = a->pointer[_macro_i##a]; \
_macro_end##a = a->pointer[_macro_i##a+1]; \
for (_macro_j##a = _macro_end##a-1; _macro_j##a >= _macro_start##a; _macro_j##a--) {
	
#define macro_foreach(a) \
int _macro_len##a = a->length-1; \
int _macro_start##a, _macro_end##a, _macro_i##a, _macro_j##a; \
for (_macro_i##a = 0; _macro_i##a < _macro_len##a; _macro_i##a += 2) { \
_macro_start##a = a->pointer[_macro_i##a]; \
_macro_end##a = a->pointer[_macro_i##a+1]; \
for (_macro_j##a = _macro_start##a; _macro_j##a < _macro_end##a; _macro_j##a++) {
	
#define macro_end_foreach(a) }}_macro_BREAK_BITSTREAM_##a:;
	
#define macro_break(a)     goto _macro_BREAK_BITSTREAM_##a
	
#define macro_pos(a)    _macro_j##a
	
	//
	//      FOR EACH DESIGNED FOR HASH TABLE
	//
	
#define macro_hashTable_foreach(a) gcstack_item* _macro_cursor##a = a->layers->root->next; \
hash_layer* _macro_layer##a; \
int* _macro_indices##a; \
int _macro_n##a, _macro_i##a; \
for (; _macro_cursor##a != NULL; _macro_cursor##a = _macro_cursor##a->next) { \
_macro_layer##a = (hash_layer*)_macro_cursor##a; \
_macro_indices##a = _macro_layer##a->indices; \
_macro_n##a = _macro_layer##a->n; \
for (_macro_i##a = 0; _macro_i##a < _macro_n##a; _macro_i##a++) { \
if (_macro_indices##a[_macro_i##a] == -1) continue;
	
#define macro_hashTable_id(a) _macro_indices##a[_macro_i##a]
#define macro_hashTable_value(a) _macro_layer##a->data[_macro_i##a]
#define macro_hashTable_double(a) *(double*)_macro_layer##a->data[_macro_i##a]
#define macro_hashTable_int(a) *(int*)_macro_layer##a->data[_macro_i##a]
#define macro_hashTable_bool(a) *(bool*)_macro_layer##a->data[_macro_i##a]
#define macro_hashTable_string(a) (char*)_macro_layer##a->data[_macro_i##a]
	
	//
	//      SIMPLIFIED ERROR HANDLING
	//
	//      Crashes thread or application and prints a detailed error message.
	//      It also prints the condition under which the thread or application should crash.
	//
#define macro_err(cond) if (cond) errorhandling_CrashWithLineAndFunctionAndMessage(__LINE__, __FUNCTION__, #cond);
	
	//
	//      This macro is specially suited for expression errors.
	//      It displays where in an expression an error happened.
	//      At the moment, it supports only single-line expression.
	//
#define macro_errExp(message,pos,expr) errorhandling_CrashExpression(message, pos, expr)
	
	//
	//      SIMPLIFIED STRING HANDLING
	//
	// PERFORMANCE COMPLEXITY LEVEL (CL)
	// 2^X per 10 sec
	// Jun 21 2012 17:37:16
	// 268435456
	// Leak 0
	// Duration: 14.928422
	// cl 27.421938, offset -0.578062
	// ________________________________
	// macro_string_concat(myName, "Alpha ", "Centuri");
	//
	//      This string concat macro is very fast.
	//      It allocated on the stack so 'assignTo' does not need to be released.
	//
#define macro_string_concat(assignTo,string1,string2) \
const char* _macro_string1##assignTo = string1; \
const char* _macro_string2##assignTo = string2; \
int _macro_string1Length##assignTo = strlen(_macro_string1##assignTo); \
int _macro_string2Length##assignTo = strlen(_macro_string2##assignTo); \
char assignTo[_macro_string1Length##assignTo+_macro_string2Length##assignTo+1]; \
memcpy(assignTo, _macro_string1##assignTo, _macro_string1Length##assignTo); \
memcpy(assignTo+_macro_string1Length##assignTo, _macro_string2##assignTo, _macro_string2Length##assignTo); \
assignTo[_macro_string1Length##assignTo+_macro_string2Length##assignTo] = '\0'
	
#endif
	
#ifdef __cplusplus
}
#endif
