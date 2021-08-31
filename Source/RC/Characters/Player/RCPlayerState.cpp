// Fill out your copyright notice in the Description page of Project Settings.
#include "RCPlayerState.h"

#include "RC/Debug/Debug.h"
#include "RC/Save/RCSaveGame.h"
#include "RC/Characters/Player/RCCharacter.h"


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

void ARCPlayerState::LoadForLevelTransition(const URCLevelTransitionSave* SaveGame)
{
	ARCCharacter* ControlledCharacter = Cast<ARCCharacter>(GetPawn());
	ASSERT_RETURN(ControlledCharacter != nullptr);

	SaveGame->LoadPlayer(ControlledCharacter);
}
