// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"
#include "Wrench.generated.h"

/**
 * 
 */
UCLASS()
class RC_API AWrench : public ABasePlayerWeapon
{
	GENERATED_BODY()
	
public:
	AWrench();

	// Returns Inventory subobject
	FORCEINLINE class UCapsuleComponent* GetHitTrigger() const { return HitTrigger; }

protected:
	// Initialize the weapon component
	void InitWeaponComponent() override;

	// Perform an attack with the weapon
	void PerformAttack() override;

	/**
	 * Called when the attack has ended
	 *
	 * @param bInterrupted	Whether the attack was interrupted
	 */
	void AttackEnded(bool bInterrupted) override;

private:
	// Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* HitTrigger;
};
