// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseEnemyWeapon.h"

#include "Kismet/GameplayStatics.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Weapons/Weapons/Components/WeaponComponent.h"

// Attack the player
bool ABaseEnemyWeapon::AttackPlayer()
{
	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	ABaseCharacter* Player = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	LOG_RETURN_VALUE(Player != nullptr, false, LogAI, Error, "Unable to find Player");

	ASSERT_RETURN_VALUE(WeaponComponent != nullptr, false);
	return WeaponComponent->AttackTarget(Player);
}
