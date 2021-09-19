// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseEnemy.h"

#include "AIController.h"
#include "BrainComponent.h"

#include "RC/AI/SplineFollowerComponent.h"
#include "RC/Debug/Debug.h"
#include "RC/Weapons/Weapons/BaseEnemyWeapon.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	SplineFollower = CreateDefaultSubobject<USplineFollowerComponent>(TEXT("Spline Follower"));
}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	SetupWeapon();
}

// Called before the component is destroyed
void ABaseEnemy::EndPlay(const EEndPlayReason::Type)
{
	// Destroy weapon with us
	Weapon->Destroy();
}

// Attack the player
void ABaseEnemy::AttackPlayer()
{
	LOG_RETURN(Weapon != nullptr, LogAI, Error, "Enemy %s doesn't have weapon", *GetName());
	
	Weapon->ShootAtPlayer();
}

// Called when the character dies
void ABaseEnemy::OnActorDied(AActor* Actor)
{
	Super::OnActorDied(Actor);

	// Stop the AI on death
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		UBrainComponent* Brain = AIController->GetBrainComponent();
		if (Brain)
		{
			Brain->StopLogic("Died");
		}
	}
}

// Spawn and setup the weapon
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