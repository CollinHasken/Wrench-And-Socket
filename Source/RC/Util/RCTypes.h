// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RCTypes.generated.h"


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
 * Damage request
 */
USTRUCT(BlueprintType)
struct FDamageRequestParams
{
	GENERATED_BODY()

	// Whether this was caused by the player 
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	bool bFromPlayer = false;

	// Damage to deal 
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	int Damage = 0;

	// Who hit us 
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	TWeakObjectPtr<class ABaseCharacter> Instigator = nullptr;

	// Class of what actually caused the damage
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	UClass* WeaponClass = nullptr;

	// The location the damage happened
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	FVector HitLocation = FVector::ZeroVector;
};

/**
 * Damage reeceived
 */
USTRUCT(BlueprintType)
struct FDamageReceivedParams
{
	GENERATED_BODY()

	FDamageReceivedParams() = default;
	FDamageReceivedParams(const FDamageRequestParams& RequestParams) : WeaponClass(RequestParams.WeaponClass) {}

	// Damage that was dealt
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	int DamageDealt = 0;

	// Class of what actually caused the damage
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	UClass* WeaponClass = nullptr;
};

/**
 * Data for the bullet
 */
struct FBulletData
{
	// Damage to deal
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	int Damage = 0;

	// Direction to go in
	UPROPERTY(BlueprintReadWrite, Category = Speed)
	FVector Direction = FVector::ZeroVector;

	// Who shot us
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	TWeakObjectPtr<class ABaseCharacter> Shooter = nullptr;

	// What shot us
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	TWeakObjectPtr<class ABaseWeapon> Weapon = nullptr;
};

/**
 * Weapon save data
 */
USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

public:
	void GrantDamageXP(float XP);

	// The current ammo for the weapon
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
	int CurrentAmmo = 150;

	// The max ammo, including upgrades
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	int MaxAmmo = 150;

	// The xp to the next level
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
	float CurrentXP = 0;

	// The xp needed to get to the next level
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	int XPTotalForNextLevel = 10;

	// The current level
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	uint8 CurrentLevelIndex = 0;

	// The currently loaded weapon
	TWeakObjectPtr<class ABasePlayerWeapon> CurrentWeapon = nullptr;
};
