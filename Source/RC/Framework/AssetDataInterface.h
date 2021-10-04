// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AssetDataInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAssetDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RC_API IAssetDataInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Get the Primary Data Asset Id associated with this data actor's data.
	virtual FPrimaryAssetId GetInfoId() const = 0;
};
