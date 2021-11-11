// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerWeapon.h"

#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCStatics.h"
#include "RC/Util/RCTypes.h"
#include "RC/Weapons/Bullets/BaseBullet.h"
#include "RC/Weapons/Weapons/Components/WeaponComponent.h"

// After properties have been loaded
void ABasePlayerWeapon::PostInitProperties()
{
	Super::PostInitProperties();

	PlayerWeaponInfo = Cast<UPlayerWeaponInfo>(WeaponInfo);
	ASSERT(PlayerWeaponInfo != nullptr || HasAnyFlags(RF_ClassDefaultObject) || GetWorld() == nullptr || (GetWorld()->WorldType != EWorldType::Game && GetWorld()->WorldType != EWorldType::PIE));
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

// Get the weapon data
const UPlayerWeaponData* ABasePlayerWeapon::GetWeaponData()
{ 
	if (PlayerWeaponData == nullptr)
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		ASSERT(Player != nullptr, "No current player, unable to get weapon data");

		ARCPlayerState* PlayerState = URCStatics::GetPlayerState(GetWorld());
		ASSERT_RETURN_VALUE(PlayerState != nullptr, nullptr, "Player doesn't have player state");

		PlayerWeaponData = PlayerState->FindOrAddDataForAsset<UPlayerWeaponData>(PlayerWeaponInfo->GetPrimaryAssetId());
		ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, nullptr, "Player doesn't have player state");
	}
	return PlayerWeaponData;
}

// Get the weapon data
const UPlayerWeaponData* ABasePlayerWeapon::GetWeaponData(ESucceedState& Success)
{
	const UPlayerWeaponData* Data = GetWeaponData();
	Success = Data != nullptr ? ESucceedState::Succeeded : ESucceedState::Failed;
	return Data;
}

// Set the new wielder
void ABasePlayerWeapon::SetWielder(ARCCharacter* NewWielder)
{
	if (NewWielder == nullptr)
	{
		return;
	}

	Super::SetWielder(static_cast<ABaseCharacter*>(NewWielder));

	// Setup weapon data
	ARCPlayerState* PlayerState = NewWielder->GetPlayerState<ARCPlayerState>();
	ASSERT_RETURN(PlayerState != nullptr, "Player doesn't have player state");

	ASSERT_RETURN(PlayerWeaponInfo != nullptr);
	PlayerWeaponData = PlayerState->FindOrAddDataForAsset<UPlayerWeaponData>(PlayerWeaponInfo->GetPrimaryAssetId());
	ASSERT_RETURN(PlayerWeaponData != nullptr, "Weapon Data not able to be added");

	PlayerWeaponData->CurrentWeapon = this;
	RecomputeDamage();
}

// Called each frame
void ABasePlayerWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Try to perform the trigger action again if the cooldown has expired
	if (CooldownTimer.Elapsed())
	{
		CooldownTimer.Invalidate();
		if (PlayerWeaponInfo != nullptr && PlayerWeaponInfo->bContinuousAttack)
		{
			PerformTriggerAction();
		}
	}
}

// Get the current ammo 
int ABasePlayerWeapon::GetCurrentAmmo() const
{
	ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, 0);
	return PlayerWeaponData->CurrentAmmo;
}

// Get the max ammo
int ABasePlayerWeapon::GetMaxAmmo() const
{
	ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, 0);
	return PlayerWeaponData->MaxAmmo;
}

// Update the trigger status
void ABasePlayerWeapon::UpdateTriggerStatus(const ETriggerStatus& NewTriggerStatus)
{
	if (NewTriggerStatus == CurrentTriggerStatus)
	{
		return;
	}

	CurrentTriggerStatus = NewTriggerStatus;

	OnTriggerStatusUpdated();

	PerformTriggerAction();
}

// Determine if the weapon can attack now
bool ABasePlayerWeapon::CanAttack() const
{
	if (PlayerWeaponInfo == nullptr || PlayerWeaponData == nullptr)
	{
		return false;
	}

	if (PlayerWeaponInfo->bHasProjectile)
	{
		if (GetCurrentAmmo() <= 0)
		{
			return false;
		}
	}

	return Super::CanAttack();
}

// Perform the action dependent on the current trigger status
bool ABasePlayerWeapon::PerformTriggerAction()
{
	switch (CurrentTriggerStatus)
	{
		case ETriggerStatus::NONE:
			return true;
		case ETriggerStatus::HALF:
			return PerformHalfTrigger();
		case ETriggerStatus::FULL:
			return PerformFullTrigger();
		default:
			ASSERT_RETURN_VALUE(false, false);
	}
}

