// Fill out your copyright notice in the Description page of Project Settings.

#include "RCStatics.h"

#include "Engine/AssetManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"

#include "RC/Debug/Debug.h"

const float URCStatics::TriggerStatusHalfMin = 0.3f;
const float URCStatics::TriggerStatusFullMin = 0.75f;

const FName URCStatics::Trigger_ProfileName = FName(TEXT("Trigger"));
const FName URCStatics::BlockAllButPlayer_ProfileName = FName(TEXT("BlockAllButPlayer"));
const FName URCStatics::Collectible_ProfileName = FName(TEXT("Collectible"));
const FName URCStatics::CollectiblePre_ProfileName = FName(TEXT("CollectiblePre"));
const FName URCStatics::CollectibleTrigger_ProfileName = FName(TEXT("CollectibleTrigger"));
const FName URCStatics::OverlapOnlyActor_ProfileName = FName(TEXT("OverlapOnlyActor"));

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

ARCPlayerState* URCStatics::GetPlayerState(const UObject* WorldContextObject)
{
	AGameStateBase* GameState = UGameplayStatics::GetGameState(WorldContextObject);
	if (GameState == nullptr)
	{
		return nullptr;
	}
	ASSERT_RETURN_VALUE(GameState->PlayerArray.Num() == 1, false);
	return Cast<ARCPlayerState>(GameState->PlayerArray[0]);
}

TSharedPtr<FStreamableHandle> URCStatics::LoadPrimaryAsset(const FPrimaryAssetId& AssetId, FStreamableDelegate DelegateToCall)
{
	UAssetManager* Manager = UAssetManager::GetIfValid();
	return Manager != nullptr ? Manager->LoadPrimaryAsset(AssetId, TArray<FName>(), MoveTemp(DelegateToCall)) : nullptr;
}

// Keep the camera in place
void URCStatics::LockCamera(const UObject* WorldContextObject, bool bLock)
{
	APlayerCameraManager* CamMan = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, 0);
	if (CamMan == nullptr)
	{
		return;
	}

	// Need to set the view target and lock outgoing to keep the camera in place
	CamMan->BlendParams.bLockOutgoing = bLock;
	CamMan->PendingViewTarget.Target = bLock ? static_cast<AActor*>(CamMan) : static_cast<AActor*>(UGameplayStatics::GetPlayerPawn(WorldContextObject, 0));
}
