// Fill out your copyright notice in the Description page of Project Settings.
#include "DangerZone.h"

#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Framework/DamageInterface.h"
#include "RC/Util/RCStatics.h"

// Sets default values
ADangerZone::ADangerZone()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create the trigger
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetCollisionProfileName(URCStatics::Trigger_ProfileName);
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ADangerZone::OnDangerZoneEntered);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &ADangerZone::OnDangerZoneExited);
	RootComponent = Trigger;
}

// Called when the game starts or when spawned
void ADangerZone::BeginPlay()
{
	Super::BeginPlay();
	
	// Don't tick when nothing's inside
	ActorsInside.Empty();
	SetActorTickEnabled(false);
}

// Called every frame
void ADangerZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Go through each actor inside
	FDamageRequestParams Params;
	Params.Damage = DamageEachTick;
	Params.DamageType = EDamageTypes::NORMAL;
	for (AActor* Actor : ActorsInside)
	{
		// Deal damage if we do each tick
		if (bDealsDamageEachTick)
		{
			IDamageInterface* DamageableActor = Cast<IDamageInterface>(Actor);
			ASSERT_CONTINUE(DamageableActor != nullptr);

			DamageableActor->RequestDamage(Params);
		}

		// Apply impulse if we do each tick
		if (bApplyImpulseEachTick)
		{
			ApplyImpulse(Actor);
		}
	}
}

// On an actor entering the danger zone trigger
void ADangerZone::OnDangerZoneEntered(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	// Only want to consider actors that can be damaged
	IDamageInterface* DamageableActor = Cast<IDamageInterface>(OtherActor);
	if (DamageableActor == nullptr)
	{
		return;
	}

	// Don't do anything if they're already dead
	if (DamageableActor->IsDead())
	{
		return;
	}

	ActorsInside.Add(OtherActor);

	// Deal damage
	if (bDealsDamageOnEnter)
	{
		FDamageRequestParams Params;
		Params.Damage = DamageOnEnter;
		Params.DamageType = EDamageTypes::NORMAL;
		DamageableActor->RequestDamage(Params);
	}

	// Stop their movement
	if (bStopMovementOnEnter)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
		if (Character != nullptr)
		{
			UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
			ASSERT_RETURN(Movement != nullptr);
			Movement->StopMovementImmediately();
		}
	}

	// Apply impulse
	if (bApplyImpulse)
	{
		ApplyImpulse(OtherActor);
	}

	// Check if we need to set ticking
	if (!IsActorTickEnabled() && ShouldTick())
	{
		SetActorTickEnabled(true);
	}
}

// On an actor exiting the danger zone trigger
void ADangerZone::OnDangerZoneExited(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	// Remove actor
	ActorsInside.RemoveSingleSwap(OtherActor);

	// Check if we need to stop ticking
	if (IsActorTickEnabled() && !ShouldTick())
	{
		SetActorTickEnabled(false);
	}
}

// Apply the impulse to the given actor
void ADangerZone::ApplyImpulse(AActor* Actor) const
{
	FVector Impulse;	
	if (ImpulseType == EDangerZoneImpulseTypes::Center)
	{
		// If we want from the center, simply get the direction from our center to the actor
		Impulse = Actor->GetActorLocation() - GetActorLocation();
		Impulse.Normalize();
	}

	ABaseCharacter* Character = Cast<ABaseCharacter>(Actor);
	if (Character != nullptr)
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		ASSERT_RETURN(Movement != nullptr);

		if (ImpulseType == EDangerZoneImpulseTypes::BoostUp)
		{
			Impulse = Movement->GetLastUpdateVelocity();
			Impulse = Impulse.GetSafeNormal2D();
			Impulse.Z = 2;
		}
		Impulse *= ImpulseMagnitude;
		Movement->AddImpulse(Impulse, true);
		return;
	}

	UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
	if (Component != nullptr)
	{
		if (ImpulseType == EDangerZoneImpulseTypes::BoostUp)
		{
			Impulse = Component->GetComponentVelocity();
			Impulse = Impulse.GetSafeNormal2D();
			Impulse.Z = 2;
		}
		Impulse *= ImpulseMagnitude;
		Component->AddImpulse(Impulse, NAME_None, true);
		return;
	}
}

// Should this danger zone tick
bool ADangerZone::ShouldTick() const
{
	// No actors to tick for
	if (ActorsInside.Num() == 0)
	{
		return false;
	}

	// Need to tick to deal damage
	if (bDealsDamageEachTick)
	{
		return true;
	}

	// Need to tick to apply impulse
	if (bApplyImpulseEachTick)
	{
		return true;
	}

	return false;
}
