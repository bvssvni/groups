//
//  errorhandling.h
//  MemGroups
//
//  Created by Sven Nilsen on 11.06.12.
//  Copyright (c) 2012 Cutout Pro. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef MemGroups_errorhandling_h
#define MemGroups_errorhandling_h
	
	//
	//      CRASH HANDLING
	//
	
	void errorhandling_CrashWithMessage
	(const char* message);
	
	void errorhandling_CrashWithFunctionAndMessage
	(const char* functionName, const char* message);
	
	void errorhandling_CrashWithLineAndFunctionAndMessage
	(int line, const char* functionName, const char* message);
	
	void errorhandling_CrashExpression
	(const char* message, int pos, const char* expr);
	
	//
	//      CRASH CONFIGURATION
	//
	
	void errorhandling_OnCrashExitApp();
	void errorhandling_OnCrashExitThread();
	
#endif
	
#ifdef __cplusplus
}
#endif
