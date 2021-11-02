// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"
#include "Enforcer.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class RC_API AEnforcer : public ABasePlayerWeapon
{
	GENERATED_BODY()

public:
	AEnforcer();
};
