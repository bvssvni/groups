//
//  boolean.h
//  MemGroups
//
//  Created by Sven Nilsen on 29.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef MemGroups_boolean_h
#define MemGroups_boolean_h

	//
	//	BOOLEAN EXPRESSION EVALUATOR
	//
	//	This is a fast and simple evaluation of expressions with
	//	AND, OR, EXCEPT represented through '*', '+', '-'.
	//
	//	Example:	A * B - C
	//
	//	At this moment it only supports bitstream properties.
	//	It evaluates A + B + C = (A + B) + C
	//	This makes it possible to add Boolean functions in the future.
	//
	//	The idea is to provide easy prototyping and power-user features
	//	without need for explicit implementing of these functions.
	//
	group* boolean_GcEval
	(gcstack* const gc, gop* const g, const char* const expr, 
	 void (* const err)(int pos, const char* message));
	
#endif

#ifdef __cplusplus
}
#endif
