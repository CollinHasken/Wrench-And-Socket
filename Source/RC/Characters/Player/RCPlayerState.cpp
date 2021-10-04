// Fill out your copyright notice in the Description page of Project Settings.
#include "RCPlayerState.h"

#include "RC/Debug/Debug.h"
#include "RC/Save/RCSaveGame.h"
#include "RC/Weapons/Weapons/BasePlayerWeapon.h"
#include "RC/Characters/Player/RCCharacter.h"


FArchive& operator<<(FArchive& Ar, FPlayerStateData& SObj)
{
	Ar << SObj.DataStructMap;
	return Ar;
}

// Save player data
void ARCPlayerState::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaveGame())
	{
		UClass* Class = ARCPlayerState::StaticClass();
		Class->SerializeTaggedProperties(Ar, (uint8*)this, Class, nullptr);

		Ar << SaveData;
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
