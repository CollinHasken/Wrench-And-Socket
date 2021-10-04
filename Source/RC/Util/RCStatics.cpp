// Fill out your copyright notice in the Description page of Project Settings.

#include "RCStatics.h"

#include "Engine/AssetManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Characters/BaseCharacter.h"

// Is the actor the player
bool URCStatics::IsActorPlayer(const AActor* Actor)
{
	return Actor != nullptr && Actor == static_cast<AActor*>(UGameplayStatics::GetPlayerCharacter(Actor, 0));
}

// Is the actor a human
bool URCStatics::IsActorHuman(const AActor* Actor)
{
	return Actor != nullptr && Cast<ABaseCharacter>(Actor) != nullptr;
}

const UObject* URCStatics::GetPrimaryAssetObject(const FPrimaryAssetId& AssetId)
{
	UAssetManager* Manager = UAssetManager::GetIfValid();	
	return Manager != nullptr ? Manager->GetPrimaryAssetObject(AssetId) : nullptr;
}

TSharedPtr<FStreamableHandle> URCStatics::LoadPrimaryAsset(const FPrimaryAssetId& AssetId, FStreamableDelegate DelegateToCall)
{
	UAssetManager* Manager = UAssetManager::GetIfValid();
	return Manager != nullptr ? Manager->LoadPrimaryAsset(AssetId, TArray<FName>(), MoveTemp(DelegateToCall)) : nullptr;
}
