// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RCStatics.generated.h"

/**
 * Common Statics
 */
UCLASS()
class RC_API URCStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Convert from milliseconds to seconds
	 * @param Milliseconds	Milliseconds to convert
	 * @Return The converted seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static float MillisecondsToSeconds(int Milliseconds) { return Milliseconds / 1000.f; }

	/**
	 * Convert from seconds to milliseconds
	 * @param Seconds	Seconds to convert
	 * @Return The converted milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static int SecondsToMilliseconds(float Milliseconds) { return static_cast<int>(Milliseconds * 1000); }

	/**
	 * Is the actor the player
	 * @param Actor	The actor to check
	 * @Return True if the actor is the player
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsActorPlayer(const AActor* Actor);

	/**
	 * Is the actor a human
	 * @param Actor	The actor to check
	 * @Return True if the actor is a human
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsActorHuman(const AActor* Actor);
};
