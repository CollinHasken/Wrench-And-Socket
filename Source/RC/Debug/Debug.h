#pragma once

#if !UE_BUILD_SHIPPING

#include "GenericPlatform/GenericPlatformMisc.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAISpline, Log, All);

bool PrintAssertMessage(const char* Cond, const char* File, int Line);
bool PrintAssertMessage(const char* Cond, const char* File, int Line, const char* Format, ...);

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

#define ASSERT(...)
#define ASSERT_RETURN(Cond, ...) if (!(Cond)) { return; }
#define ASSERT_RETURN_VALUE(Cond, RV, ...) if (!(Cond)) { return RV; }
#define ASSERT_CONTINUE(Cond, ...) if (!(Cond)) { continue; }
#define LOG_RETURN(Cond, ...) if (!(Cond)) { return; }
#define LOG_RETURN_VALUE(Cond, RV, ...) if (!(Cond)) { return RV; }
#define LOG_CONTINUE(Cond, ...) if (!(Cond)) { continue; }

#endif // !UE_BUILD_SHIPPING