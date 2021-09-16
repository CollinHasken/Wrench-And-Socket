// Fill out your copyright notice in the Description page of Project Settings.
#include "RCPlayerState.h"

#include "RC/Debug/Debug.h"
#include "RC/Save/RCSaveGame.h"
#include "RC/Weapons/Weapons/BasePlayerWeapon.h"
#include "RC/Characters/Player/RCCharacter.h"


// Save player data
void ARCPlayerState::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaveGame())
	{
		UClass* Class = ARCPlayerState::StaticClass();
		Class->SerializeTaggedProperties(Ar, (uint8*)this, Class, nullptr);
	}
}

// Save the player data for a level transition 
void ARCPlayerState::SaveForLevelTransition(URCLevelTransitionSave* SaveGame)
{
	ASSERT_RETURN(SaveGame != nullptr);

	// Pass the array to fill with data from Player State
	FMemoryWriter MemWriter(SaveGame->SavedPlayer.PlayerStateByteData);
	FSaveGameArchive Ar(MemWriter, true);
	Serialize(Ar);

	ARCCharacter* ControlledCharacter = Cast<ARCCharacter>(GetPawn());
	ASSERT_RETURN(ControlledCharacter != nullptr);

	SaveGame->SavePlayer(ControlledCharacter);
}

// Load the player data for a level transition
void ARCPlayerState::LoadForLevelTransition(const URCLevelTransitionSave* SaveGame)
{
	// Convert binary array back into actor's variables
	FMemoryReader MemReader(SaveGame->SavedPlayer.PlayerStateByteData);
	FSaveGameArchive Ar(MemReader, true);
	Serialize(Ar);

	ARCCharacter* ControlledCharacter = Cast<ARCCharacter>(GetPawn());
	ASSERT_RETURN(ControlledCharacter != nullptr);

	SaveGame->LoadPlayer(ControlledCharacter);
}

// Find or add the weapon data for a specific class
FWeaponData& ARCPlayerState::FindOrAddWeaponDataForClass(UClass* WeaponClass)
{
	FWeaponData* WeaponData = FindWeaponDataForClass(WeaponClass);
	return WeaponData != nullptr ? *WeaponData : AddWeaponDataForClass(WeaponClass);
}

// Add a weapon data for a specific class
FWeaponData& ARCPlayerState::AddWeaponDataForClass(UClass* WeaponClass)
{
	// Make sure it's unique
	FWeaponData* ExistingWeaponData = FindWeaponDataForClass(WeaponClass);
	if (ExistingWeaponData != nullptr)
	{
		ASSERT(ExistingWeaponData != nullptr, "Trying to add weapon data for existing class");
		return *ExistingWeaponData;
	}

	// Add data to the map
	FWeaponData& WeaponData = Data.WeaponDataMap.Add(WeaponClass);

	// Set the values to the default
	ABasePlayerWeapon* DefaultWeapon = WeaponClass->GetDefaultObject<ABasePlayerWeapon>();
	if (DefaultWeapon != nullptr)
	{
		FWeaponLevelInfo LevelConfigs[ABasePlayerWeapon::MAX_LEVELS];
		DefaultWeapon->GetLevelConfigs(LevelConfigs);
		const FWeaponConfig& Config = DefaultWeapon->GetConfig();

		WeaponData.CurrentAmmo = Config.BaseMaxAmmo;
		WeaponData.MaxAmmo = Config.BaseMaxAmmo;
		WeaponData.XPTotalForNextLevel = LevelConfigs[1].XPNeeded;
	}

	return WeaponData;
}
