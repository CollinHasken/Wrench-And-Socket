// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Util/RCTypes.h"
#include "Health.generated.h"

/**
 * Health save data
 */
UCLASS(BlueprintType)
class UHealthData : public UCollectibleData
{
	GENERATED_BODY()

public:
	/**
	 * Grant an amount of health
	 * @param Amount The amount of health to grant
	 */
	void GrantCollectible(int Amount) override;
};
