// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"
#include "Enforcer.generated.h"

/**
 *  A shotgun weapon with multiple barrels
 *  - Full Trigger: Shoot all barrels in quick succession
 *  - Half Trigger: Shoot one barrel
 *  - No Trigger: Reload the barrels
 */
UCLASS(Abstract, Blueprintable)
class RC_API AEnforcer : public ABasePlayerWeapon
{
	GENERATED_BODY()

public:
	AEnforcer();

	// Called each frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Determine if the weapon can attack now
	bool CanAttack() const override;

	// Perform an attack with the weapon
	void PerformAttack() override;

	// Called when the trigger status updates
	void OnTriggerStatusUpdated(ETriggerStatus PreviousStatus) override;

	// Perform an action when the trigger is held halfway
	bool PerformHalfTrigger() override;

	// Called when the cooldown has ended
	void CooldownEnded() override;

private:
	// Maybe reload the weapon
	void MaybeReload();

	// The base time it takes to reload the weapon
	// Probably replaced with animations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float ReloadTimeBase = 0.5f;

	// The added time it takes to reload the weapon
	// Probably replaced with animations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float ReloadTimePerBarrel = 0.5f;

	// The amount of barrels that can be loaded
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float MaxBarrelsLoaded = 2;

	// Amount of barrels loaded to fire
	int BarrelsLoaded;

	// Timer to keep track of reloading the barrels
	FTimeStamp ReloadTimer;
};
