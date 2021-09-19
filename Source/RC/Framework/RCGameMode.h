// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RCGameMode.generated.h"

/**
 * Default game mode
 */
UCLASS(minimalapi)
class ARCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARCGameMode();

	/**
	 * Called when a new player is spawned
	 * Load in the level transition persistent data if we were loading a level
	 * @param	NewPlayer The new player
	 */
	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/**
	 * Save the persistent data for a level transition
	 * @param Data The persistent data
	 */
	void SaveLevelTransitionDataToMemory(TArray<uint8>* Data);
};



