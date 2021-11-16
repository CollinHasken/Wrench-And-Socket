// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/Weapons/RCWeaponTypes.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"

#include "BasePlayerWeapon.generated.h"

// Broadcasted when the weapon gains xp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponXPGained, class ABasePlayerWeapon*, Weapon, float, CurrentXP, float, RequiredXP);

// Broadcasted when the weapon levels up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponLevelUp, class ABasePlayerWeapon*, Weapon, uint8, CurrentLevel);

// Broadcasted when the player weapon attacks
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerWeaponAttack, class ABasePlayerWeapon*, Weapon, ETriggerStatus, TriggerStatus);


/**
 * The base player weapon
 */
UCLASS(Abstract)
class RC_API ABasePlayerWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	// Get the player weapon info
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UPlayerWeaponInfo* GetPlayerWeaponInfo() const;

	// Get the weapon data
	const UPlayerWeaponData* GetWeaponData();

	/**
	 * Set the new wielder for this weapon
	 * @param NewWielder		The player that is now wielding this weapon
	 */
	void SetWielder(class ARCCharacter* NewWielder);

	/**
	 * Update the trigger status. Possibly start shooting
	 * @param NewTriggerStatus	The status of the trigger to update to
	 */
	void UpdateTriggerStatus(const ETriggerStatus& NewTriggerStatus);

	//Get the current trigger status
	UFUNCTION(BlueprintPure, Category = "Weapon")
	ETriggerStatus GetTriggerStatus() { return CurrentTriggerStatus; }

	// Get the current ammo 
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int GetCurrentAmmo() const;

	// Get the max ammo
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int GetMaxAmmo() const;

	// Get the total XP needed to get to the next level
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int GetXPTotalForNextLevel() const;

	/**
	 * Get the weapon level configs
	 * @param OutLevelConfigs	The level configs
	 */
	void GetLevelConfigs(const FWeaponLevelInfo OutLevelConfigs[UPlayerWeaponInfo::MAX_LEVELS]) const;

	// Get the XP Gained delegate
	FOnWeaponXPGained& OnXPGained() { return XPGainedDelegate; }

	// Get the Level Up delegate
	FOnWeaponLevelUp& OnLevelUp() { return LevelUpDelegate; }

	// Get the Attack delegate
	FOnPlayerWeaponAttack& OnPlayerWeaponAttack() { return PlayerAttackDelegate; }

protected:
	friend UPlayerWeaponData;

	// After properties have been loaded
	void PostInitProperties() override;

	// Determine if the weapon can attack now
	bool CanAttack() const override;

	// Called when the trigger status updates
	virtual void OnTriggerStatusUpdated() {};

	// Perform the action dependent on the current trigger status
	bool PerformTriggerAction();

	// Perform an action when the trigger is held fully
	virtual bool PerformFullTrigger();

	// Perform an action when the trigger is held halfway
	virtual bool PerformHalfTrigger();

	// Called when the cooldown has ended
	void CooldownEnded() override;

	// Determine and cache the current amount of damage the weapon should do
	virtual void RecomputeDamage();

	// Get current level data
	const FWeaponLevelInfo* GetCurrentLevelData() const;

	// Get the weapon data
	UFUNCTION(BlueprintCallable, meta=(ExpandEnumAsExecs = "Success"), Category = "Weapon")
	const UPlayerWeaponData* GetWeaponData(ESucceedState& Success);

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
	UPlayerWeaponData* PlayerWeaponData = nullptr;

	// Player weapon info
	UPlayerWeaponInfo* PlayerWeaponInfo = nullptr;

private:
	// Broadcasted when the weapon gains xp
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponLevelUp LevelUpDelegate;

	// Broadcasted when the weapon levels up
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponXPGained XPGainedDelegate;

	// Broadcasted when the player weapon attacks
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnPlayerWeaponAttack PlayerAttackDelegate;
};
