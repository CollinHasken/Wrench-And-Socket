// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseCharacter.h"

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

