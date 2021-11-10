#pragma once

#define DEBUG_ENABLED !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

#if DEBUG_ENABLED

#include "GenericPlatform/GenericPlatformMisc.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAISpline, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCollectible, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSave, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

/**********
TODO
Create console variable struct that stores the default
Store array of variables
Reset them when stopping PIE

***********/


bool PrintAssertMessage(const char* Cond, const char* File, int Line);
bool PrintAssertMessage(const char* Cond, const char* File, int Line, const char* Format, ...);

#define stringify(name) #name

#define ASSERT(Cond, ...)															\
	if (!(Cond)) {																	\
		if (PrintAssertMessage(#Cond, __FILE__, __LINE__, ##__VA_ARGS__)) {			\
			UE_DEBUG_BREAK();														\
		}																			\
	}																	

#define ASSERT_RETURN(Cond, ...)        	   				 						\
	if (!(Cond)) {																	\
		if (PrintAssertMessage(#Cond, __FILE__, __LINE__, ##__VA_ARGS__)) {			\
			UE_DEBUG_BREAK();														\
		}																			\
		return; 																	\
	}																	

#define ASSERT_RETURN_VALUE(Cond, RV, ...)											\
	if (!(Cond)) {																	\
		if (PrintAssertMessage(#Cond, __FILE__, __LINE__, ##__VA_ARGS__)) {			\
			UE_DEBUG_BREAK();														\
		}																			\
		return RV;																	\
	}																	

#define ASSERT_CONTINUE(Cond, ...)													\
	if (!(Cond)) {																	\
		if (PrintAssertMessage(#Cond, __FILE__, __LINE__, ##__VA_ARGS__)) {			\
			UE_DEBUG_BREAK();														\
		}																			\
		continue;																	\
	}			

#define LOG_CHECK(Cond, CategoryName, Verbosity, Format, ...)        	   			\
	if (!(Cond)) {																	\
		UE_LOG(CategoryName, Verbosity, TEXT(Format), ##__VA_ARGS__);				\
	}	

#define LOG_RETURN(Cond, CategoryName, Verbosity, Format, ...)        	   			\
	if (!(Cond)) {																	\
		UE_LOG(CategoryName, Verbosity, TEXT(Format), ##__VA_ARGS__);				\
		return; 																	\
	}		

#define LOG_RETURN_VALUE(Cond, RV, CategoryName, Verbosity, Format, ...)			\
	if (!(Cond)) {																	\
		UE_LOG(CategoryName, Verbosity, TEXT(Format), ##__VA_ARGS__);				\
		return RV;																	\
	}																	

#define LOG_CONTINUE(Cond, CategoryName, Verbosity, Format, ...)					\
	if (!(Cond)) {																	\
		UE_LOG(CategoryName, Verbosity, TEXT(Format), ##__VA_ARGS__);				\
		continue;																	\
	}	

#else

#define stringify(name)
#define ASSERT(...)
#define ASSERT_RETURN(Cond, ...) if (!(Cond)) { return; }
#define ASSERT_RETURN_VALUE(Cond, RV, ...) if (!(Cond)) { return RV; }
#define ASSERT_CONTINUE(Cond, ...) if (!(Cond)) { continue; }
#define LOG_CHECK(...)
#define LOG_RETURN(Cond, ...) if (!(Cond)) { return; }
#define LOG_RETURN_VALUE(Cond, RV, ...) if (!(Cond)) { return RV; }
#define LOG_CONTINUE(Cond, ...) if (!(Cond)) { continue; }

#endif // !UE_BUILD_SHIPPING