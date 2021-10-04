// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerWeapon.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Curves/CurveFloat.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCTypes.h"
#include "RC/Weapons/Bullets/BaseBullet.h"

// After properties have been loaded
void ABasePlayerWeapon::PostInitProperties()
{
	Super::PostInitProperties();

	PlayerWeaponInfo = Cast<UPlayerWeaponInfo>(WeaponInfo);
	UWorld* a = GetWorld();
	auto b = a != nullptr ? a->WorldType : EWorldType::None;
	ASSERT(PlayerWeaponInfo != nullptr || HasAnyFlags(RF_ClassDefaultObject) || GetWorld() == nullptr || (GetWorld()->WorldType != EWorldType::Game && GetWorld()->WorldType != EWorldType::PIE));
}

// Set the new wielder
void ABasePlayerWeapon::SetWielder(ARCCharacter* NewWielder, UCameraComponent* NewWielderCamera)
{
	Super::SetWielder(static_cast<ABaseCharacter*>(NewWielder));

	WielderCamera = NewWielderCamera;

	// Setup weapon data
	ARCPlayerState* PlayerState = NewWielder->GetPlayerState<ARCPlayerState>();
	ASSERT_RETURN(PlayerState != nullptr, "Player doesn't have player state");

	ASSERT_RETURN(PlayerWeaponInfo != nullptr);
	WeaponData = PlayerState->FindOrAddDataForAsset<FWeaponData>(PlayerWeaponInfo->GetPrimaryAssetId());
	ASSERT_RETURN(WeaponData != nullptr, "Weapon Data not able to be added");

	WeaponData->CurrentWeapon = this;
	RecomputeDamage();
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
		TimerManager.SetTimer(ShootTimer, this, &ABasePlayerWeapon::MaybeShoot, WeaponInfo->StartFiringDelay, true, 0);
	}
}

// Check if we can shoot
bool ABasePlayerWeapon::CanShoot()
{
	// See if we have ammo
	if (GetCurrentAmmo() == 0)
	{
		return false;
	}

	return Super::CanShoot();
}

// Shoot the weapon
bool ABasePlayerWeapon::Shoot()
{
	bool bSuccess = CurrentTriggerStatus == ETriggerStatus::FULL ? ShootFull() : ShootHalf();
	// Start cooldown if it was successful
	if (bSuccess)
	{
		ASSERT_RETURN_VALUE(PlayerWeaponInfo != nullptr, false);
		CooldownTimer.Set(PlayerWeaponInfo->CooldownDelay);
	}
	return bSuccess;
}

// Get the current ammo 
int ABasePlayerWeapon::GetCurrentAmmo()
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, 0);
	return WeaponData->CurrentAmmo;
}

// Get the max ammo
int ABasePlayerWeapon::GetMaxAmmo()
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, 0);
	return WeaponData->MaxAmmo;
}

// Get the total XP needed to get to the next level
int ABasePlayerWeapon::GetXPTotalForNextLevel() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, -1);
	return WeaponData->XPTotalForNextLevel;
};

// Get the weapon level configs
void ABasePlayerWeapon::GetLevelConfigs(const FWeaponLevelInfo OutLevelConfigs[UPlayerWeaponInfo::MAX_LEVELS]) const
{
	ASSERT_RETURN(PlayerWeaponInfo != nullptr);
	OutLevelConfigs = PlayerWeaponInfo->WeaponLevelConfigs;
}

// Get the player weapon info
const UPlayerWeaponInfo* ABasePlayerWeapon::GetPlayerWeaponInfo() const
{
	// The CDO doesn't have the player weapon info cached
	if (HasAnyFlags(EObjectFlags::RF_ClassDefaultObject))
	{
		return Cast<UPlayerWeaponInfo>(WeaponInfo);
	}
	return PlayerWeaponInfo;
}

// Determine and cache the current amount of damage the weapon should do
void ABasePlayerWeapon::RecomputeDamage()
{
	const FWeaponLevelInfo* LevelInfo = GetCurrentLevelData();
	ASSERT_RETURN(LevelInfo != nullptr);

	CurrentDamage = LevelInfo->BaseDamage;
}

// Maybe perform a shot
void ABasePlayerWeapon::MaybeShoot()
{
	if (!CanShoot())
	{
		return;
	}

	Shoot();
}

// Perform a shot when the trigger is fully held
bool ABasePlayerWeapon::ShootFull()
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, false);
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

	WeaponData->CurrentAmmo -= 1;
	ShotDelegate.Broadcast(this);

	return Super::ShootAtTarget(Target);
}

// Perform a shot when the trigger is held halfway
bool ABasePlayerWeapon::ShootHalf()
{
	return ShootFull();
}

// Get current level data
const FWeaponLevelInfo* ABasePlayerWeapon::GetCurrentLevelData() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, nullptr);
	ASSERT_RETURN_VALUE(PlayerWeaponInfo != nullptr, nullptr);
	ASSERT_RETURN_VALUE(WeaponData->CurrentLevelIndex < UPlayerWeaponInfo::MAX_LEVELS, nullptr);
	return &(PlayerWeaponInfo->WeaponLevelConfigs[WeaponData->CurrentLevelIndex]);
}

// Get the current XP
float ABasePlayerWeapon::GetCurrentXP() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, -1);
	return WeaponData->CurrentXP;
};

// Get the current level
uint8 ABasePlayerWeapon::GetCurrentLevel() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, 0);
	return WeaponData->CurrentLevelIndex + 1;
};

// Can the weapon level up
bool ABasePlayerWeapon::CanLevelUp() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, false);
	if (GetCurrentLevel() >= UPlayerWeaponInfo::MAX_LEVELS)
	{
		return false;
	}

	return WeaponData->CurrentXP >= GetXPTotalForNextLevel();
}

// Called when XP has been applied to the weapon data
void ABasePlayerWeapon::OnXPGained(float XP)
{
	XPGainedDelegate.Broadcast(this, WeaponData->CurrentXP, GetXPTotalForNextLevel());

	if (CanLevelUp())
	{
		LevelUp();
	}
}

// Level up the weapon
void ABasePlayerWeapon::LevelUp()
{
	ASSERT_RETURN(PlayerWeaponInfo != nullptr);
	ASSERT_RETURN(WeaponData != nullptr);

	// Increment level
	WeaponData->CurrentLevelIndex += 1;

	const FWeaponLevelInfo* LevelInfo = GetCurrentLevelData();
	ASSERT_RETURN(LevelInfo != nullptr);

	// Remove XP that was required and maybe level up again
	WeaponData->CurrentXP -= LevelInfo->XPNeeded;
	WeaponData->CurrentXP = FMath::Max(WeaponData->CurrentXP, 0.0f);
	WeaponData->XPTotalForNextLevel = GetCurrentLevel() >= UPlayerWeaponInfo::MAX_LEVELS ? 0 : PlayerWeaponInfo->WeaponLevelConfigs[GetCurrentLevel()].XPNeeded;

	// Update damage
	RecomputeDamage();

	// Execute delegate
	LevelUpDelegate.Broadcast(this, GetCurrentLevel());

	if (CanLevelUp())
	{
		LevelUp();
	}
}
