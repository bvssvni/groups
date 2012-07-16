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
	bitstream* boolean_GcEval
	(gcstack* const gc, groups* const g, const char* const expr, 
	 void (* const err)(int pos, const char* message));

	void boolean_RunUnitTests
	(void);
	
	//
	//	IDEA FOR FUTURE DEVELOPMENT: BOOLEAN FUNCTIONS
	//
	//	Example of a Boolean function: 
	//
	//	Name * (> Age 14)	which equals
	//	Name - (<= Age 13)
	//
	//	Boolean functions operates on a set and returns a subset.
	//	The fastest way to execute them is to first calculate the set.
	//	Unlike normal functions, a Boolean function has an inverted
	//	function by default, since it supports "NOT" behind the scene.
	//
	//	less
	//	lessOrEqual
	//	more
	//	moreOrEqual
	//	equal
	//	range
	//	max
	//	min
	//	first
	//	last
	//	middle
	//	longestText
	//	shortedText
	//	matchesRegex
	//	odd
	//	even
	//	multiplyOf
	//	prime
	//
	//	IDEA FOR FUTURE DEVELOPMENT: PARAMETER PASSING
	//
	//	boolean_Eval(g, "%b + %b", displayErr, a, b);
	//
	//	Like the 'printf' command, support parameters passed to the
	//	function for easier use.
	//
	
#endif

#ifdef __cplusplus
}
#endif
