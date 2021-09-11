// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerWeapon.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Curves/CurveFloat.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Weapons/Bullets/BaseBullet.h"

// Set the weapon data that stores things like level and XP
void ABasePlayerWeapon::SetData(FWeaponData& InWeaponData)
{
	WeaponData = &InWeaponData; 
	WeaponData->CurrentWeapon = this;

	WeaponConfig.BaseDamage = GetCurrentLevelData().BaseDamage;
}

// Set the new wielder
void ABasePlayerWeapon::SetWielder(ABaseCharacter* NewWielder, UCameraComponent* NewWielderCamera)
{
	Super::SetWielder(NewWielder);

	WielderCamera = NewWielderCamera;
}

void ABasePlayerWeapon::MaybeShoot()
{
	if (!CanShoot())
	{
		return;
	}

	Shoot();
}

bool ABasePlayerWeapon::Shoot()
{
	bool bSuccess = CurrentTriggerStatus == ETriggerStatus::FULL ? ShootFull() : ShootHalf();
	// Start cooldown if it was successful
	if (bSuccess)
	{
		CooldownTimer.Set(WeaponConfig.CooldownDelay);
	}
	return bSuccess;
}

// Update the trigger status
void ABasePlayerWeapon::UpdateTriggerStatus(const ETriggerStatus& NewTriggerStatus)
{
	CurrentTriggerStatus = NewTriggerStatus;

	FTimerManager& TimerManager = GetWorldTimerManager();
	if (CurrentTriggerStatus == ETriggerStatus::NONE)
	{
		TimerManager.ClearTimer(ShootTimer);
	}
	else if (CanStartShooting())
	{
		TimerManager.SetTimer(ShootTimer, this, &ABasePlayerWeapon::MaybeShoot, WeaponConfig.StartFiringDelay, true, 0);
	}
}

void ABasePlayerWeapon::GrantDamageXP(float XP)
{
	ASSERT_RETURN(WeaponData != nullptr);
	WeaponData->CurrentXP += XP;

	XPGainedDelegate.Broadcast(this, WeaponData->CurrentXP, GetXPTotalForNextLevel());

	if (CanLevelUp())
	{
		LevelUp();
	}
}

bool ABasePlayerWeapon::ShootFull()
{
	ASSERT_RETURN_VALUE(BulletOffsetSocket != nullptr, false);
	ASSERT_RETURN_VALUE(Wielder != nullptr, false);

	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	auto BulletTransform = BulletOffsetSocket->GetSocketTransform(Mesh);

	// Get the target
	FVector Target;
	if (WielderCamera != nullptr)
	{
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("Gun Trace")), true, this);
		RV_TraceParams.bReturnPhysicalMaterial = false;
		RV_TraceParams.AddIgnoredActor(static_cast<AActor*>(Wielder));

		FHitResult Hit(EForceInit::ForceInit);
		FVector Start = WielderCamera->GetComponentLocation();
		FVector End = Start + (WielderCamera->GetComponentRotation().Vector() * 4000);
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Camera, RV_TraceParams);

		Target = Hit.bBlockingHit ? Hit.ImpactPoint : End;
	}
	else
	{
		Target = GetActorLocation() + GetActorForwardVector();
	}

	return Super::ShootAtTarget(Target);
}

bool ABasePlayerWeapon::ShootHalf()
{
	return ShootFull();
}

const FWeaponLevelInfo& ABasePlayerWeapon::GetCurrentLevelData() const
{
	//ASSERT_RETURN_VALUE(WeaponData != nullptr, FWeaponLevelInfo());
	ASSERT_RETURN_VALUE(WeaponData->CurrentLevelIndex < MAX_LEVELS, WeaponLevelConfigs[WeaponData->CurrentLevelIndex - 1]);
	return WeaponLevelConfigs[WeaponData->CurrentLevelIndex];
}

/** Get the current level */
uint8 ABasePlayerWeapon::GetCurrentLevel() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, -1);
	return WeaponData->CurrentLevelIndex + 1;
};

/** Get the current XP */
float ABasePlayerWeapon::GetCurrentXP() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, -1);
	return WeaponData->CurrentXP;
};

/** Get the total XP needed to get to the next level */
float ABasePlayerWeapon::GetXPTotalForNextLevel() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, -1);
	return GetCurrentLevel() >= MAX_LEVELS ? 0 : WeaponLevelConfigs[GetCurrentLevel()].XPNeeded;
};

bool ABasePlayerWeapon::CanLevelUp() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, false);
	if (GetCurrentLevel() >= MAX_LEVELS)
	{
		return false;
	}

	return WeaponData->CurrentXP >= GetXPTotalForNextLevel();
}

void ABasePlayerWeapon::LevelUp()
{
	ASSERT_RETURN(WeaponData != nullptr);

	// Increment level
	WeaponData->CurrentLevelIndex += 1;

	// Remove XP that was required and maybe level up again
	WeaponData->CurrentXP -= GetCurrentLevelData().XPNeeded;
	WeaponData->CurrentXP = FMath::Max(WeaponData->CurrentXP, 0.0f);

	// Update damage
	WeaponConfig.BaseDamage = GetCurrentLevelData().BaseDamage;

	// Execute delegate
	LevelUpDelegate.Broadcast(this, GetCurrentLevel());

	if (CanLevelUp())
	{
		LevelUp();
	}
}
