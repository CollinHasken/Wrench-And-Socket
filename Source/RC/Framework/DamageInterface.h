// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "RC/Weapons/RCWeaponTypes.h"

#include "DamageInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RC_API IDamageInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Request for this interface to be damaged
	 * @param Params	The request params
	 */
	virtual void RequestDamage(FDamageRequestParams& Params) = 0;
	
	// Is the interface dead
	virtual bool IsDead() const = 0;
};
