// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug.h"

#if DEBUG_ENABLED
#include <cstdlib>

#include "Containers/UnrealString.h"
#include "Engine/World.h"
#include "Internationalization/Text.h"
#include "Misc/MessageDialog.h"

DEFINE_LOG_CATEGORY(LogAI);
DEFINE_LOG_CATEGORY(LogAISpline);
DEFINE_LOG_CATEGORY(LogCollectible);
DEFINE_LOG_CATEGORY(LogSave);
DEFINE_LOG_CATEGORY(LogUI);
DEFINE_LOG_CATEGORY(LogWeapon);

bool PrintAssertMessage(const char* Cond, const char* File, int Line, const char* Format, ...)
{
	va_list Args;
	va_start(Args, Format);

	char Message[4096];
	vsprintf_s(Message, Format, Args);
	va_end(Args);

	char FullMessage[4096];
	_snprintf_s(FullMessage, 4096, "%s, %s\n\nFile: %s\nLine: %d", Cond, Message, File, Line);

	const FText Title = FText::FromString(FString("Assertion Failed!"));
	const FText Body = FText::FromString(FString(FullMessage));

	EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::Type::YesNoCancel, EAppReturnType::Type::Yes, Body, &Title);
	switch (Result) {
	case EAppReturnType::Type::Yes:
		return true;

	case EAppReturnType::Type::No:
		return false;

	case EAppReturnType::Type::Cancel:
		FGenericPlatformMisc::RequestExit(false);
		return false;
	};

	return true;
}

bool PrintAssertMessage(const char* Cond, const char* File, int Line)
{
	return PrintAssertMessage(Cond, File, Line, "Assert Hit");
}
#endif
