// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_GetCurrentPatrolPosition.generated.h"

/**
 * Get the current patrol position from the AI's spline follower
 */
UCLASS()
class RC_API UBTTask_GetCurrentPatrolPosition : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_GetCurrentPatrolPosition(const FObjectInitializer& ObjectInitializer);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory) override;
};