// Perform an action when the trigger is held fully
bool ABasePlayerWeapon::PerformFullTrigger() 
{ 
	bool bSucceeded = Attack();
	if (bSucceeded)
	{
		if (PlayerWeaponInfo->bHasProjectile)
		{
			PlayerWeaponData->CurrentAmmo -= 1;
		}

		PlayerAttackDelegate.Broadcast(this, ETriggerStatus::FULL);
	}
	return bSucceeded;
}

// Perform an action when the trigger is held halfway
bool ABasePlayerWeapon::PerformHalfTrigger()
{
	PlayerAttackDelegate.Broadcast(this, ETriggerStatus::HALF);
	return true;
}

// Get the total XP needed to get to the next level
int ABasePlayerWeapon::GetXPTotalForNextLevel() const
{
	ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, -1);
	return PlayerWeaponData->XPTotalForNextLevel;
};

// Get the weapon level configs
void ABasePlayerWeapon::GetLevelConfigs(const FWeaponLevelInfo OutLevelConfigs[UPlayerWeaponInfo::MAX_LEVELS]) const
{
	ASSERT_RETURN(PlayerWeaponInfo != nullptr);
	OutLevelConfigs = PlayerWeaponInfo->WeaponLevelConfigs;
}

// Determine and cache the current amount of damage the weapon should do
void ABasePlayerWeapon::RecomputeDamage()
{
	const FWeaponLevelInfo* LevelInfo = GetCurrentLevelData();
	ASSERT_RETURN(LevelInfo != nullptr);

	WeaponComponent->SetDamage(LevelInfo->BaseDamage);
}

// Get current level data
const FWeaponLevelInfo* ABasePlayerWeapon::GetCurrentLevelData() const
{
	ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, nullptr);
	ASSERT_RETURN_VALUE(PlayerWeaponInfo != nullptr, nullptr);
	ASSERT_RETURN_VALUE(PlayerWeaponData->CurrentLevelIndex < UPlayerWeaponInfo::MAX_LEVELS, nullptr);
	return &(PlayerWeaponInfo->WeaponLevelConfigs[PlayerWeaponData->CurrentLevelIndex]);
}

// Get the current XP
float ABasePlayerWeapon::GetCurrentXP() const
{
	ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, -1);
	return PlayerWeaponData->CurrentXP;
};

// Get the current level
uint8 ABasePlayerWeapon::GetCurrentLevel() const
{
	ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, 0);
	return PlayerWeaponData->CurrentLevelIndex + 1;
};

// Can the weapon level up
bool ABasePlayerWeapon::CanLevelUp() const
{
	ASSERT_RETURN_VALUE(PlayerWeaponData != nullptr, false);
	if (GetCurrentLevel() >= UPlayerWeaponInfo::MAX_LEVELS)
	{
		return false;
	}

	return PlayerWeaponData->CurrentXP >= GetXPTotalForNextLevel();
}

// Called when XP has been applied to the weapon data
void ABasePlayerWeapon::OnXPGained(float XP)
{
	XPGainedDelegate.Broadcast(this, PlayerWeaponData->CurrentXP, GetXPTotalForNextLevel());

	if (CanLevelUp())
	{
		LevelUp();
	}
}

// Level up the weapon
void ABasePlayerWeapon::LevelUp()
{
	ASSERT_RETURN(PlayerWeaponInfo != nullptr);
	ASSERT_RETURN(PlayerWeaponData != nullptr);

	// Increment level
	PlayerWeaponData->CurrentLevelIndex += 1;

	const FWeaponLevelInfo* LevelInfo = GetCurrentLevelData();
	ASSERT_RETURN(LevelInfo != nullptr);

	// Remove XP that was required and maybe level up again
	PlayerWeaponData->CurrentXP -= LevelInfo->XPNeeded;
	PlayerWeaponData->CurrentXP = FMath::Max(PlayerWeaponData->CurrentXP, 0.0f);
	PlayerWeaponData->XPTotalForNextLevel = GetCurrentLevel() >= UPlayerWeaponInfo::MAX_LEVELS ? 0 : PlayerWeaponInfo->WeaponLevelConfigs[GetCurrentLevel()].XPNeeded;

	// Update damage
	RecomputeDamage();

	// Execute delegate
	LevelUpDelegate.Broadcast(this, GetCurrentLevel());

	if (CanLevelUp())
	{
		LevelUp();
	}
}
