// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_GetClosestPatrolPosition.generated.h"

/**
 * Get the closest patrol point of the AI's spline follower
 */
UCLASS(Blueprintable)
class RC_API UBTTask_GetClosestPatrolPosition : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_GetClosestPatrolPosition(const FObjectInitializer& ObjectInitializer);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory) override;
};
