// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "RCSaveGame.generated.h"

USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

public:

	/* Identifier for which Actor this belongs to */
	UPROPERTY()
		FName ActorName;

	/* For movable Actors, keep location,rotation,scale. */
	UPROPERTY()
		FTransform Transform;

	/* Contains all 'SaveGame' marked variables of the Actor */
	UPROPERTY()
		TArray<uint8> ByteData;
};

USTRUCT()
struct FPlayerSaveData : public FActorSaveData
{
	GENERATED_BODY()

public:
	/* Contains all 'SaveGame' marked variables of the Player State */
	UPROPERTY()
	TArray<uint8> PlayerStateByteData;
};

/**
 * 
 */
UCLASS()
class RC_API URCSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FActorSaveData> SavedActors;

	void SaveActor(class AActor* Actor);
};

/**
 *
 */
UCLASS()
class RC_API URCLevelTransitionSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FPlayerSaveData SavedPlayer;

	void SavePlayer(class ARCCharacter* Player);
	void LoadPlayer(class ARCCharacter* Player) const;
};

struct FSaveGameArchive : public FObjectAndNameAsStringProxyArchive 
{
	FSaveGameArchive(FArchive& InInnerArchive, bool bInLoadIfFindFails) : FObjectAndNameAsStringProxyArchive(InInnerArchive, bInLoadIfFindFails)
	{ 
		ArIsSaveGame = true;		
		ArNoDelta = true; 
	} 
};
