// Fill out your copyright notice in the Description page of Project Settings.
#include "RCSaveGame.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"

#include "RC/Characters/Player/RCCharacter.h"


void URCSaveGame::SaveActor(AActor* Actor)
{
	FActorSaveData ActorData;
	ActorData.ActorName = Actor->GetFName();
	ActorData.Transform = Actor->GetActorTransform();

	// Pass the array to fill with data from Actor
	FMemoryWriter MemWriter(ActorData.ByteData);

	FSaveGameArchive Ar(MemWriter, true);
	// Converts Actor's SaveGame UPROPERTIES into binary array
	Actor->Serialize(Ar);

	SavedActors.Add(ActorData);
}

void URCLevelTransitionSave::SavePlayer(ARCCharacter* Player)
{
	SavedPlayer.ActorName = Player->GetFName();
	SavedPlayer.Transform = Player->GetActorTransform();

	// Pass the array to fill with data from Actor
	FMemoryWriter MemWriter(SavedPlayer.ByteData);
	FSaveGameArchive Ar(MemWriter, true);
	Player->Serialize(Ar);
}

void URCLevelTransitionSave::LoadPlayer(ARCCharacter* Player) const
{
	// Convert binary array back into actor's variables
	FMemoryReader MemReader(SavedPlayer.ByteData);
	FSaveGameArchive Ar(MemReader, true);
	Player->Serialize(Ar);

	//ISaveGameInterface::Execute_OnActorLoaded(Actor);
}
