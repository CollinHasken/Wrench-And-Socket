// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerWeapon.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Curves/CurveFloat.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/DataSingleton.h"
#include "RC/Weapons/Bullets/BaseBullet.h"


// Called when the game starts or when spawned
void ABasePlayerWeapon::BeginPlay()
{
	Super::BeginPlay();

	ASSERT_RETURN(GEngine != nullptr);
	UDataSingleton* Singleton = Cast<UDataSingleton>(GEngine->GameSingleton);
	LevelDilationCurve = Singleton->LevelDilationCurve;
}

// Called every frame
void ABasePlayerWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LevelUpTimer.IsActive())
	{
		ASSERT_RETURN(LevelDilationCurve != nullptr);
		float Dilation = LevelDilationCurve->GetFloatValue(LevelUpTimer.GetTimeSince());

		GetWorld()->GetWorldSettings()->SetTimeDilation(Dilation);
	}
	else if (LevelUpTimer.IsValid())
	{
		// Time has expired, invalidate it
		LevelUpTimer.Invalidate();
	}
}

// Set the new wielder
void ABasePlayerWeapon::SetWielder(ABaseCharacter* NewWielder, UCameraComponent* NewWielderCamera)
{
	Wielder = NewWielder;
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

	// Get the trajectory
	FVector Trajectory;
	if (WielderCamera != nullptr)
	{
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("Gun Trace")), true, this);
		RV_TraceParams.bReturnPhysicalMaterial = false;
		RV_TraceParams.AddIgnoredActor(static_cast<AActor*>(Wielder));

		FHitResult Hit(EForceInit::ForceInit);
		FVector Start = WielderCamera->GetComponentLocation();
		FVector End = Start + (WielderCamera->GetComponentRotation().Vector() * 4000);
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Camera, RV_TraceParams);

		if (Hit.bBlockingHit)
		{
			Trajectory = Hit.ImpactPoint - BulletTransform.GetLocation();
		}
		else
		{
			Trajectory = End - BulletTransform.GetLocation();
		}

		Trajectory.Normalize();
	}
	else
	{
		Trajectory = GetActorForwardVector();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Wielder;
	SpawnParams.bNoFail = true;
	ABaseBullet* Bullet = World->SpawnActor<ABaseBullet>(WeaponConfig.BulletClass, BulletTransform, SpawnParams);
	ASSERT_RETURN_VALUE(Bullet != nullptr, false);

	FBulletData BulletData;
	BulletData.Damage = 10;
	BulletData.Direction = Trajectory;
	BulletData.Shooter = GetWielder();
	BulletData.Weapon = this;

	Bullet->Init(BulletData);

	return true;
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

/** Get the total XP needed to get to the next level */
float ABasePlayerWeapon::GetXPTotalForNextLevel() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, -1);
	return WeaponData->CurrentLevelIndex + 1 >= MAX_LEVELS ? 0 : WeaponLevelConfigs[WeaponData->CurrentLevelIndex + 1].XPNeeded;
};


bool ABasePlayerWeapon::CanLevelUp() const
{
	ASSERT_RETURN_VALUE(WeaponData != nullptr, false);
	if (WeaponData->CurrentLevelIndex + 1 >= MAX_LEVELS)
	{
		return false;
	}

	return WeaponData->CurrentXP >= GetXPTotalForNextLevel();
}

void ABasePlayerWeapon::LevelUp()
{
	ASSERT_RETURN(WeaponData != nullptr);
	// Slow-mo
	if (LevelDilationCurve != nullptr)
	{
		float MinTime = 0, MaxTime = 0;
		LevelDilationCurve->GetTimeRange(MinTime, MaxTime);

		LevelUpTimer.Set(MaxTime);
	}

	// Increment level
	WeaponData->CurrentLevelIndex += 1;

	// Remove XP that was required and maybe level up again
	WeaponData->CurrentXP -= GetCurrentLevelData().XPNeeded;
	if (CanLevelUp())
	{
		LevelUp();
	}
}
