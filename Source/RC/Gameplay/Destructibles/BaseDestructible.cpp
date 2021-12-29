// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseDestructible.h"

#include "Components/StaticMeshComponent.h"

#include "RC/Debug/Debug.h"
#include "RC/Gameplay/Destructibles/DestructibleComponent.h"

ABaseDestructible::ABaseDestructible()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetSimulatePhysics(true);
	RootComponent = Mesh;

	Destructible = CreateDefaultSubobject<UDestructibleComponent>(TEXT("Destructible"));
}

// Save destructible status
void ABaseDestructible::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaveGame())
	{
		if (Destructible != nullptr)
		{
			Ar << *Destructible;
		}
	}
}

// Request for this character to be damaged
void ABaseDestructible::RequestDamage(FDamageRequestParams& Params)
{
	ASSERT_RETURN(Destructible != nullptr);

	/**
	 *
	 * TODO
	 * Check if it was from wrench
	 * 
	 */
	Destructible->ApplyHit();
}

// Is the destructible destroyed
bool ABaseDestructible::IsDead() const
{
	return Destructible->IsDestroyed();
}
