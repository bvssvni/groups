/*
 *  member.h
 *  MemGroups
 *
 *  Created by Sven Nilsen on 25.05.12.
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

#ifndef memgroups_member
#define memgroups_member
	
#include "gcstack.h"
#include "variable.h"
	
typedef struct member {
	gcstack_item gc;
	gcstack* variables;
	int m_ready;
	variable** m_variableArray;
} member;

	void member_Delete
	(void* p);
	
	member* member_AllocWithGC
	(gcstack* gc);
	
	member* member_Init
	(member* obj);
	
	member* member_InitWithMember
	(member* obj, member* b);
	
	void member_AddDouble
	(member* obj, int propId, double val);
	
	void member_AddString
	(member* obj, int propId, char const* val);
	
	void member_AddInt
	(member* obj, int propId, int val);
	
	void member_AddBool
	(member* obj, int propId, int val);
	
	//
	// Uses binary search to find the index of a property id.
	//
	int member_IndexOf
	(member* obj, int propId);
	
#endif

#ifdef __cplusplus
}
#endif
