// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"
#include "BaseEnemyWeapon.generated.h"

/**
 * Base weapon an enemy uses
 */
UCLASS()
class RC_API ABaseEnemyWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:

	// Attack the player
	bool AttackPlayer();
};
