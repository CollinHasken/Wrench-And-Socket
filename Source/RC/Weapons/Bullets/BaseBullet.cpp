// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseBullet.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Framework/DamageInterface.h"
#include "RC/Weapons/Bullets/BaseBulletHitEffect.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"
#include "RC/Util/DataSingleton.h"
#include "RC/Util/RCStatics.h"

ABaseBullet::ABaseBullet()
{
 	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	Collision->InitSphereRadius(5.0f);
	Collision->bTraceComplexOnMove = true;
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionProfileName(COLLISION_PRESET_PLAYERBULLET);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	Mesh->SetupAttachment(RootComponent);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	Movement->UpdatedComponent = Collision;
	Movement->InitialSpeed = 10000.0f;
	Movement->MaxSpeed = 10000.0f;
	Movement->bRotationFollowsVelocity = true;
	Movement->ProjectileGravityScale = 0.f;
}

//  Initial setup
void ABaseBullet::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Movement->OnProjectileStop.AddDynamic(this, &ABaseBullet::OnImpact);
	Collision->MoveIgnoreActors.Add(GetInstigator());
	Collision->MoveIgnoreActors.Add(GetOwner());
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	/*
	AShooterWeapon_Projectile* OwnerWeapon = Cast<AShooterWeapon_Projectile>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}

	SetLifeSpan(WeaponConfig.ProjectileLife);
	MyController = GetInstigatorController();
	*/
}

// Setup weapon properties
void ABaseBullet::Init(const FBulletData& BulletData)
{
	Damage = BulletData.Damage;
	DamageType = BulletData.DamageType;
	Shooter = BulletData.Shooter;
	Weapon = BulletData.Weapon;
	ABaseWeapon* WeaponObj = Weapon.Get();
	if (WeaponObj != nullptr)
	{
		WeaponId = WeaponObj->GetInfoId();
	}

	ASSERT_RETURN(Movement != nullptr);
	Movement->Velocity = BulletData.Direction * Movement->InitialSpeed;
}

// Handle hit
void ABaseBullet::OnImpact(const FHitResult& HitResult)
{
	Movement->StopMovementImmediately();

	const TWeakObjectPtr<AActor> HitActor = HitResult.Actor;
	if (HitActor != nullptr)
	{
		IDamageInterface* DamageableActor = Cast<IDamageInterface>(HitActor);
		if (DamageableActor != nullptr)
		{
			// Request damage on hit
			FDamageRequestParams DamageParams;
			DamageParams.bFromPlayer = URCStatics::IsActorPlayer(Shooter.Get());
			DamageParams.Damage = Damage;
			DamageParams.DamageType = DamageType;
			DamageParams.Instigator = Shooter;
			DamageParams.CauseId = WeaponId;
			DamageParams.HitLocation = HitResult.ImpactPoint;
			DamageParams.HitNormal = HitResult.Normal;
			DamageableActor->RequestDamage(DamageParams);
		}
	}

	// Spawn effect
	if (HitEffectClass != nullptr)
	{
		UWorld* World = GetWorld();
		ASSERT_RETURN(World != nullptr);

		FTransform EffectTransform(FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).Rotator(), HitResult.ImpactPoint);
		World->SpawnActor<ABaseBulletHitEffect>(HitEffectClass, EffectTransform);
	}

	Destroy();
}
