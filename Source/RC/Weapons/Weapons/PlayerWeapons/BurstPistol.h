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
	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called when the trigger status updates
	void OnTriggerStatusUpdated(ETriggerStatus PreviousStatus) override;

	// Perform an action when the trigger is held halfway
	bool PerformHalfTrigger() override;

	// Get the value on the curve give the timer
	float GetValueOnCurve(const UCurveFloat* Curve, const FTimeStamp& TimeStamp, const float MaxTime);

	// The curve for the cooldown applied while the trigger is held
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* RapidCooldownCurve = nullptr;

	// The curve for the accuracy while the trigger is held
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* RapidAccuracyCurve = nullptr;

	// Time stamp of when the trigger started being held for cooldown changes
	FTimeStamp CooldownTimeStamp;

	// Time stamp of when the trigger started being held for accuracy changes
	FTimeStamp AccuracyTimeStamp;

	// The max cooldown time
	float MaxCooldownTime = 0;

	// The max accuracy time
	float MaxAccuracyTime = 0;
};
