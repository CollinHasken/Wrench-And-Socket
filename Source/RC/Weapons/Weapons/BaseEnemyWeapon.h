// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/BaseWeapon.h"
#include "BaseEnemyWeapon.generated.h"

/**
 * 
 */
UCLASS()
class RC_API ABaseEnemyWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:

	/** Shoot at the player */
	bool ShootAtPlayer();
};