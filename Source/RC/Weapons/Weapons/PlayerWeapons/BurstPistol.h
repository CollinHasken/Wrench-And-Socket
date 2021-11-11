// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"
#include "RC/Util/TimeStamp.h"

#include "BurstPistol.generated.h"

/**
 * 
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
	UPROPERTY(Category = Mesh, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* RapidCooldownCurve = nullptr;

	// Time stamp of when the trigger started being held
	FTimeStamp TriggerTimeStamp;
};
