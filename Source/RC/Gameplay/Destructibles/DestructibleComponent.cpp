// Fill out your copyright notice in the Description page of Project Settings.
#include "DestructibleComponent.h"

#include "RC/Debug/Debug.h"

// Save the destructible
FArchive& operator<<(FArchive& Ar, UDestructibleComponent& SObj)
{
	if (Ar.IsSaveGame())
	{
		Ar << SObj.bIsDestroyed;
	}
	return Ar;
}

void UDestructibleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsDestroyed())
	{
		HideOwner();
	}
}

// Apply hit to destructible	 
void UDestructibleComponent::ApplyHit()
{
	// Already destroyed
	if (IsDestroyed())
	{
		return;
	}

	++CurrentHits;
	if (CurrentHits >= HitsToDestruction)
	{
		Destroy();
	}
}

// Destroy this destructible
void UDestructibleComponent::Destroy()
{
	AActor* Owner = GetOwner();
	ASSERT_RETURN(Owner != nullptr);

	bIsDestroyed = true;

	// Spawn husk
	if (HuskClass != NULL)	{
		UWorld* World = GetWorld();
		ASSERT_RETURN(World != nullptr);


		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.bNoFail = true;
		FTransform SpawnTransform = Owner->GetActorTransform();
		World->SpawnActor<AActor>(HuskClass, SpawnTransform, SpawnParams);
	}

	HideOwner();
}

void UDestructibleComponent::HideOwner()
{
	AActor* Owner = GetOwner();
	ASSERT_RETURN(Owner != nullptr);

	// Hide ourselves
	Owner->SetActorHiddenInGame(true);
	Owner->SetActorEnableCollision(false);
}
