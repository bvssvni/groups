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
	
/* This is to make the code easier to read and to import from C#. */
#define true 1
#define false 0
#define string const char*
#define byte unsigned char
	
	/*
		GROUP PROPERTY TYPES
	
		A property is composed of index and type.
		TYPE_STRIDE defines the maximum number of properties.
		Notice this does not put a limit upon number of members.
	*/
	enum
	{
		TYPE_STRIDE = 1000000,
		TYPE_DOUBLE = 1,
		TYPE_STRING = 2,
		TYPE_INT = 3,
		TYPE_BOOL = 4,
		TYPE_UNKNOWN = 5,
		TYPE_VARIABLE = 100,
		TYPE_ARRAY = 200
	};
	
	/*
		FOREACH MACROES
	
		These need to be copied to your source file in order to work.
		They simplify the task of dealing with nested bitstreams.
	
		macro_bitstream_foreach (bitstream) {
			if (macro_bitstream_pos(bitstream) > 200)
				macro_bitstream_break(bitstream);
		} macro_bitstream_end_foreach(bitstream)
	
		// Iterating reverse:
		macro_bitstream_foreach_reverse (bitstream) {
			...
		} macro_bitstream_end_foreach(bitstream)
	
		// Nested loops:
		macro_bitstream_foreach (a) {
			macro_bitstream_foreach (b) {
				 // Jumps outside the loop.
				macro_bitstream_break(a);
			} macro_bitstream_end_foreach(b)
		} macro_bitstream_end_foreach(a)
	
		To access the index of the array which the bitstream
		is referring to, use
	
		int i = macro_bitstream_pos(prop);
	
		Tip: Declaring variables before the loop will increase 
		performance.
	*/
	
