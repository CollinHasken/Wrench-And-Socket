// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponProjectileComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCTypes.h"
#include "RC/Weapons/Bullets/BaseBullet.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"

void UWeaponProjectileComponent::Init(const UWeaponInfo& InWeaponInfo)
{
	Super::Init(InWeaponInfo);

	Accuracy = InWeaponInfo.BaseAccuracy;

	// Save off socket
	const ABaseWeapon* Owner = GetOwner<ABaseWeapon>();
	ASSERT_RETURN(Owner != nullptr, "Weapon component placed on %s. It needs to inherit from ABaseWeapon", *GetOwner()->GetName());

	WeaponMesh = Owner->GetMesh();
	if (WeaponMesh != nullptr)
	{
		const FName& SocketName = Owner->GetSocketName();
		if (!SocketName.IsNone())
		{
			BulletOffsetSocket = WeaponMesh->GetSocketByName(SocketName);
		}
	}
}

// Attack with the weapon
bool UWeaponProjectileComponent::Attack()
{
	ASSERT_RETURN_VALUE(GetBulletOffsetSocket() != nullptr, false);
	ASSERT_RETURN_VALUE(Wielder != nullptr, false);

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, false);

	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	// Get the target
	FVector Target;
	if (WielderCamera != nullptr)
	{
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("Gun Trace")), true, Owner);
		RV_TraceParams.bReturnPhysicalMaterial = false;
		RV_TraceParams.AddIgnoredActor(static_cast<AActor*>(Wielder));

		FHitResult Hit(EForceInit::ForceInit);
		FVector Start = WielderCamera->GetComponentLocation();
		FVector End = Start + (WielderCamera->GetComponentRotation().Vector() * GetRange());
		World->LineTraceSingleByChannel(Hit, Start, End, ECC_Camera, RV_TraceParams);

		Target = Hit.bBlockingHit ? Hit.ImpactPoint : End;
	}
	else
	{
		Target = Owner->GetActorLocation() + Owner->GetActorForwardVector();
	}

	return ShootAtTarget(Target);
}

// Attack with the weapon at the given target
bool UWeaponProjectileComponent::AttackTarget(ABaseCharacter* Target)
{
	return ShootAtTarget(Target);
}

// Shoot the weapon at the specified target
bool UWeaponProjectileComponent::ShootAtTarget(const FVector& TargetLocation)
{
	ASSERT_RETURN_VALUE(GetBulletOffsetSocket() != nullptr, false);

	ABaseWeapon* Weapon = GetOwner<ABaseWeapon>();
	ASSERT_RETURN_VALUE(Weapon != nullptr, false, "Weapon component placed on %s. It needs to inherit from ABaseWeapon", *GetOwner()->GetName());

	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	// Spawn the bullet at the offset
	const FTransform& BulletTransform = GetBulletOffsetSocket()->GetSocketTransform(WeaponMesh);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Weapon;
	SpawnParams.Instigator = static_cast<APawn*>(Wielder);
	SpawnParams.bNoFail = true;
	ABaseBullet* Bullet = World->SpawnActor<ABaseBullet>(ProjectileClass, BulletTransform, SpawnParams);
	ASSERT_RETURN_VALUE(Bullet != nullptr, false);

	FVector Trajectory = TargetLocation - BulletTransform.GetLocation();
	Trajectory.Normalize();

	// Accuracy offset
	if (!FMath::IsNearlyEqual(Accuracy, 1))
	{
		Trajectory = FMath::VRandCone(Trajectory, FMath::DegreesToRadians(25 * (1 - Accuracy)));
	}

	// Initialize bullet to send it off
	FBulletData BulletData(*WeaponInfo);
	BulletData.Damage = GetDamage();
	BulletData.Direction = Trajectory;
	BulletData.Shooter = Wielder;
	BulletData.Weapon = Weapon;

	Bullet->Init(BulletData);
	return true;
}

// Shoot the weapon at the specified target
bool UWeaponProjectileComponent::ShootAtTarget(ABaseCharacter* Target)
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
		UE_LOG(LogAI, Error, TEXT("Target %s doesn't have capsule component"), *Target->GetName());
		FBox Bounds = Target->GetComponentsBoundingBox(false, true);
		TargetLocation = Bounds.GetCenter();
	}
	return ShootAtTarget(TargetLocation);
}
