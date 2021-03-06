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
	/**
	 * Initialize the game.
	 * The GameMode's InitGame() event is called before any other functions (including PreInitializeComponents() )
	 * and is used by the GameMode to initialize parameters and spawn its helper classes.
	 * @warning: this is called before actors' PreInitializeComponents.
	 */
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/**
	 * Called when a new player is spawned
	 * Load in the level transition persistent data if we were loading a level
	 * @param	NewPlayer The new player
	 */
	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/**
	 * Save the persistent data for a level transition
	 * @param Data The persistent data
	 * @Returns whether the save succeeded
	 */
	bool SaveLevelTransitionDataToMemory(TArray<uint8>* Data);

	// Save the given actor to the current save
	// Useful for when theres data to save for an actor that's being deleted
	void SaveActorForLevelTransition(AActor* Actor);

protected:
	/**
	 * Called when the player has died
	 * @param Player	The player that died
	 */
	UFUNCTION()
	void OnPlayerDied(AActor* Player);

	// Create a new save data for the current level
	void InitializeLevelTransitionData();

	// Load the data we saved for transitioning between or reseting levels
	void LoadLevelTransitionData();

	// Current save
	UPROPERTY()
	class URCLevelTransitionSave* CurrentSave = nullptr;
};
