// Fill out your copyright notice in the Description page of Project Settings.


#include "RC/Weapons/Weapons/BaseEnemyWeapon.h"

#include "Kismet/GameplayStatics.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Debug/Debug.h"

// Shoot at the player
bool ABaseEnemyWeapon::ShootAtPlayer()
{
	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, false);

	ABaseCharacter* Player = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	LOG_RETURN_VALUE(Player != nullptr, false, LogAI, Error, "Unable to find Player");

	return Super::ShootAtTarget(Player);
}