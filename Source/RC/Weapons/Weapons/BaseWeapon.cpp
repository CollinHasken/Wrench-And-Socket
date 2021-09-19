// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseWeapon.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "RC/Weapons/Bullets/BaseBullet.h"
#include "RC/Characters/BaseCharacter.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCTypes.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Empty Root"));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

// After properties have been loaded
void ABaseWeapon::PostInitProperties()
{
	Super::PostInitProperties();

	// Save off socket
	if (Mesh != nullptr) 
	{
		BulletOffsetSocket = Mesh->GetSocketByName(GetSocketName());
	} 
	else 
	{
		ASSERT(Mesh != nullptr, "Mesh wasn't created?")
	}
}

// Apply the weapon config
void ABaseWeapon::ApplyConfig(const FWeaponConfig& Config)
{
	WeaponConfig = Config;
}

// Set the new wielder
void ABaseWeapon::SetWielder(ABaseCharacter* NewWielder)
{
	Wielder = NewWielder;

	// Attach weapon to socket
	if (!SocketName.IsNone())
	{
		USkeletalMeshComponent* WielderMesh = Wielder->FindComponentByClass<USkeletalMeshComponent>();
		if (WielderMesh)
		{
			FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
			AttachmentRules.bWeldSimulatedBodies = true;
			ASSERT_RETURN(GetMesh() != nullptr);
			GetMesh()->AttachToComponent(WielderMesh, AttachmentRules, SocketName);
		}
	}
}

// Returns whether the weapon can start shooting
bool ABaseWeapon::CanStartShooting()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	// Still coolingdown
	if (CooldownTimer.IsActive())
	{
		return false;
	}

	// Already have a shot queueing up
	if (TimerManager.IsTimerActive(ShootTimer))
	{
		return false;
	}

	return true;
}

// Returns whether the weapon can be shot now
bool ABaseWeapon::CanShoot()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	// Still coolingdown
	if (CooldownTimer.IsActive())
	{
		return false;
	}

	return true;
}

// Shoot the weapon
bool ABaseWeapon::Shoot()
{	
	CooldownTimer.Set(WeaponConfig.CooldownDelay);
	return true;
}

// Shoot the weapon at the specified target
bool ABaseWeapon::ShootAtTarget(const FVector& TargetLocation)
{
	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	// Spawn the bullet at the offset
	const FTransform& BulletTransform = BulletOffsetSocket->GetSocketTransform(Mesh);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Wielder;
	SpawnParams.bNoFail = true;
	ABaseBullet* Bullet = World->SpawnActor<ABaseBullet>(WeaponConfig.BulletClass, BulletTransform, SpawnParams);
	ASSERT_RETURN_VALUE(Bullet != nullptr, false);

	FVector Trajectory = TargetLocation - BulletTransform.GetLocation();
	Trajectory.Normalize();

	// Initialize bullet to send it off
	FBulletData BulletData;
	BulletData.Damage = WeaponConfig.BaseDamage;
	BulletData.Direction = Trajectory;
	BulletData.Shooter = GetWielder();
	BulletData.Weapon = this;

	Bullet->Init(BulletData);
	return true;
}

// Shoot the weapon at the specified target
bool ABaseWeapon::ShootAtTarget(ABaseCharacter* Target)
{
	// Get the middle of the target's bounding box
	// Maybe change it to looking for a plug in the future
	ASSERT_RETURN_VALUE(Target != nullptr, false);

	UCapsuleComponent* TargetCapsule = Target->GetCapsuleComponent();
	FVector TargetLocation;
	if (TargetCapsule != nullptr)
	{
		TargetLocation = TargetCapsule->GetComponentLocation();
	}
	else
	{
		UE_LOG(LogAI, Error, TEXT("Target %s doesn't have capsule component"), *GetName());
		FBox Bounds = Target->GetComponentsBoundingBox(false, true);
		TargetLocation = Bounds.GetCenter();
	}
	return ShootAtTarget(TargetLocation);
}

// Called when the shot cooldown has expired
void ABaseWeapon::CooldownExpired()
{
}
