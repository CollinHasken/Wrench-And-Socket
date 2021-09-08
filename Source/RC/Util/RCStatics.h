// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RCStatics.generated.h"

/**
 * 
 */
UCLASS()
class RC_API URCStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Game")
	static float MillisecondsToSeconds(int Milliseconds) { return Milliseconds / 1000.f; }

	UFUNCTION(BlueprintPure, Category = "Game")
	static int SecondsToMilliseconds(float Milliseconds) { return static_cast<int>(Milliseconds * 1000); }

	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsActorPlayer(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsActorHuman(const AActor* Actor);
};