#define macro_bitstream_foreach_reverse(a) 				\
	int _macro_start##a, _macro_end##a, _macro_i##a, _macro_j##a; 	\
	for (_macro_i##a = a->length-2;					\
	_macro_i##a >= 0; _macro_i##a -= 2) { 				\
		_macro_start##a = a->pointer[_macro_i##a]; 		\
		_macro_end##a = a->pointer[_macro_i##a+1]; 		\
		for (_macro_j##a = _macro_end##a-1; 			\
		_macro_j##a >= _macro_start##a; _macro_j##a--) {
	
#define macro_bitstream_foreach(a) 					\
	int _macro_len##a = a->length-1; 				\
	int _macro_start##a, _macro_end##a, _macro_i##a, _macro_j##a; 	\
	for (_macro_i##a = 0; 						\
	_macro_i##a < _macro_len##a; _macro_i##a += 2) { 		\
		_macro_start##a = a->pointer[_macro_i##a]; 		\
		_macro_end##a = a->pointer[_macro_i##a+1]; 		\
		for (_macro_j##a = _macro_start##a;			\
		_macro_j##a < _macro_end##a; _macro_j##a++) {

/* Add a dummy goto in order to remove compiler warning. */
#define macro_bitstream_end_foreach(a) 					\
	}} macro_bitstream_break(a); _macro_BREAK_BITSTREAM_##a:;
	
#define macro_bitstream_break(a)	goto _macro_BREAK_BITSTREAM_##a
	
#define macro_bitstream_pos(a)   	 _macro_j##a
	
	/*
		FOR EACH - DESIGNED FOR HASH TABLE
	*/
	
#define macro_hashTable_foreach(a) 					\
	const gcstack_item* _macro_cursor##a = a->layers->root->next; 	\
	const member_hash_layer* _macro_layer##a; 			\
	const int* _macro_indices##a; 					\
	int _macro_n##a, _macro_i##a; 					\
	for (; _macro_cursor##a != NULL; 				\
	_macro_cursor##a = _macro_cursor##a->next) { 			\
		_macro_layer##a = (member_hash_layer*)_macro_cursor##a; \
		_macro_indices##a = _macro_layer##a->indices; 		\
		_macro_n##a = _macro_layer##a->n; 			\
		for (_macro_i##a = 0; 					\
		_macro_i##a < _macro_n##a; _macro_i##a++) { 		\
			if (_macro_indices##a[_macro_i##a] == -1) 	\
				continue;
	
#define macro_hashTable_id(a) _macro_indices##a[_macro_i##a]
#define macro_hashTable_value(a) _macro_layer##a->data[_macro_i##a]
#define macro_hashTable_double(a) *(double*)_macro_layer##a->data[_macro_i##a]
#define macro_hashTable_int(a) *(int*)_macro_layer##a->data[_macro_i##a]
#define macro_hashTable_bool(a) *(int*)_macro_layer##a->data[_macro_i##a]
#define macro_hashTable_string(a) (char*)_macro_layer##a->data[_macro_i##a]
	
	/*
		SIMPLIFIED ERROR HANDLING
	
		Crashes thread or application and prints a detailed error 
		message. It also prints the condition under which the thread or 
		application should crash.
	*/
#define macro_err(cond) 						\
	if (cond) 							\
		errorhandling_CrashWithLineAndFunctionAndMessage 	\
		(__LINE__, __FUNCTION__, #cond);
	
#define macro_err_return(cond) 						\
	if (cond) {							\
		errorhandling_CrashWithLineAndFunctionAndMessage 	\
		(__LINE__, __FUNCTION__, #cond);			\
		return;							\
	}
	
#define macro_err_return_null(cond) 					\
	if (cond) {							\
		errorhandling_CrashWithLineAndFunctionAndMessage 	\
		(__LINE__, __FUNCTION__, #cond);			\
		return NULL;						\
	}
	
#define macro_err_return_zero(cond) 					\
	if (cond) {							\
		errorhandling_CrashWithLineAndFunctionAndMessage 	\
		(__LINE__, __FUNCTION__, #cond);			\
		return 0;						\
	}
	
	/*
		UNIT TESTING
	
	
		This macro prints out a proper message if the arguments
		are not the same and kills the thread/application if that is the
		case.
	*/
#define macro_test_string(a, b) 					\
	if (strcmp(a, b) != 0) { 					\
		printf("|%s| expected |%s|\r\n", a, b); 		\
		macro_err(strcmp(a, b) != 0); 				\
	}
	
	/*
		This macro prints out a proper message if the int arguments
		are not equal and kills the thread/application if that is the
		case.
	*/
#define macro_test_int(a, b) 						\
	if (a != b) { 							\
		printf("|%i| expected |%i|\r\n", a, b); 		\
		macro_err(a != b); 					\
	}
	
#define macro_test_double(a, b) 					\
	if (a != b) { 							\
		printf("|%g| expected |%g|\r\n", a, b); 		\
		macro_err(a != b); 					\
	}
	
#define macro_test_double_margin(a, b, c) 				\
	if (!errorhandling_CheckMargin(a, b, c)) { 			\
		printf("|%g| expected |%g|\r\n", a, b); 		\
		macro_err(!errorhandling_CheckMargin(a, b, c)); 	\
	}
	
#define macro_test_null(a) 						\
	if (a != NULL) { 						\
		printf("Expected NULL\r\n"); 				\
		macro_err(a != NULL); 					\
	}
	
	/*
	      This macro is specially suited for expression errors.
	      It displays where in an expression an error happened.
	      At the moment, it supports only single-line expression.
	*/
#define macro_errExp(message,pos,expr) 					\
	errorhandling_CrashExpression(message, pos, expr)
	
	/*
		FAST STRING CONCATENATION
	
	 	macro_string_concat(myName, "Alpha ", "Centuri");
	
		This string concat declares <myName> on the stack.
		Using dynamic arrays, no malloc call is necessary.
	*/
#define macro_decl_string_concat(dest,string1,string2) 			\
	const char* _macro_string1 ## dest = string1; 			\
	const char* _macro_string2 ## dest = string2; 			\
	size_t _macro_len1 ## dest = 					\
		strlen(_macro_string1 ## dest); 			\
	size_t _macro_len2 ## dest = 					\
		strlen(_macro_string2 ## dest); 			\
	char dest[_macro_len1 ## dest + 				\
		      _macro_len2 ## dest + 1]; 			\
	memcpy(dest, _macro_string1 ## dest, 				\
	       	_macro_len1 ## dest); 					\
	memcpy(dest + _macro_len1 ## dest, 				\
		_macro_string2 ## dest, _macro_len2 ## dest); 		\
	dest[_macro_len1 ## dest + _macro_len2 ## dest] = '\0'
	
#define macro_unused(a) (void)(a);
	
#endif
	
#ifdef __cplusplus
}
#endif
