// Fill out your copyright notice in the Description page of Project Settings.
#include "BurstPistol.h"

#include "Curves/CurveFloat.h"

#include "RC/Debug/Debug.h"
#include "RC/Weapons/Weapons/Components/WeaponProjectileComponent.h"

ABurstPistol::ABurstPistol()
{
	WeaponComponent = CreateDefaultSubobject<UWeaponProjectileComponent>(TEXT("Weapon"));
}

// Called every frame
void ABurstPistol::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update cooldown
	if (TriggerTimeStamp.IsActive())
	{
		// At the max cooldown
		if (TriggerTimeStamp.Elapsed())
		{
			TriggerTimeStamp.Invalidate();
			PrimaryActorTick.SetTickFunctionEnable(false);
		}
		else
		{
			// Set the cooldown to the point on the curve
			CurrentCooldown = RapidCooldownCurve->GetFloatValue(TriggerTimeStamp.GetTimeSince());
		}
	}
}

// Called when the trigger status updates
void ABurstPistol::OnTriggerStatusUpdated()
{
	switch (CurrentTriggerStatus)
	{
		case ETriggerStatus::NONE:
		case ETriggerStatus::HALF:
			// Reset the cooldown to the weapon's info
			CurrentCooldown = WeaponInfo->Cooldown;
			TriggerTimeStamp.Invalidate();
			break;
		case ETriggerStatus::FULL:
			if (RapidCooldownCurve != nullptr)
			{
				// Set time stamp for the length of the cooldown curve
				float MinTime = 0, MaxTime = 0;
				RapidCooldownCurve->GetTimeRange(MinTime, MaxTime);
				TriggerTimeStamp.Set(MaxTime);

				CurrentCooldown = RapidCooldownCurve->GetFloatValue(0);
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
