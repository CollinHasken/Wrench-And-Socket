// Fill out your copyright notice in the Description page of Project Settings.
#include "RCWeaponTypes.h"

#include "RC/Debug/Debug.h"
#include "RC/Util/RCStatics.h"
#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"


// Set the default values the data should have
void UPlayerWeaponData::SetDefaults(const FPrimaryAssetId& Id)
{
	Super::SetDefaults(Id);

	const UPlayerWeaponInfo* PlayerWeaponInfo = URCStatics::GetPrimaryAssetObject<UPlayerWeaponInfo>(AssetId);
	ASSERT_RETURN(PlayerWeaponInfo != nullptr, "Asset wasn't the correct type of UPlayerWeaponInfo %s", *AssetId.ToString());
	
	XPTotalForNextLevel = PlayerWeaponInfo->WeaponLevelConfigs[CurrentLevelIndex + 1].XPNeeded;

	if (PlayerWeaponInfo->bHasProjectile)
	{
		CurrentAmmo = PlayerWeaponInfo->BaseMaxAmmo;
		MaxAmmo = PlayerWeaponInfo->BaseMaxAmmo;
	}
}

// Grant XP
void UPlayerWeaponData::GrantDamageXP(float XP)
{
	CurrentXP += XP;

	// If the weapon still exists, let it know
	ABasePlayerWeapon* Weapon = CurrentWeapon.Get();
	if (Weapon != nullptr)
	{
		Weapon->OnXPGained(XP);
	}
}

// Grant ammo
void UPlayerWeaponData::GrantAmmo(int Ammo)
{
	CurrentAmmo = FMath::Min(CurrentAmmo + Ammo, MaxAmmo);
}
