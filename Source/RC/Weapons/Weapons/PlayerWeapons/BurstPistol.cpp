// Fill out your copyright notice in the Description page of Project Settings.
#include "BurstPistol.h"

#include "Curves/CurveFloat.h"

#include "RC/Debug/Debug.h"
#include "RC/Weapons/Weapons/Components/WeaponProjectileComponent.h"

ABurstPistol::ABurstPistol()
{
	WeaponComponent = CreateDefaultSubobject<UWeaponProjectileComponent>(TEXT("Weapon"));
}

// Called when the game starts or when spawned
void ABurstPistol::BeginPlay()
{
	Super::BeginPlay();

	ASSERT_RETURN(RapidCooldownCurve != nullptr);
	ASSERT_RETURN(RapidAccuracyCurve != nullptr);

	// Cache the length of the curves
	float Unused;
	RapidCooldownCurve->GetTimeRange(Unused, MaxCooldownTime);
	RapidAccuracyCurve->GetTimeRange(Unused, MaxAccuracyTime);
}

// Called every frame
void ABurstPistol::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update cooldown
	if (CooldownTimeStamp.IsActive())
	{
		// At the max cooldown
		if (CooldownTimeStamp.Elapsed())
		{
			CooldownTimeStamp.Invalidate();
		}
		else
		{
			// If the trigger is held, then update the cooldown based on the time on the curve
			if (CurrentTriggerStatus == ETriggerStatus::FULL)
			{
				CurrentCooldown = GetValueOnCurve(RapidCooldownCurve, CooldownTimeStamp, MaxCooldownTime);
			}
		}
	}

	// Update accuracy
	if (AccuracyTimeStamp.IsActive())
	{
		// At the max accuracy
		if (AccuracyTimeStamp.Elapsed())
		{
			AccuracyTimeStamp.Invalidate();
		}
		else
		{
			// Update accuracy based on the time on the curve
			UWeaponProjectileComponent* ProjectileWeaponComponent = Cast<UWeaponProjectileComponent>(WeaponComponent);
			ProjectileWeaponComponent->SetAccuracy(GetValueOnCurve(RapidAccuracyCurve, AccuracyTimeStamp, MaxAccuracyTime));
		}
	}
}

// Called when the trigger status updates
void ABurstPistol::OnTriggerStatusUpdated(ETriggerStatus PreviousStatus)
{
	switch (CurrentTriggerStatus)
	{
		case ETriggerStatus::NONE:
		case ETriggerStatus::HALF:
			// Update cooldown and accuracy if the trigger was previously held
			if (PreviousStatus == ETriggerStatus::FULL)
			{
				// Start the cooldown and accuracy going back to max
				// If the timer is still active, then we need to add any extra time that was chopped when setting them before
				CooldownTimeStamp.Set(CooldownTimeStamp.IsActive() ? CooldownTimeStamp.GetTimeSince() + MaxCooldownTime - CooldownTimeStamp.GetTotalDuration() : MaxCooldownTime);
				AccuracyTimeStamp.Set(AccuracyTimeStamp.IsActive() ? AccuracyTimeStamp.GetTimeSince() + MaxAccuracyTime - AccuracyTimeStamp.GetTotalDuration() : MaxAccuracyTime);

				// Set the cooldown back to the default
				CurrentCooldown = WeaponInfo->Cooldown;
			}
			break;
		case ETriggerStatus::FULL:
		{
			// Set time stamps for the curves' length
			// If the timer was already active, then we need to subtract the remaining time on the timer as that's the progress going back to normal
			float Time = CooldownTimeStamp.IsActive() ? MaxCooldownTime - CooldownTimeStamp.GetTimeRemaining() : MaxCooldownTime;
			CooldownTimeStamp.Set(Time);
			Time = AccuracyTimeStamp.IsActive() ? MaxAccuracyTime - AccuracyTimeStamp.GetTimeRemaining() : MaxAccuracyTime;
			AccuracyTimeStamp.Set(Time);

			// Set the cooldown and accuracy to the values on the curves
			CurrentCooldown = GetValueOnCurve(RapidCooldownCurve, CooldownTimeStamp, MaxCooldownTime);

			UWeaponProjectileComponent* ProjectileWeaponComponent = Cast<UWeaponProjectileComponent>(WeaponComponent);
			ProjectileWeaponComponent->SetAccuracy(GetValueOnCurve(RapidAccuracyCurve, AccuracyTimeStamp, MaxAccuracyTime));
		}
			break;
		default:
			break;
	}
}

// Perform an action when the trigger is held halfway
bool ABurstPistol::PerformHalfTrigger()
{
	// Just perform a regular shot
	return PerformFullTrigger();
}

// Get the value on the curve give the timer
float ABurstPistol::GetValueOnCurve(const UCurveFloat* Curve, const FTimeStamp& TimeStamp, const float MaxTime)
{
	float TimeOnCurve = 0;
	if (CurrentTriggerStatus == ETriggerStatus::FULL)
	{
		// Add any extra time that wasn't set for the timer
		TimeOnCurve = TimeStamp.GetTimeSince() + MaxTime - TimeStamp.GetTotalDuration();
	}
	else
	{
		// We're going backwards on the curve so we want the time remaining
		TimeOnCurve = TimeStamp.GetTimeRemaining();
	}
	return Curve->GetFloatValue(TimeOnCurve);
}
