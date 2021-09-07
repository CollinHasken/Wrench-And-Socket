// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseEnemy.h"

#include "RC/AI/SplineFollowerComponent.h"
#include "RC/Debug/Debug.h"
#include "RC/Weapons/Weapons/BaseEnemyWeapon.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplineFollower = CreateDefaultSubobject<USplineFollowerComponent>(TEXT("Spline Follower"));
}

/** Attack the player */
void ABaseEnemy::AttackPlayer()
{
	LOG_RETURN(Weapon != nullptr, LogAI, Error, "Enemy %s doesn't have weapon", *GetName());
	
	Weapon->ShootAtPlayer();
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	SetupWeapon();
}

void ABaseEnemy::EndPlay(const EEndPlayReason::Type)
{
	Weapon->Destroy();
}

/** Spawn and setup the weapon */
void ABaseEnemy::SetupWeapon()
{
	UClass* WeaponClassObj = WeaponClass.Get();
	LOG_RETURN(WeaponClassObj != nullptr, LogAI, Warning, "Enemy %s doesn't have weapon class set", *GetName());

	UWorld* World = GetWorld();
	ASSERT_RETURN(World != nullptr);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*GetName().Append(TEXT("Weapon")));
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Cast<APawn>(this);
	SpawnParams.bNoFail = true;

	FTransform SpawnTransform = FTransform::Identity;

	Weapon = World->SpawnActor<ABaseEnemyWeapon>(WeaponClassObj, SpawnTransform, SpawnParams);
	ASSERT_RETURN(Weapon != nullptr);

	Weapon->SetWielder(this);
}