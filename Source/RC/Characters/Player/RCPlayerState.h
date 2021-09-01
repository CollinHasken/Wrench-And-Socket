// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "RC/Util/RCTypes.h"

#include "RCPlayerState.generated.h"


USTRUCT()
struct FPlayerStateData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(SaveGame)
	TMap<UClass*, FWeaponData> WeaponDataMap;
};

/**
 * 
 */
UCLASS()
class RC_API ARCPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void Serialize(FArchive& Ar) override;

	void SaveForLevelTransition(class URCLevelTransitionSave* SaveGame);
	void LoadForLevelTransition(const class URCLevelTransitionSave* SaveGame);

	/**
	 * Find or add the weapon data for a specific class
	 * @param WeaponClass	The class of the weapon to find
	 * 
	 * Returns the weapon data
	 */
	inline FWeaponData& FindOrAddWeaponDataForClass(UClass* WeaponClass) { return Data.WeaponDataMap.FindOrAdd(WeaponClass); }

private:
	UPROPERTY(SaveGame)
	FPlayerStateData Data;
};
