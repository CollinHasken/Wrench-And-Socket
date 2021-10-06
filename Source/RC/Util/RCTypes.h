// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RCTypes.generated.h"

static const FName NAME_NONE(NAME_None);

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

	// Asset ID of what actually caused the damage
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	FPrimaryAssetId CauseId;

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
	FDamageReceivedParams(const FDamageRequestParams& RequestParams) : CauseId(RequestParams.CauseId) {}

	// Damage that was dealt
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	int DamageDealt = 0;

	// Asset ID of what caused the damage
	UPROPERTY(BlueprintReadWrite, Category = Damage)
	FPrimaryAssetId CauseId = FPrimaryAssetId();
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

USTRUCT()
struct FBaseData
{
	GENERATED_USTRUCT_BODY()

public:
	virtual ~FBaseData() {};

	// Serialize savegame properties
	virtual void Serialize(FArchive& Ar);

	/**
	 * Set the default values the data should have
	 * 
	 * @param Id	The asset Id for the data
	 */
	virtual void SetDefaults(const FPrimaryAssetId& Id) { AssetId = Id; };

	// Get the struct of the data
	UScriptStruct* GetStruct() const { return DataStruct; }

protected:
	UScriptStruct* DataStruct = FBaseData::StaticStruct();
	FPrimaryAssetId AssetId = FPrimaryAssetId();
};

typedef TMap<FPrimaryAssetId, TSharedPtr<FBaseData>> SaveDataMap;
typedef TMap<UScriptStruct*, SaveDataMap> SaveDataStructMap;

FArchive& operator<<(FArchive& Ar, FPrimaryAssetId& SObj);
FArchive& operator<<(FArchive& Ar, SaveDataStructMap& StateData);

/**
 * Weapon save data
 */
USTRUCT(BlueprintType)
struct FWeaponData : public FBaseData
{
	GENERATED_USTRUCT_BODY()

public:
	FWeaponData() { DataStruct = FWeaponData::StaticStruct(); }

	/**
	 * Set the default values the data should have
	 *
	 * @param Id	The asset Id for the data
	 */
	void SetDefaults(const FPrimaryAssetId& Id) override;

	/**
	 * Grant XP
	 * @param XP The amount of XP to grant
	 */
	void GrantDamageXP(float XP);

	/**
	 * SAVE DATA
	 */
	// The current ammo for the weapon
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
	int CurrentAmmo = 150;

	// The xp to the next level
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
	float CurrentXP = 0;

	// The current level
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
	uint8 CurrentLevelIndex = 0;

	/**
	 * NOT SAVE DATA
	 */
	// The xp needed to get to the next level
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	int XPTotalForNextLevel = 10;

	// The max ammo, including upgrades
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	int MaxAmmo = 150;

	// Class of the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	TSubclassOf<class ABasePlayerWeapon> WeaponClass;

	// The currently loaded weapon
	TWeakObjectPtr<class ABasePlayerWeapon> CurrentWeapon = nullptr;
};

/**
 * Collectible save data
 */
USTRUCT(BlueprintType)
struct FCollectibleData : public FBaseData
{
	GENERATED_USTRUCT_BODY()

public:
	FCollectibleData() { DataStruct = FCollectibleData::StaticStruct(); }

	/**
	 * Grant an amount of the collectible
	 * @param Amount The amount of the collectible to grant
	 */
	void GrantCollectible(int Amount);

	// Current amount of the collectible the player has
	UPROPERTY(BlueprintReadOnly, Category = Collectible, SaveGame)
	int CurrentAmount = 0;
};
