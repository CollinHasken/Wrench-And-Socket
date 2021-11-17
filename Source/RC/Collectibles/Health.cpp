// Fill out your copyright notice in the Description page of Project Settings.
#include "Health.h"

#include "Kismet/GameplayStatics.h"

#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Components/HealthComponent.h"
#include "RC/Debug/Debug.h"

// Grant an amount of the collectible
void UHealthData::GrantCollectible(int Amount)
{
	ARCCharacter* Player = Cast<ARCCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	LOG_RETURN(Player != nullptr, LogCollectible, Warning, "Trying to give health but the player isn't retrievable");

	UHealthComponent* Health = Player->GetHealth();
	ASSERT_RETURN(Health != nullptr, "Unable to get player health");

	Health->GrantHealth(Amount);
}
