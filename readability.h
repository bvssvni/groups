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

// This is to make the code easier to read and to import from C#.
#define bool int
#define true 1
#define false 0
#define string char const*
#define byte unsigned char

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

/*******************************COPY********************************
 
#define foreach_reverse(a) \
int __start##a, __end##a, __i##a, __j##a; \
for (__i##a = a->length-2; __i##a >= 0; __i##a -= 2) { \
__start##a = a->pointer[__i##a]; \
__end##a = a->pointer[__i##a+1]; \
for (__j##a = __end##prop-1; __j##a >= __start##a; __j##a--) {

#define foreach(a) \
int __len##a = a->length-1; \
int __start##a, __end##a, __i##a, __j##a; \
for (__i##a = 0; __i##a < __len##a; __i##a += 2) { \
__start##a = a->pointer[__i##a]; \
__end##a = a->pointer[__i##a+1]; \
for (__j##a = __start##prop; __j##a < __end##a; __j##a++) {

#define end_foreach }}__BREAK_BITSTREAM_prop:;

#define _break(a)     goto __BREAK_BITSTREAM_##a

#define _pos(a)    __j##a

*******************************************************************/
