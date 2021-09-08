// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Debug/Debug.h"
#include "RC/Characters/Components/HealthComponent.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Util/RCStatics.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Health->OnActorDied().AddDynamic(this, &ABaseCharacter::OnActorDied);
}

void ABaseCharacter::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaveGame())
	{
		if (Health != nullptr)
		{
			Ar << *Health;
		}
	}
}

void ABaseCharacter::RequestDamage(FDamageRequestParams& Params)
{
	ASSERT_RETURN(Health != nullptr);

	// Determine damage mods (crit, armor, etc)
	float DamageToDeal = Params.Damage;

	Params.Damage = DamageToDeal;
	float DamageDealt = Health->ApplyDamage(Params);

	ABaseCharacter* DamageInstigator = Params.Instigator.Get();
	if (DamageInstigator != nullptr)
	{
		FDamageReceivedParams ReceivedParams(Params);
		ReceivedParams.DamageDealt = DamageDealt;

		DamageInstigator->OnDamageGiven(ReceivedParams);
	}	
}

void ABaseCharacter::OnActorDied(AActor* Actor)
{
	// Disable all collision on capsule
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule != nullptr)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	else
	{
		LOG_CHECK(Capsule != nullptr, LogActor, Error, "Character %s doesn't have a capsule component", *GetName());
	}

	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (SkeletalMesh != nullptr)
	{
		SkeletalMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	}
	else
	{
		LOG_CHECK(SkeletalMesh != nullptr, LogActor, Error, "Character %s doesn't have a mesh component", *GetName());
	}

	SetActorEnableCollision(true);

	if (!bIsRagdolling)
	{
		// Ragdoll
		if (SkeletalMesh != nullptr)
		{
			SkeletalMesh->SetAllBodiesSimulatePhysics(true);
			SkeletalMesh->SetSimulatePhysics(true);
			SkeletalMesh->WakeAllRigidBodies();
			SkeletalMesh->bBlendPhysics = true;
		}

		UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(GetMovementComponent());
		if (Movement != nullptr)
		{
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
			Movement->SetComponentTickEnabled(false);
		}
		else
		{
			LOG_CHECK(Movement != nullptr, LogActor, Error, "Character %s doesn't have a movment component", *GetName());
		}

		bIsRagdolling = true;
	}
}
