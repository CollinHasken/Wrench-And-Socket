// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/Util/RCDataTypes.h"

#include "RCTypes.generated.h"

static const FName NAME_NONE(NAME_None);

/**
 * Success/Fail enum for branching UFUNCTIONS
 */
UENUM(BlueprintType)
enum class ESucceedState : uint8
{
	Succeeded,
	Failed
};

/**
 * AI states
 */
UENUM(BlueprintType, Category = "AI")
enum class EAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Search		UMETA(DisplayName = "Search"),
	Chase		UMETA(DisplayName = "Chase"),
	Combat		UMETA(DisplayName = "Combat"),

	NUM_STATES	UMETA(Hidden)
};

/**
 * Inventory slots for the player
 */
UENUM(BlueprintType, Category = "Inventory")
enum class EInventorySlot : uint8
{
	Slot1 = 0	UMETA(DisplayName = "Slot 1"),
	Slot2		UMETA(DisplayName = "Slot 2"),
	Slot3		UMETA(DisplayName = "Slot 3"),
	Slot4		UMETA(DisplayName = "Slot 4"),
	Slot5		UMETA(DisplayName = "Slot 5"),
	Slot6		UMETA(DisplayName = "Slot 6"),
	Slot7		UMETA(DisplayName = "Slot 7"),
	Slot8		UMETA(DisplayName = "Slot 8"),
	SlotWrench	UMETA(DisplayName = "Slot Wrench"),

	NUM_SLOTS	UMETA(Hidden)
};

/**
 * Quick slots mapping to inventory slots
 */
UENUM(BlueprintType, Category = "Inventory")
enum class EQuickSlot : uint8
{
	Slot1 = 0	UMETA(DisplayName = "Slot 1"),
	Slot2		UMETA(DisplayName = "Slot 2"),
	Slot3		UMETA(DisplayName = "Slot 3"),
	Slot4		UMETA(DisplayName = "Slot 4"),

	NUM_SLOTS	UMETA(Hidden)
};

/**
 * How to handle adding a status effect stack when there's already a stack
 */
UENUM(BlueprintType, Category = "Status Effect")
enum class EStatusEffectRequestType : uint8
{
	Override	UMETA(DisplayName = "Override Existing"),
	Add			UMETA(DisplayName = "Add To Existing"),
	Ignore		UMETA(DisplayName = "Ignore If Existing"),
	Default		UMETA(DisplayName = "Default")
};


/**
 * Collectible save data
 */
UCLASS(BlueprintType)
class UCollectibleData : public UBaseData
{
	GENERATED_BODY()

public:
	/**
	 * Grant an amount of the collectible
	 * @param Amount The amount of the collectible to grant
	 */
	virtual void GrantCollectible(int Amount);

	// Current amount of the collectible the player has
	UPROPERTY(BlueprintReadOnly, Category = Collectible, SaveGame)
	int CurrentAmount = 0;
};
