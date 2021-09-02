// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/AI/BaseAIController.h"
#include "BasicGruntAIController.generated.h"

/**
 * 
 */
UCLASS()
class RC_API ABasicGruntAIController : public ABaseAIController
{
	GENERATED_BODY()

public:
	void Tick(float DeltaTime) override;

	float tests = 0;
	EAIStateChangeResult test() { tests = 2; return EAIStateChangeResult::InProgress; }
protected:
	virtual void SetupStateChangeFunctions() override;

};
