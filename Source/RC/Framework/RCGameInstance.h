// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RCGameInstance.generated.h"

// Broadcasts when the we auto save
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAutoSave);

/**
 * Instance of the game
 */
UCLASS()
class RC_API URCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	// Make a save in the auto save
	UFUNCTION(BlueprintCallable)
	bool AutoSave();

	/**
	 * Request to load a new level
	 * Serializes and loads persistent data
	 * @param LevelName	Name of the level map to load
	 */
	UFUNCTION(BlueprintCallable)
	void RequestNewLevel(const FName& LevelName);

	// Reset the current level, fading in and out the camera
	UFUNCTION(BlueprintCallable)
	void ResetLevel();

	// Whether the level is currently being restarted
	bool IsResettingLevel() const { return bIsResetting; }

	const TArray<uint8>& GetLevelTransitionData() { return LevelTransitionData; }

	// Get the auto save delegate
	FOnAutoSave& OnAutoSave() { return AutoSaveDelegate; }

protected:
	// Timer to have a minimum time for the reset
	FTimerHandle ResetFadeTimer;

	bool bIsResetting = false;

private:
	// Broadcasts when the actor this component is on has been damaged
	UPROPERTY(BlueprintAssignable, Category = Save, meta = (AllowPrivateAccess))
	FOnAutoSave AutoSaveDelegate;

	TArray<uint8> LevelTransitionData;
};
