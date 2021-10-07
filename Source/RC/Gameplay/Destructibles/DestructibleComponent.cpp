// Fill out your copyright notice in the Description page of Project Settings.
#include "DestructibleComponent.h"

#include "RC/Debug/Debug.h"

// Apply hit to destructible	 
void UDestructibleComponent::ApplyHit()
{
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

	// Destroy ourselves
	Owner->Destroy();
}