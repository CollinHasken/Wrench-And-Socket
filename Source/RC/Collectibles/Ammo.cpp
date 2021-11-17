// Fill out your copyright notice in the Description page of Project Settings.
#include "Ammo.h"

#include "RC/Util/RCStatics.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Weapons/RCWeaponTypes.h"

// Set the default values the data should have
void UAmmoData::SetDefaults(const FPrimaryAssetId& Id)
{
	Super::SetDefaults(Id);

	const UAmmoInfo* AmmoInfo = URCStatics::GetPrimaryAssetObject<UAmmoInfo>(AssetId);
	ASSERT_RETURN(AmmoInfo != nullptr, "Asset wasn't the correct type of UAmmoInfo %s", *AssetId.ToString());

	// Stored to prevent looking up each collectible grant
	WeaponInfoId = AmmoInfo->WeaponInfoId;
}

// Grant an amount of the collectible
void UAmmoData::GrantCollectible(int Amount)
{
	ARCPlayerState* PlayerState = URCStatics::GetPlayerState(GetWorld());
	ASSERT_RETURN(PlayerState != nullptr, "Unable to get player state");

	UPlayerWeaponData* PlayerWeaponData = PlayerState->FindOrAddDataForAsset<UPlayerWeaponData>(WeaponInfoId);
	ASSERT_RETURN(PlayerWeaponData != nullptr, "Unable to get weapon data for ammo %s for weapon %s",*AssetId.ToString(), *WeaponInfoId.ToString());

	PlayerWeaponData->GrantAmmo(Amount);
}

// Called when the game starts or when spawned
void AAmmo::BeginPlay()
{
	// Need to determine the ammo before the super uses it
	DetermineAmmo();
	
	Super::BeginPlay();
}

// Determine which ammo type and amount this should give
void AAmmo::DetermineAmmo()
{
	ARCPlayerState* PlayerState = URCStatics::GetPlayerState(GetWorld());
	ASSERT_RETURN(PlayerState != nullptr, "Unable to get player state");

	const TMap<FPrimaryAssetId, UBaseData*>* WeaponDataMap = PlayerState->FindDataMap(UPlayerWeaponData::StaticClass());
	// If weapon data isn't there, then none of the guns have been shot
	// Just go with default and return
	if (WeaponDataMap == nullptr)
	{
		return;
	}

	// Find the weapon with the lowest percentage full ammo
	float LowestPercentFull = 1;
	const TPair<FPrimaryAssetId, UBaseData*>* LowestWeaponPair = nullptr;
	for (const TPair<FPrimaryAssetId, UBaseData*> WeaponDataPair : (*WeaponDataMap))
	{
		UPlayerWeaponData* PlayerWeaponData = Cast<UPlayerWeaponData>(WeaponDataPair.Value);
		ASSERT_CONTINUE(PlayerWeaponData != nullptr, "Data %s in player weapon data map but isn't of type UPlayerWeaponData", *WeaponDataPair.Key.ToString());

		float PercentFull = (float)PlayerWeaponData->CurrentAmmo / PlayerWeaponData->MaxAmmo;
		if (PercentFull <= LowestPercentFull)
		{
			LowestPercentFull = PercentFull;
			LowestWeaponPair = &WeaponDataPair;
		}
	}
	ASSERT_RETURN(LowestWeaponPair != nullptr, "Data map was found but no weapon found");

	// Get the ammo that this weapon uses
	const UPlayerWeaponInfo* WeaponInfo = URCStatics::GetPrimaryAssetObject<UPlayerWeaponInfo>(LowestWeaponPair->Key);
	ASSERT_RETURN(WeaponInfo != nullptr, "Unable to get weapon info %s", *LowestWeaponPair->Key.ToString());

	CollectibleInfo = static_cast<UCollectibleInfo*>(WeaponInfo->AmmoInfo);
}
