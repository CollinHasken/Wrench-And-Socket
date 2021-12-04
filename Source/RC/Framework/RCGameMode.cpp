// Copyright Epic Games, Inc. All Rights Reserved.

#include "RCGameMode.h"

#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/ConstructorHelpers.h"

#include "RC/Characters/Components/HealthComponent.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Framework/RCGameInstance.h"
#include "RC/Save/RCSaveGame.h"
#include "RC/Save/SaveGameInterface.h"

void ARCGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	URCGameInstance* GameInstance = GetGameInstance<URCGameInstance>();
	ASSERT_RETURN(GameInstance != nullptr);
	
	// Only load data for reset
	if (GameInstance->IsResettingLevel())
	{
		LoadLevelTransitionData();
	}
	else
	{
		InitializeLevelTransitionData();
	}
}

// Called when a new player is spawned
void ARCGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ARCPlayerState* PlayerState = NewPlayer->GetPlayerState<ARCPlayerState>();
	if (PlayerState)
	{
		URCGameInstance* GameInstance = GetGameInstance<URCGameInstance>();
		ASSERT_RETURN(GameInstance != nullptr);

		ARCCharacter* Player = NewPlayer->GetPawn<ARCCharacter>();
		ASSERT_RETURN(Player != nullptr);

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

			// Now that the player state has loaded, move the player to any existing checkpoint
			FTransform CheckpointTransform;
			if (PlayerState->GetCheckpoint(CheckpointTransform))
			{
				Player->SetActorTransform(CheckpointTransform, false, nullptr, ETeleportType::ResetPhysics);
			}
		}

		// If we're resetting, we want the camera to be faded out until the game instance fades us back in
		if (GameInstance->IsResettingLevel())
		{
			APlayerController* PlayerController = Player->GetController<APlayerController>();
			ASSERT_RETURN(PlayerController != nullptr);

			PlayerController->PlayerCameraManager->SetManualCameraFade(1, FLinearColor::Black, true);
		}

		// Listen for death
		UHealthComponent* HealthComponent = Player->GetHealth();
		ASSERT_RETURN(HealthComponent != nullptr);

		HealthComponent->OnActorDied().AddDynamic(this, &ARCGameMode::OnPlayerDied);
	}
	else
	{
		ASSERT(PlayerState != nullptr, "Player doesn't have our player state");
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

// Save the persistent data for a level transition
bool ARCGameMode::SaveLevelTransitionDataToMemory(TArray<uint8>* Data)
{
	if (CurrentSave == nullptr)
	{
		LOG_CHECK(CurrentSave != nullptr, LogSave, Error, "No current save. This should've been created when the level was loaded");
		CurrentSave = Cast<URCLevelTransitionSave>(UGameplayStatics::CreateSaveGameObject(URCLevelTransitionSave::StaticClass()));
	}
	ASSERT_RETURN_VALUE(CurrentSave != nullptr, false);

	ASSERT_RETURN_VALUE(GameState != nullptr, false);
	ASSERT_RETURN_VALUE(GameState->PlayerArray.Num() == 1, false);

	ARCPlayerState* PlayerState = Cast<ARCPlayerState>(GameState->PlayerArray[0]);
	ASSERT_RETURN_VALUE(PlayerState != nullptr, false);

	// Save off the player's data
	PlayerState->SaveForLevelTransition(CurrentSave);

	// Iterate the entire world of actors
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor->IsPendingKill())
		{
			continue;
		}		

		// Only interested in saveable actors
		if (!Actor->GetClass()->ImplementsInterface(USaveGameInterface::StaticClass()))
		{
			continue;
		}

		// Does this actor need to be saved
		if (!ISaveGameInterface::Execute_ActorNeedsSaving(Actor))
		{
			continue;
		}

		CurrentSave->SaveActor(Actor);
	}

	// Put savegame into data array
	UGameplayStatics::SaveGameToMemory(CurrentSave, *Data);
	UGameplayStatics::SaveGameToSlot(CurrentSave, "Test", 0);
	return true;
}

// Save the given actor to the current save
void ARCGameMode::SaveActorForLevelTransition(AActor* Actor)
{
	if (Actor->IsPendingKill())
	{
		return;
	}

	// Only interested in saveable actors
	if (!Actor->GetClass()->ImplementsInterface(USaveGameInterface::StaticClass()))
	{
		return;
	}

	ASSERT_RETURN(CurrentSave != nullptr);
	CurrentSave->SaveActor(Actor);
}

// Create a new save data for the current level
void ARCGameMode::InitializeLevelTransitionData()
{
	CurrentSave = Cast<URCLevelTransitionSave>(UGameplayStatics::CreateSaveGameObject(URCLevelTransitionSave::StaticClass()));
	ASSERT(CurrentSave != nullptr);
}

void ARCGameMode::LoadLevelTransitionData()
{
	if (UGameplayStatics::DoesSaveGameExist("Test", 0))
	{
		CurrentSave = Cast<URCLevelTransitionSave>(UGameplayStatics::LoadGameFromSlot("Test", 0));
		LOG_RETURN(CurrentSave != nullptr, LogSave, Error, "Failed to load SaveGame Data.");
		
		for (FActorSaveData ActorSaveData : CurrentSave->SavedActors)
		{
			// Find the actor with this name
			AActor* FoundActor = nullptr;
			for (FActorIterator It(GetWorld()); It; ++It)
			{
				AActor* Actor = *It;
				if (Actor != nullptr && Actor->GetFName() == ActorSaveData.ActorName)
				{
					FoundActor = Actor;
					break;
				}
			}
			LOG_CONTINUE(FoundActor != nullptr, LogSave, Error, "Unable to find actor for %s", *ActorSaveData.ActorName.ToString());

			FMemoryReader MemReader(ActorSaveData.ByteData);

			FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
			Ar.ArIsSaveGame = true;
			// Convert binary array back into actor's variables
			FoundActor->Serialize(Ar);

			//ISGameplayInterface::Execute_OnActorLoaded(Actor);
		}
	}
	else
	{
		InitializeLevelTransitionData();
	}
}

// Called when the player has died
void ARCGameMode::OnPlayerDied(AActor * Player)
{
	UWorld* World = GetWorld();
	ASSERT_RETURN(World != nullptr);

	FTimerHandle ResetTimer;
	World->GetTimerManager().SetTimer(ResetTimer, [&]()
		{
			URCGameInstance* GameInstance = GetGameInstance<URCGameInstance>();
			GameInstance->ResetLevel();
		},
		1, false);
}
