// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"
#include "RC/Util/TimeStamp.h"

#include "BurstPistol.generated.h"

/**
 *  A basic pistol
 *  - Full Trigger: Increase fire rate over time and lose accuracy, like a gatling gun
 *  - Half Trigger: Precision semi-auto shots
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABurstPistol : public ABasePlayerWeapon
{
	GENERATED_BODY()

public:
	ABurstPistol();

	// Called each frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the trigger status updates
	void OnTriggerStatusUpdated() override;

	// Perform an action when the trigger is held halfway
	bool PerformHalfTrigger() override;

	// The curve for the cooldown applied while the trigger is held
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* RapidCooldownCurve = nullptr;

	// Time stamp of when the trigger started being held
	FTimeStamp TriggerTimeStamp;
};
