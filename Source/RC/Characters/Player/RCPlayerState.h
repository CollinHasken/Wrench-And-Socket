// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "RC/Util/RCTypes.h"

#include "RCPlayerState.generated.h"

/**
 * Player state data that will be saved
 */
USTRUCT()
struct FPlayerStateData
{
	GENERATED_USTRUCT_BODY()

public:
	// Weapon data for weapons the player owns
	UPROPERTY(SaveGame)
	TMap<UClass*, FWeaponData> WeaponDataMap;
};

/**
 * Player state
 */
UCLASS()
class RC_API ARCPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// Save player data
	virtual void Serialize(FArchive& Ar) override;

	/**
	 * Save the player data for a level transition 
	 * @param SaveGame	The Save to save to
	 */
	void SaveForLevelTransition(class URCLevelTransitionSave* SaveGame);

	/**
	 * Load the player data for a level transition
	 * @param SaveGame	The Save to load from
	 */
	void LoadForLevelTransition(const class URCLevelTransitionSave* SaveGame);

	/**
	 * Find or add the weapon data for a specific class
	 * @param WeaponClass	The class of the weapon to find
	 * 
	 * Returns the weapon data
	 */
	FWeaponData& FindOrAddWeaponDataForClass(UClass* WeaponClass);

	/**
	 * Find the weapon data for a specific class
	 * @param WeaponClass	The class of the weapon to find
	 *
	 * Returns the weapon data if found
	 */
	inline FWeaponData* FindWeaponDataForClass(UClass* WeaponClass) { return Data.WeaponDataMap.Find(WeaponClass); }

	/**
	 * Add a weapon data for a specific class
	 * @param WeaponClass	The class of the weapon to add
	 *
	 * Returns the weapon data that was added
	 */
	FWeaponData& AddWeaponDataForClass(UClass* WeaponClass);

private:
	// Player save data
	UPROPERTY(SaveGame)
	FPlayerStateData Data;
};
