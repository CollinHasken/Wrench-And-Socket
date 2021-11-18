// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "CoreMinimal.h"

#include "TimeStamp.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTimeStamp, Log, All);

/**
 * A way to set a time stamp in the future and check when its elapsed. For simpler use cases than timer manager
 */
class RC_API FTimeStamp
{
public:

	/**
	 * Set the time until it becomes elapsed
	 * 
	 * @param NewTime	The new time in milliseconds
	 */
	void Set(const unsigned int NewTime);

	/**
	 * Set the time until it becomes elapsed
	 *
	 * @param NewTime	The new time in seconds
	 */
	void Set(const float NewTime);

	// Invalidate the time stamp
	void Invalidate();

	/**
	 * Test if the timer has elapsed
	 * 
	 * @Return True if the timer has elapsed and is valid
	 */
	bool Elapsed() const;

	/**
	 * Get the time since the timer started
	 *
	 * @Return The time since the timer started
	 */
	float GetTimeSince() const;

	/**
	 * Get the time until the timer ends
	 *
	 * @Return The time until the timer ends
	 */
	float GetTimeRemaining() const;

	/**
	 * Get the total duration of the timer
	 *
	 * @Return The total duration of the timer
	 */
	float GetTotalDuration() const;

	/**
	 * Is the time stamp valid
	 *
	 * @Return True if the timestamp has been set
	 */
	bool IsValid() const;

	/**
	 * Is the timer valid and waiting to elapse
	 * 
	 * @Return True if the timestamp is valid and nto elapsed
	 */
	bool IsActive() const;

private:

	// The time when this will elapse
	float Time = -1;

	// The time the timer started at
	float StartTime = -1;
};


/**
 * Subsystem for timestamps
 */
UCLASS()
class RC_API UTimeStampSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UTimeStampSubsystem();
	~UTimeStampSubsystem() override;
};