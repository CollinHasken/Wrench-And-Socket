// Copyright Epic Games, Inc. All Rights Reserved.

#include "RCGameMode.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/ConstructorHelpers.h"

#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Framework/RCGameInstance.h"
#include "RC/Save/RCSaveGame.h"
#include "RC/Save/SaveGameInterface.h"

ARCGameMode::ARCGameMode()
{
}

// Called when a new player is spawned
void ARCGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ARCPlayerState* PlayerState = NewPlayer->GetPlayerState<ARCPlayerState>();
	if (PlayerState)
	{
		URCGameInstance* GameInstance = GetGameInstance<URCGameInstance>();
		ASSERT_RETURN(GameInstance != nullptr);

		// Load level transition data if its there
		const TArray<uint8>& LevelTransitionData = GameInstance->GetLevelTransitionData();
		if (LevelTransitionData.Num() != 0)
		{
			URCLevelTransitionSave* LevelTransitionSave = Cast<URCLevelTransitionSave>(UGameplayStatics::LoadGameFromMemory(LevelTransitionData));
			if (LevelTransitionSave != nullptr)
			{
				PlayerState->LoadForLevelTransition(LevelTransitionSave);
			}

			Super::HandleStartingNewPlayer_Implementation(NewPlayer);
		}
	}
	else
	{
		ASSERT(PlayerState != nullptr, "Player doesn't have our player state");
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

// Save the persistent data for a level transition
void ARCGameMode::SaveLevelTransitionDataToMemory(TArray<uint8>* Data)
{
	URCLevelTransitionSave* LevelTransitionSave = Cast<URCLevelTransitionSave>(UGameplayStatics::CreateSaveGameObject(URCLevelTransitionSave::StaticClass()));
	ASSERT_RETURN(LevelTransitionSave != nullptr);

	ASSERT_RETURN(GameState != nullptr);
	ASSERT_RETURN(GameState->PlayerArray.Num() == 1);

	ARCPlayerState* PlayerState = Cast<ARCPlayerState>(GameState->PlayerArray[0]);
	ASSERT_RETURN(PlayerState != nullptr);

	// Save off the player's data
	PlayerState->SaveForLevelTransition(LevelTransitionSave);

	/*
	* TODO
	* Save off other actors
	// Iterate the entire world of actors
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		// Only interested in our 'gameplay actors', skip actors that are being destroyed
		if (Actor->IsPendingKill() || !Actor->Implements<ISaveGameInterface>())
		{
			continue;
		}		

		CurrentSaveGame->SaveActor(Actor);
	}
	*/

	// Put savegame into data array
	UGameplayStatics::SaveGameToMemory(LevelTransitionSave, *Data);
	UGameplayStatics::SaveGameToSlot(LevelTransitionSave, "Test", 0);
}