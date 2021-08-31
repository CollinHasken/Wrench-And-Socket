// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RCGameMode.generated.h"

UCLASS(minimalapi)
class ARCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARCGameMode();

	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void SaveLevelTransitionDataToMemory(TArray<uint8>* Data);
};



