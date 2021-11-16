// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/Util/RCDataTypes.h"

#include "RCWeaponTypes.generated.h"


/**
 * Status of the gun's trigger
 */
UENUM(BlueprintType, Category = "Weapon")
enum class ETriggerStatus : uint8
{
	NONE UMETA(DisplayName = "No Trigger"),
	HALF UMETA(DisplayName = "Half Pulled Trigger"),
	FULL UMETA(DisplayName = "Full Pulled Trigger"),
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
	int Damage = 0;

	// Direction to go in
	FVector Direction = FVector::ZeroVector;

	// Who shot us
	TWeakObjectPtr<class ABaseCharacter> Shooter = nullptr;

	// What shot us
	TWeakObjectPtr<class ABaseWeapon> Weapon = nullptr;
};

/**
 * Config for every weapon
 */
UCLASS(BlueprintType)
class RC_API UWeaponInfo : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// The class of the weapon this info is for
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<class ABaseWeapon> WeaponClass;

	// The montage to play when an attack is performed on the wielder
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	class UAnimMontage* WielderAttackMontage = nullptr;

	// The montage to play when an attack is performed on the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	class UAnimMontage* WeaponAttackMontage = nullptr;

	// Whether the attack for the weapon component will be called form an anim notify from the attack montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	bool AttackFromMontage = false;

	// The cooldown after each attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	float Cooldown = 0.5f;

	// The base damage to deal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	float BaseDamage = 1;

	// The base range the weapon has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	float BaseRange = 4000;

	// The socket that we'll attach this weapon to
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	FName SocketName = FName();
};

/**
 * Info for each weapon level
 */
USTRUCT(BlueprintType)
struct FWeaponLevelInfo
{
	GENERATED_USTRUCT_BODY()

	// The XP needed to get to this level from the previous
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float XPNeeded = 100.0f;

	// The base damage from this weapon at this level
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float BaseDamage = 10.0f;
};

/**
 * Info for player weapons
 */
UCLASS()
class RC_API UPlayerWeaponInfo : public UWeaponInfo
{
	GENERATED_BODY()

public:
	// Maximum levels
	static constexpr uint8 MAX_LEVELS = 10;

	// Info for each weapon level
	UPROPERTY(EditDefaultsOnly, Category = Level)
	FWeaponLevelInfo WeaponLevelConfigs[MAX_LEVELS];

	// XP granted per hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Level)
	float XPPerHit = 5.0f;

	// The display name for the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	FName DisplayName;

	// The icon for the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	TSoftObjectPtr<class UTexture2D> IconTexture;

	// Whether the weapon continue attacking if the trigger is still held down
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	bool bContinuousAttack = true;

	/**
	 * SHOOTABLES
	 */
	 // Whether the weapon has a projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	bool bHasProjectile = true;

	// The base max ammo for the weapon. Only used for player weapons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, Meta = (EditCondition = "bHasProjectile", EditConditionHides))
	int BaseMaxAmmo = 150;
};

/**
 * Weapon save data
 */
UCLASS(BlueprintType)
class UPlayerWeaponData : public UBaseData
{
	GENERATED_BODY()

public:
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
	 // The xp to the next level
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
		float CurrentXP = 0;

	// The current level
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
		uint8 CurrentLevelIndex = 0;

	// The current ammo for the weapon
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
		int CurrentAmmo = 150;

	// The max ammo, including upgrades
	UPROPERTY(BlueprintReadOnly, Category = Weapon, SaveGame)
		int MaxAmmo = 150;

	/**
	 * NOT SAVE DATA
	 */
	 // The xp needed to get to the next level
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
		int XPTotalForNextLevel = 10;

	// The currently loaded weapon
	TWeakObjectPtr<class ABasePlayerWeapon> CurrentWeapon = nullptr;
};
