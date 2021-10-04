// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"
#include "BasePlayerWeapon.generated.h"

// Broadcasted when the weapon gains xp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponXPGained, class ABasePlayerWeapon*, Weapon, float, CurrentXP, float, RequiredXP);

// Broadcasted when the weapon levels up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponLevelUp, class ABasePlayerWeapon*, Weapon, uint8, CurrentLevel);

// Broadcasted when the weapon fires
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponShot, class ABasePlayerWeapon*, Weapon);

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
	UPROPERTY(EditDefaultsOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	FWeaponLevelInfo WeaponLevelConfigs[MAX_LEVELS];

	// XP granted per hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float XPPerHit = 5.0f;

	// The display name for the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FName DisplayName;

	// The icon for the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess))
	TSoftObjectPtr<class UTexture2D> IconTexture;
};


/**
 * The base player weapon
 */
UCLASS(Abstract)
class RC_API ABasePlayerWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	/**
	 * Set the new wielder for this weapon
	 * @param NewWielder		The player that is now wielding this weapon
	 * @param NewWielderCamera	The camera of the new wielder to be used for aiming
	 */
	void SetWielder(class ARCCharacter* NewWielder, class UCameraComponent* NewWielderCamera);

	/**
	 * Update the trigger status. Possibly start shooting
	 * @param NewTriggerStatus	The status of the trigger to update to
	 */
	UFUNCTION()
	void UpdateTriggerStatus(const ETriggerStatus& NewTriggerStatus);

	/**
	 * Get the trigger status given a value from 0 - 1.0
	 * @param Value	The value to convert
	 * @Return The trigger status
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	static ETriggerStatus TriggerValueToStatus(float Value) { return Value < 0.3f ? ETriggerStatus::NONE : (Value < 0.75f ? ETriggerStatus::HALF : ETriggerStatus::FULL); }

	//Get the current trigger status
	UFUNCTION(BlueprintPure, Category = "Weapon")
	ETriggerStatus GetTriggerStatus() { return CurrentTriggerStatus; }

	// Check if we can shoot
	bool CanShoot() override;

	// Shoot the weapon
	virtual bool Shoot() override;

	// Get the current ammo 
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int GetCurrentAmmo();
	
	// Get the max ammo
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int GetMaxAmmo();

	// Get the total XP needed to get to the next level
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int GetXPTotalForNextLevel() const;

	/**
	 * Get the weapon level configs
	 * @param OutLevelConfigs	The level configs
	 */
	void GetLevelConfigs(const FWeaponLevelInfo OutLevelConfigs[UPlayerWeaponInfo::MAX_LEVELS]) const;

	// Get the player weapon info
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	const UPlayerWeaponInfo* GetPlayerWeaponInfo() const;

	// Get the weapon data
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FWeaponData& GetWeaponData() { return *WeaponData; }

	// Get the XP Gained delegate
	FOnWeaponXPGained& OnXPGained() { return XPGainedDelegate; }

	// Get the Level Up delegate
	FOnWeaponLevelUp& OnLevelUp() { return LevelUpDelegate; }

	// Get the Shot delegate
	FOnWeaponShot& OnShot() { return ShotDelegate; }

protected:
	friend FWeaponData;

	// After properties have been loaded
	virtual void PostInitProperties() override;

	// Determine and cache the current amount of damage the weapon should do
	virtual void RecomputeDamage();

	// Maybe perform a shot
	void MaybeShoot();

	// Perform a shot when the trigger is fully held
	bool ShootFull();

	// Perform a shot when the trigger is held halfway
	bool ShootHalf();

	// Get current level data
	const FWeaponLevelInfo* GetCurrentLevelData() const;

	// Get the current XP
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentXP() const;

	// Get the current level
	UFUNCTION(BlueprintPure, Category = "Weapon")
	uint8 GetCurrentLevel() const;

	// Can the weapon level up
	bool CanLevelUp() const;

	/**
	 * Called when XP has been applied to the weapon data
	 * @param XP	The XP that was gained
	 */
	void OnXPGained(float XP);

	// Level up the weapon
	void LevelUp();

	// Whether the weapon is currently shooting
	UPROPERTY(Category = Weapon, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ETriggerStatus CurrentTriggerStatus = ETriggerStatus::NONE;

	// The weapon data
	FWeaponData* WeaponData = nullptr;

	// Player weapon info
	UPlayerWeaponInfo* PlayerWeaponInfo = nullptr;

	// The camera of the wielder to use for aiming
	class UCameraComponent* WielderCamera = nullptr;

private:
	// Broadcasted when the weapon gains xp
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponLevelUp LevelUpDelegate;

	// Broadcasted when the weapon levels up
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponXPGained XPGainedDelegate;

	// Broadcasted when the weapon fires
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponShot ShotDelegate;	
};
