// Fill out your copyright notice in the Description page of Project Settings.
#include "DestructibleComponent.h"

#include "Kismet/GameplayStatics.h"

#include "RC/Debug/Debug.h"
#include "RC/Framework/RCGameMode.h"

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

	if (bIsDestroyed)
	{
		AActor* Owner = GetOwner();
		ASSERT_RETURN(Owner != nullptr);

		Owner->Destroy();
	}
}

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

	bIsDestroyed = true;

	// Save that this has been destroyed
	ARCGameMode* GameMode = Cast<ARCGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode != nullptr)
	{
		GameMode->SaveActorForLevelTransition(Owner);
	}
	else
	{
		ASSERT(GameMode != nullptr);
	}

	Owner->Destroy();
}
