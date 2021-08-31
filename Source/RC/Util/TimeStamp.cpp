// Fill out your copyright notice in the Description page of Project Settings.
#include "RC/Util/TimeStamp.h"

#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

#include "RC/Debug/Debug.h"
#include "RC/Util/RCStatics.h"

static UTimeStampSubsystem* TimeStampSubsystem = nullptr;

DEFINE_LOG_CATEGORY(LogTimeStamp);

void FTimeStamp::Set(const float NewTime)
{
	if (TimeStampSubsystem == nullptr)
	{
		UE_LOG(LogTimeStamp, Error, TEXT("TimeStamp Subsystem not initialized yet, trying to set a timer without a world?"));
		return;
	}
	ASSERT_RETURN(TimeStampSubsystem->GetWorld() != nullptr, "TimeStamp Subsystem doesn't have a world");	

	StartTime = TimeStampSubsystem->GetWorld()->GetTimeSeconds();
	Time = StartTime + NewTime;
}

void FTimeStamp::Set(const unsigned int NewTime)
{
	// Forward with it in seconds
	Set(URCStatics::MillisecondsToSeconds(NewTime));
}

void FTimeStamp::Invalidate()
{
	Time = -1;
	StartTime = -1;
}

bool FTimeStamp::Elapsed()
{
	if (!IsValid())
	{
		return false;
	}

	if (TimeStampSubsystem == nullptr)
	{
		UE_LOG(LogTimeStamp, Error, TEXT("TimeStamp Subsystem not initialized yet, trying to set a timer without a world?"));
		return false;
	}
	ASSERT_RETURN_VALUE(TimeStampSubsystem->GetWorld() != nullptr, false, "TimeStamp Subsystem doesn't have a world");

	return TimeStampSubsystem->GetWorld()->GetTimeSeconds() > Time || FMath::IsNearlyEqual(TimeStampSubsystem->GetWorld()->GetTimeSeconds(), Time);
}

float FTimeStamp::GetTimeSince()
{
	if (TimeStampSubsystem == nullptr)
	{
		UE_LOG(LogTimeStamp, Error, TEXT("TimeStamp Subsystem not initialized yet, trying to set a timer without a world?"));
		return false;
	}
	ASSERT_RETURN_VALUE(TimeStampSubsystem->GetWorld() != nullptr, false, "TimeStamp Subsystem doesn't have a world");

	return TimeStampSubsystem->GetWorld()->GetTimeSeconds() - StartTime;
}

bool FTimeStamp::IsValid()
{
	return Time != -1;
}

bool FTimeStamp::IsActive()
{
	return IsValid() && !Elapsed();
}

UTimeStampSubsystem::UTimeStampSubsystem() : UWorldSubsystem()
{
	ASSERT_RETURN(TimeStampSubsystem == nullptr, "There should only be one Time Stamp Subsystem");
	if (GetWorld() == nullptr)
	{
		return;
	}

	// Only game worlds
	EWorldType::Type WorldType = GetWorld()->WorldType;
	if (WorldType != EWorldType::Game && WorldType != EWorldType::PIE)
	{
		return;
	}

	TimeStampSubsystem = this;
}

UTimeStampSubsystem::~UTimeStampSubsystem()
{
#if WITH_EDITOR
	if (GetWorld() == nullptr)
	{
		return;
	}
	
	// Only game worlds
	EWorldType::Type WorldType = GetWorld()->WorldType;
	if (WorldType != EWorldType::PIE)
	{
		return;
	}
#endif

	TimeStampSubsystem = nullptr;
}