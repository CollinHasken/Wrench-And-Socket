// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGruntAIController.h"

void ABasicGruntAIController::SetupStateChangeFunctions()
{
	Super::SetupStateChangeFunctions();

	TStateChangeFunctionMap NewMap;

	TNewStateChangeFunctionMap IdleMap;
	IdleMap.Emplace(EAIState::Patrol, new FStateChangePredicate<ABasicGruntAIController>(this, &ABasicGruntAIController::test));
	NewMap.Add(EAIState::Idle, IdleMap);

	AddStateChangeFunctions(NewMap);
}

void ABasicGruntAIController::Tick(float DeltaTime)
{
	if (tests > 0)
	{
		tests -= DeltaTime;
		if (tests <= 0)
		{
			FinishStateChange();
		}
	}
}
