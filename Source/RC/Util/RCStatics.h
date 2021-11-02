// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/StreamableManager.h"
#include "RC/Util/RCTypes.h"
#include "RC/Weapons/RCWeaponTypes.h"
#include "RCStatics.generated.h"

/**
 * Common Statics
 */
UCLASS()
class RC_API URCStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Convert from milliseconds to seconds
	 * @param Milliseconds	Milliseconds to convert
	 * @Return The converted seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static float MillisecondsToSeconds(int Milliseconds) { return Milliseconds / 1000.f; }

	/**
	 * Convert from seconds to milliseconds
	 * @param Seconds	Seconds to convert
	 * @Return The converted milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static int SecondsToMilliseconds(float Milliseconds) { return static_cast<int>(Milliseconds * 1000); }

	/**
	 * Get the trigger status given a value from 0 - 1.0
	 * @param Value	The value to convert
	 * @Return The trigger status
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	static ETriggerStatus TriggerValueToStatus(float Value) { return Value < URCStatics::TriggerStatusHalfMin ? ETriggerStatus::NONE : (Value < URCStatics::TriggerStatusFullMin ? ETriggerStatus::HALF : ETriggerStatus::FULL); }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	static bool IsInventorySlotForPrimaryWeapon(const EInventorySlot Slot) { return static_cast<uint8>(Slot) >= static_cast<uint8>(EInventorySlot::Slot1) && static_cast<uint8>(Slot) <= static_cast<uint8>(EInventorySlot::Slot8); }

	/**
	 * Is the actor the player
	 * @param Actor	The actor to check
	 * @Return True if the actor is the player
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsActorPlayer(const AActor* Actor);

	/**
	 * Is the actor a human
	 * @param Actor	The actor to check
	 * @Return True if the actor is a human
	 */
	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsActorHuman(const AActor* Actor);

	/** Returns the player state or Null if it can't be retrieved*/
	UFUNCTION(BlueprintPure, Category = "Game", meta = (WorldContext = "WorldContextObject"))
	static class ARCPlayerState* GetPlayerState(const UObject* WorldContextObject);

	/**
	 * Get the primary asset
	 * @param AssetId The ID of the asset to retrieve
	 * @Return The primary asset
	 */
	UFUNCTION(BlueprintPure, Category = "Data")
	static const UObject* GetPrimaryAssetObject(const FPrimaryAssetId& AssetId);

	template<typename AssetClass>
	static const AssetClass* GetPrimaryAssetObject(const FPrimaryAssetId& AssetId);

	static TSharedPtr<FStreamableHandle> LoadPrimaryAsset(const FPrimaryAssetId& AssetId, FStreamableDelegate DelegateToCall);

	// Trigger values
	const static float TriggerStatusHalfMin;
	const static float TriggerStatusFullMin;

	// Physics profiles
	static const FName BlockAllButPlayer_ProfileName;
	static const FName Collectible_ProfileName;
	static const FName CollectiblePre_ProfileName;
	static const FName CollectibleTrigger_ProfileName;
	static const FName OverlapOnlyActor_ProfileName;
};

#if CPP
#include "RCStatics.inl"
#endif
