// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Collectibles/Collectible.h"
#include "Bolt.generated.h"

/**
 * 
 */
UCLASS()
class RC_API ABolt : public ACollectible
{
	GENERATED_BODY()
	
public:
	// The amount of bolts collecting this should grant
	// This needs to be outside of the collectible info so we can have different bolt amounts adding to the same collectible data
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	int BoltAmount = 1;

private:
	// Get the amount to be granted when this has been collected
	int GetCollectionAmount() const override { return BoltAmount; }
};
