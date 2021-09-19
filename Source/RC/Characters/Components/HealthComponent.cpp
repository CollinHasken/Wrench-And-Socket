// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"

#include "Perception/AISense_Damage.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Util/RCStatics.h"

UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Save off component's save data
FArchive& operator<<(FArchive& Ar, UHealthComponent& SObj)
{
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		UClass* Class = UHealthComponent::StaticClass();
		Class->SerializeTaggedProperties(Ar, (uint8*)&SObj, Class, nullptr);
	}
	return Ar;
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;	
}

// Apply damage requested
float UHealthComponent::ApplyDamage(const FDamageRequestParams& DamageParams)
{
	// Already dead
	if (bIsDead)
	{
		return -1;
	}

	// Remove health
	float DamageDealt = FMath::Min(DamageParams.Damage, CurrentHealth);
	CurrentHealth -= DamageParams.Damage;

	// Broadcast damage
	ActorDamagedDelegate.Broadcast(GetOwner(), CurrentHealth);

	if (CurrentHealth <= 0)
	{
		// Ran out of health
		OnKilled();
	} 
	else
	{
		// Let the AI system know this actor was damaged
		AActor* Owner = GetOwner();
		AActor* Instigator = static_cast<AActor*>(DamageParams.Instigator.Get());
		UAISense_Damage::ReportDamageEvent(Owner, Owner, Instigator, DamageParams.Damage, DamageParams.HitLocation, DamageParams.HitLocation);
	}
	return DamageDealt;
}

// Once we've ran out of health
void UHealthComponent::OnKilled()
{
	bIsDead = true;

	ActorDiedDelegate.Broadcast(GetOwner());

	GetOwner()->SetLifeSpan(5);
}
