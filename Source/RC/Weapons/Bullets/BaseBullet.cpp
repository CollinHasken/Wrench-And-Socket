// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseBullet.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "RC/Weapons/Bullets/BaseBulletHitEffect.h"
#include "RC/Characters/BaseCharacter.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"
#include "RC/Util/DataSingleton.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCStatics.h"

// Sets default values
ABaseBullet::ABaseBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	//Mesh->AttachTo(RootComponent, TEXT("NAME_None"), EAttachLocation::SnapToTargetIncludingScale, false);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	Movement->UpdatedComponent = Collision;
	Movement->InitialSpeed = 10000.0f;
	Movement->MaxSpeed = 10000.0f;
	Movement->bRotationFollowsVelocity = true;
	Movement->ProjectileGravityScale = 0.f;
}

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

// Called when the game starts or when spawned
void ABaseBullet::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseBullet::Init(const FBulletData& BulletData)
{
	Damage = BulletData.Damage;
	Shooter = BulletData.Shooter;
	Weapon = BulletData.Weapon;
	ABaseWeapon* WeaponObj = Weapon.Get();
	WeaponClass = WeaponObj != nullptr ? WeaponObj->GetClass() : nullptr;

	ASSERT_RETURN(Movement != nullptr);
	Movement->Velocity = BulletData.Direction * Movement->InitialSpeed;
}

void ABaseBullet::OnImpact(const FHitResult& HitResult)
{
	Movement->StopMovementImmediately();

	const TWeakObjectPtr<AActor> HitActor = HitResult.Actor;
	if (HitActor != nullptr)
	{
		ABaseCharacter* HitBaseCharacter = Cast<ABaseCharacter>(HitActor);
		if (HitBaseCharacter != nullptr)
		{
			FDamageRequestParams DamageParams;
			DamageParams.bFromPlayer = URCStatics::IsActorPlayer(Shooter.Get());
			DamageParams.Damage = Damage;
			DamageParams.Instigator = Shooter;
			DamageParams.WeaponClass = WeaponClass;
			DamageParams.HitLocation = HitResult.ImpactPoint;
			HitBaseCharacter->RequestDamage(DamageParams);
		}
	}

	if (HitEffectClass != nullptr)
	{
		UWorld* World = GetWorld();
		ASSERT_RETURN(World != nullptr);

		FTransform EffectTransform(FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).Rotator(), HitResult.ImpactPoint);
		World->SpawnActor<ABaseBulletHitEffect>(HitEffectClass, EffectTransform);
	}

	//SetLifeSpan(0.5f);
	Destroy();
}

// Called every frame
void ABaseBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

