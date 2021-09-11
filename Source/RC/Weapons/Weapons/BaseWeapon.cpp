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

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Empty Root"));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void ABaseWeapon::PostInitProperties()
{
	Super::PostInitProperties();

	if (Mesh != nullptr) 
	{
		BulletOffsetSocket = Mesh->GetSocketByName(GetSocketName());
	} 
	else 
	{
		ASSERT(Mesh != nullptr, "Mesh wasn't created?")
	}
}

void ABaseWeapon::ApplyConfig(const FWeaponConfig& Config)
{
	WeaponConfig = Config;
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
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

bool ABaseWeapon::Shoot()
{
	
	CooldownTimer.Set(WeaponConfig.CooldownDelay);
	return true;
}

bool ABaseWeapon::ShootAtTarget(const FVector& TargetLocation)
{
	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	const FTransform& BulletTransform = BulletOffsetSocket->GetSocketTransform(Mesh);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Wielder;
	SpawnParams.bNoFail = true;
	ABaseBullet* Bullet = World->SpawnActor<ABaseBullet>(WeaponConfig.BulletClass, BulletTransform, SpawnParams);
	ASSERT_RETURN_VALUE(Bullet != nullptr, false);

	FVector Trajectory = TargetLocation - BulletTransform.GetLocation();
	Trajectory.Normalize();

	FBulletData BulletData;
	BulletData.Damage = WeaponConfig.BaseDamage;
	BulletData.Direction = Trajectory;
	BulletData.Shooter = GetWielder();
	BulletData.Weapon = this;

	Bullet->Init(BulletData);
	return true;
}

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

void ABaseWeapon::CooldownExpired()
{

}
