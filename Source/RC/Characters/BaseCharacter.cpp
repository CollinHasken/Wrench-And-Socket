// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Debug/Debug.h"
#include "RC/Characters/Components/HealthComponent.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Util/RCStatics.h"

ABaseCharacter::ABaseCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Health->OnActorDied().AddDynamic(this, &ABaseCharacter::OnActorDied);
}

// Save character's data
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

// Request for this character to be damaged
void ABaseCharacter::RequestDamage(FDamageRequestParams& Params)
{
	ASSERT_RETURN(Health != nullptr);

	// Determine damage mods (crit, armor, etc)
	float DamageToDeal = Params.Damage;

	Params.Damage = DamageToDeal;
	float DamageDealt = Health->ApplyDamage(Params);

	// Let the instigator know damage was dealt
	ABaseCharacter* DamageInstigator = Params.Instigator.Get();
	if (DamageInstigator != nullptr)
	{
		FDamageReceivedParams ReceivedParams(Params);
		ReceivedParams.DamageDealt = DamageDealt;

		DamageInstigator->OnDamageGiven(ReceivedParams);
	}	
}

// Called when the character dies
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

	// Set skeletal collision
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

		// Stop any movement
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
