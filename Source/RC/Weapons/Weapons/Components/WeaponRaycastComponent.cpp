// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponRaycastComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCStatics.h"
#include "RC/Util/RCTypes.h"
#include "RC/Weapons/Bullets/BaseBullet.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"

// Initialize the weapon component
void UWeaponRaycastComponent::Init(const UWeaponInfo& WeaponInfo)
{
	Super::Init(WeaponInfo);

	// Save off socket
	const ABaseWeapon* Owner = GetOwner<ABaseWeapon>();
	ASSERT_RETURN(Owner != nullptr, "Weapon component placed on %s. It needs to inherit from ABaseWeapon", *GetOwner()->GetName());

	WeaponMesh = Owner->GetMesh();

	if (WeaponMesh != nullptr)
	{
		const FName& SocketName = Owner->GetSocketName();
		if (!SocketName.IsNone())
		{
			VFXOffsetSocket = WeaponMesh->GetSocketByName(SocketName);
		}
	}
}

// Attack with the weapon
bool UWeaponRaycastComponent::Attack()
{
	ASSERT_RETURN_VALUE(GetVFXOffsetSocket() != nullptr, false);
	ASSERT_RETURN_VALUE(Wielder != nullptr, false);

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, false);

	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	// If the camera is aiming at something, shoot towards that
	FVector TargetDirection;
	if (WielderCamera != nullptr)
	{
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("Gun Trace")), true, Owner);
		RV_TraceParams.bReturnPhysicalMaterial = false;
		RV_TraceParams.AddIgnoredActor(static_cast<AActor*>(Wielder));

		FHitResult Hit(EForceInit::ForceInit);
		FVector Start = WielderCamera->GetComponentLocation();
		// The range is for the weapon's range, so we need to add the distance from the camera to the wielder to the cast
		FVector End = Start + WielderCamera->GetComponentRotation().Vector() * (GetRange() + (FVector::Dist(Start, Wielder->GetActorLocation())));
		World->LineTraceSingleByChannel(Hit, Start, End, ECC_Camera, RV_TraceParams);

		// Either go to what we hit or the end of the cast
		TargetDirection = Hit.bBlockingHit ? Hit.ImpactPoint : End;
		TargetDirection -= GetVFXOffsetSocket()->GetSocketTransform(WeaponMesh).GetLocation();
		TargetDirection.Normalize();
	}
	else
	{
		// Just shoot towards the weapon's forward
		TargetDirection = Owner->GetActorForwardVector();
	}

	return ShootTowardsTarget(TargetDirection);
}

// Attack with the weapon at the given target
bool UWeaponRaycastComponent::AttackTarget(ABaseCharacter* Target)
{
	ASSERT_RETURN_VALUE(Target != nullptr, false);

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, false);

	UCapsuleComponent* TargetCapsule = Target->GetCapsuleComponent();
	FVector TargetDirection;
	if (TargetCapsule != nullptr)
	{
		TargetDirection = TargetCapsule->GetComponentLocation() - Owner->GetActorLocation();
	}
	else
	{
		UE_LOG(LogAI, Error, TEXT("Target %s doesn't have capsule component"), *Target->GetName());
		FBox Bounds = Target->GetComponentsBoundingBox(false, true);
		TargetDirection = Bounds.GetCenter() - Owner->GetActorLocation();
	}
	TargetDirection.Normalize();
	return ShootTowardsTarget(TargetDirection);
}

// Shoot the weapon towards the specified target
bool UWeaponRaycastComponent::ShootTowardsTarget(const FVector& TargetDirection)
{
	LOG_CHECK(TargetDirection.IsNormalized(), LogWeapon, Warning, "Target direction isn't normalized");

	ABaseWeapon* Weapon = GetOwner<ABaseWeapon>();
	ASSERT_RETURN_VALUE(Weapon != nullptr, false, "Weapon component placed on %s. It needs to inherit from ABaseWeapon", *GetOwner()->GetName());

	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	FTransform VFXTransform = GetVFXOffsetSocket()->GetSocketTransform(WeaponMesh);
	VFXTransform.SetRotation(TargetDirection.ToOrientationQuat());

	// Find actors in close range
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Gun Trace")), true, Weapon);
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(static_cast<AActor*>(Wielder));

	TArray<FHitResult> Hits;
	FVector Start = VFXTransform.GetLocation();
	FVector End = Start + (TargetDirection * GetRange() * .3f);
	bool bHit = World->SweepMultiByProfile(Hits, Start, End, VFXTransform.GetRotation(), FName("OverlapOnlyActor"), FCollisionShape::MakeBox(CloseTraceHalfSize), TraceParams);

	// Find actors in far range
	TArray<FHitResult> FarHits;
	FVector FarEnd = End + (TargetDirection * GetRange() * .7f);
	bool bFarHit = World->SweepMultiByProfile(FarHits, End, FarEnd, VFXTransform.GetRotation(), FName("OverlapOnlyActor"), FCollisionShape::MakeBox(FarTraceHalfSize), TraceParams);

	// Setup damage params
	FDamageRequestParams DamageParams;
	DamageParams.bFromPlayer = URCStatics::IsActorPlayer(Wielder);
	DamageParams.Damage = GetDamage();
	DamageParams.Instigator = Wielder;
	DamageParams.CauseId = WeaponInfoId;

	FHitResult BlockedHit;
	FCollisionQueryParams BlockedTraceParams = FCollisionQueryParams(FName(TEXT("Block Trace")), true, Weapon);
	BlockedTraceParams.bReturnPhysicalMaterial = false;
	BlockedTraceParams.AddIgnoredActor(static_cast<AActor*>(Wielder));

	TArray<AActor*> HitActors;
	// Damage the unique actors found in the raycasts
	auto DamageHitActors = [&](const TArray<FHitResult>& Hits)
	{
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			// Already damaged this actor
			if (HitActors.Contains(HitActor))
			{
				continue;
			}

			// Make sure we can damage this actor
			IDamageInterface* DamageableActor = Cast<IDamageInterface>(Hit.GetActor());
			if (DamageableActor == nullptr)
			{
				continue;
			}

			// Test if there's something blocking the shot between where we hit them
			World->LineTraceSingleByChannel(BlockedHit, Start, Hit.Location, ECC_Camera, BlockedTraceParams);
			if (BlockedHit.bBlockingHit)
			{
				if (BlockedHit.GetActor() != HitActor)
				{
					continue;
				}
			}

			// Do the damage
			DamageParams.HitLocation = Hit.ImpactPoint;
			DamageableActor->RequestDamage(DamageParams);

			// Store that we damaged this actor
			HitActors.Add(HitActor);
		}
	};

	// Damage actors close and far
	DamageHitActors(Hits);
	DamageHitActors(FarHits);

	// Spawn the VFX at the offset
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Weapon;
	SpawnParams.Instigator = static_cast<APawn*>(Wielder);
	SpawnParams.bNoFail = true;
	World->SpawnActor<AActor>(VFXClass, VFXTransform, SpawnParams);

	return true;
}
