// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"
#include "BasePlayerWeapon.generated.h"

UENUM(BlueprintType, Category = "Weapon")
enum class ETriggerStatus : uint8
{
	NONE UMETA(DisplayName = "No Trigger"),
	HALF UMETA(DisplayName = "Half Pulled Trigger"),
	FULL UMETA(DisplayName = "Full Pulled Trigger"),
};

USTRUCT(BlueprintType)
struct FWeaponLevelInfo
{
	GENERATED_USTRUCT_BODY()

		/** The XP needed to get to this level from the previous*/
		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
		float XPNeeded = 100.0f;

	/** The base damage from this weapon at this level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
		float BaseDamage = 10.0f;
};

/**
 * 
 */
UCLASS()
class RC_API ABasePlayerWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaTime) override;

	void SetData(FWeaponData& InWeaponData) { WeaponData = &InWeaponData; WeaponData->CurrentWeapon = this; }

	/**
	 * Set the new wielder for this weapon
	 * @param NewWielder		The pawn that is now wielding this weapon
	 * @param NewWielderCamera	The camera of the new wielder to be used for aiming
	 */
	void SetWielder(class ABaseCharacter* NewWielder, class UCameraComponent* NewWielderCamera);

	/**
	 * Start shooting
	 */
	UFUNCTION()
		void UpdateTriggerStatus(const ETriggerStatus& NewTriggerStatus);

	UFUNCTION(BlueprintPure, Category = "Weapon")
		static ETriggerStatus TriggerValueToStatus(float Value) { return Value < 0.3f ? ETriggerStatus::NONE : (Value < 0.75f ? ETriggerStatus::HALF : ETriggerStatus::FULL); }

	UFUNCTION(BlueprintPure, Category = "Weapon")
		ETriggerStatus GetTriggerStatus() { return CurrentTriggerStatus; }

	// Shoot the weapon
	virtual bool Shoot() override;

	UFUNCTION(BlueprintPure, Category = "Weapon")
		FWeaponData& GetWeaponData() { return *WeaponData; }

	void GrantDamageXP(float XP);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MaybeShoot();

	/** Perform a shot when the trigger is fully held **/
	bool ShootFull();

	/** Perform a shot when the trigger is held halfway **/
	bool ShootHalf();

	/** Get current level data */
	const FWeaponLevelInfo& GetCurrentLevelData() const;

	bool CanLevelUp() const;

	/** Get the total XP needed to get to the next level */
	float GetXPTotalForNextLevel() const;

	/** Level up the weapon */
	void LevelUp();

	UPROPERTY(EditDefaultsOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
		FWeaponLevelInfo WeaponLevelConfigs[MAX_LEVELS];

	UPROPERTY(EditDefaultsOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
		float XPPerHit = 5.0f;

	/** Whether the weapon is currently shooting */
	UPROPERTY(Category = Weapon, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ETriggerStatus CurrentTriggerStatus = ETriggerStatus::NONE;

	FWeaponData* WeaponData = nullptr;

	// Timer to keep track of level up slowmo
	FTimeStamp LevelUpTimer;

	// The camera of the wielder to use for aiming
	class UCameraComponent* WielderCamera = nullptr;

	class UCurveFloat* LevelDilationCurve = nullptr;
};
