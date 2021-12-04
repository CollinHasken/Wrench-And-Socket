// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveGameInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USaveGameInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that should be saved
 */
class RC_API ISaveGameInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// Called once the actor has loaded
	UFUNCTION(BlueprintNativeEvent)
	void OnActorLoaded();

	// Whether this actor currently needs to save
	UFUNCTION(BlueprintNativeEvent)
	bool ActorNeedsSaving();

	// Whether this actor currently needs to save
	virtual bool ActorNeedsSaving_Implementation() { return true; }
};
