// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "RCSaveGame.generated.h"

/**
 * Base data to save for an actor
 */
USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

public:
	// Identifier for which Actor this belongs to
	UPROPERTY()
	FSoftObjectPath ActorPath;

	// Contains all SaveGame marked variables of the Actor
	UPROPERTY()
	TArray<uint8> ByteData;
};

/**
 * Data for the player
 */
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
 * Save game containing data of actors
 */
UCLASS()
class RC_API URCSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Save an actor's data
	 * @param Actor	The actor to save
	 */
	void SaveActor(class AActor* Actor);

	// Array of saved actors
	UPROPERTY()
	TArray<FActorSaveData> SavedActors;
};

/**
 * Save game for a level transition
 * Mostly just the player's data
 */
UCLASS()
class RC_API URCLevelTransitionSave : public URCSaveGame
{
	GENERATED_BODY()

public:
	/**
	 * Save the player's data
	 * @param Player The player to save
	 */
	void SavePlayer(class ARCCharacter* Player);

	/**
	 * Load the player's data
	 * @param Player The player to load
	 */
	void LoadPlayer(class ARCCharacter* Player) const;

	// Player's save data
	UPROPERTY()
	FPlayerSaveData SavedPlayer;
};

/**
 * Archive marked as save game
 */
struct FSaveGameArchive : public FObjectAndNameAsStringProxyArchive 
{
	FSaveGameArchive(FArchive& InInnerArchive, bool bInLoadIfFindFails) : FObjectAndNameAsStringProxyArchive(InInnerArchive, bInLoadIfFindFails)
	{ 
		ArIsSaveGame = true;		
		ArNoDelta = true; 
	} 
};
