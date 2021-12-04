// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseEnemy.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Kismet/GameplayStatics.h"

#include "RC/AI/SplineFollowerComponent.h"
#include "RC/Characters/Components/HealthComponent.h"
#include "RC/Debug/Debug.h"
#include "RC/Framework/RCGameMode.h"
#include "RC/Weapons/Weapons/EnemyWeapons/BaseEnemyWeapon.h"

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

	// If we load dead, destroy ourselves
	if (GetHealth() != nullptr && GetHealth()->IsDead())
	{
		Destroy();
		return;
	}

	SetupWeapon();
}

// Called before the component is destroyed
void ABaseEnemy::EndPlay(const EEndPlayReason::Type)
{
	// Destroy weapon with us
	if (Weapon != nullptr)
	{
		Weapon->Destroy();
	}
}

ABaseWeapon* ABaseEnemy::GetEquippedWeapon() const
{
	return Weapon;
}

// Attack the player
void ABaseEnemy::AttackPlayer()
{
	LOG_RETURN(Weapon != nullptr, LogAI, Error, "Enemy %s doesn't have weapon", *GetName());
	
	Weapon->AttackPlayer();
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

	// Save that this enemy has died
	ARCGameMode* GameMode = Cast<ARCGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode != nullptr)
	{
		GameMode->SaveActorForLevelTransition(this);
	}
	else
	{
		ASSERT(GameMode != nullptr);
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