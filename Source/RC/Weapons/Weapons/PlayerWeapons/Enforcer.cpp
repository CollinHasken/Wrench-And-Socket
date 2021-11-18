// Fill out your copyright notice in the Description page of Project Settings.
#include "Enforcer.h"

#include "RC/Debug/Debug.h"
#include "RC/Weapons/Weapons/Components/WeaponRaycastComponent.h"


AEnforcer::AEnforcer()
{
	WeaponComponent = CreateDefaultSubobject<UWeaponRaycastComponent>(TEXT("Weapon Comp"));
}

void AEnforcer::BeginPlay()
{
	Super::BeginPlay();

	BarrelsLoaded = MaxBarrelsLoaded;
}

// Called every frame
void AEnforcer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set the barrel as reloaded once the timer is done
	if (ReloadTimer.Elapsed())
	{
		ReloadTimer.Invalidate();
		BarrelsLoaded = MaxBarrelsLoaded;
	}		
}

// Determine if the weapon can attack now
bool AEnforcer::CanAttack() const
{
	// If there's a barrel loaded
	if (BarrelsLoaded <= 0)
	{
		return false;
	}

	// If we aren't reloading
	if (ReloadTimer.IsActive())
	{
		return false;
	}

	return Super::CanAttack();
}

// Perform an attack with the weapon
void AEnforcer::PerformAttack()
{
	Super::PerformAttack();

	--BarrelsLoaded;
}

// Called when the trigger status updates
void AEnforcer::OnTriggerStatusUpdated(ETriggerStatus PreviousStatus)
{
	switch (CurrentTriggerStatus)
	{
		case ETriggerStatus::NONE:
			// Reload the weapon if the trigger isn't held
			MaybeReload();
			break;
		case ETriggerStatus::HALF:			
			break;
		case ETriggerStatus::FULL:
			break;
		default:
			break;
	}
}

// Perform an action when the trigger is held halfway
bool AEnforcer::PerformHalfTrigger()
{
	// Just perform a regular shot
	return Super::PerformFullTrigger();
}

// Called when the cooldown has ended
void AEnforcer::CooldownEnded()
{
	Super::CooldownEnded();

	if (BarrelsLoaded > 0)
	{
		// If the trigger is fully held, try to attack again
		if (GetTriggerStatus() == ETriggerStatus::FULL)
		{
			PerformFullTrigger();
		}
	}
	else if (CurrentTriggerStatus == ETriggerStatus::NONE)
	{
		// Reload now if the trigger isn't held
		MaybeReload();
	}
}

// Maybe reload the weapon
void AEnforcer::MaybeReload()
{
	// Reload once the timer's up
	if (BarrelsLoaded != MaxBarrelsLoaded && !ReloadTimer.IsValid())
	{
		// Load longer for more barrels
		ReloadTimer.Set(ReloadTimeBase + (ReloadTimePerBarrel * (MaxBarrelsLoaded - BarrelsLoaded)));
	}
}
