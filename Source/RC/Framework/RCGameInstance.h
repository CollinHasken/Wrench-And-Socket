// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RCGameInstance.generated.h"


/**
 * 
 */
UCLASS()
class RC_API URCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void RequestNewLevel(const FName& LevelName);

	const TArray<uint8>& GetLevelTransitionData() { return LevelTransitionData; }

private:
	TArray<uint8> LevelTransitionData;
};
