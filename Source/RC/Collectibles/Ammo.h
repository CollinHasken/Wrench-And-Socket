// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/Collectibles/Collectible.h"
#include "RC/Util/RCTypes.h"

#include "Ammo.generated.h"

/**
* Ammo collectible info
*/
UCLASS(BlueprintType)
class RC_API UAmmoInfo : public UCollectibleInfo
{
	GENERATED_BODY()

public:
	// Id of the weapon info for the ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	FPrimaryAssetId WeaponInfoId;
};

/**
 * Ammo save data
 */
UCLASS(BlueprintType)
class UAmmoData : public UCollectibleData
{
	GENERATED_BODY()

public:
	/**
	 * Set the default values the data should have
	 *
	 * @param Id	The asset Id for the data
	 */
	void SetDefaults(const FPrimaryAssetId& Id) override;

	/**
	 * Grant an amount of ammo
	 * @param Amount The amount of ammo to grant
	 */
	void GrantCollectible(int Amount) override;

private:
	// Id of the weapon info for the ammo
	// Stored to prevent looking up each collectible grant
	FPrimaryAssetId WeaponInfoId;
};

/**
 * Ammo actor that will dynamically select the type of ammo to grant
 * based off of the weapon with the least percentage of ammo
 */
UCLASS()
class RC_API AAmmo : public ACollectible
{
	GENERATED_BODY()
	
public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Determine which ammo type and amount this should give
	void DetermineAmmo();
};
